#include "shell_executor.h"
#include "shell_cd.h"
#include "rtl.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace shell_executor {

	bool checkName(std::string name) {
		bool retVal = true;
		if (name.empty()) {
			retVal = false;
		}

		return retVal;
	}

	void incorrectSyntax() {
		std::string error = "The syntax of the command is incorrect.\n";
		kiv_os_lib::printErr(error.c_str(), error.length());
	}

	bool setStdIn(shell_parser::InOutType in, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
		kiv_os::THandle *std_in) {
		bool retVal = true;
		switch (in) {
		case shell_parser::InOutType::STANDARD:
			*std_in = kiv_os::stdInput;
			break;
		case shell_parser::InOutType::PIPE:
			*std_in = pipeHandles[1];
			if (*std_in == NULL) {
				// Missing pipe.
				incorrectSyntax();
				retVal = false;
			}
			break;
		case shell_parser::InOutType::FILE_NEW:
			if (!params.empty()) {
				std::string filename = params.back();
				*std_in = kiv_os_rtl::Create_File(filename.c_str(), 0);
			} else {
				// Missing argument (filename).
				incorrectSyntax();
				retVal = false;
			}
			break;
		}

		return retVal;
	}

	bool setStdOut(shell_parser::InOutType out, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
		kiv_os::THandle *std_out) {
		bool retVal = true;
		bool ok;
		switch (out) {
		case shell_parser::InOutType::STANDARD:
			*std_out = kiv_os::stdOutput;
			break;
		case shell_parser::InOutType::PIPE:
			ok = kiv_os_rtl::Create_Pipe(pipeHandles);
			if (ok) {
				*std_out = pipeHandles[0];
			} else {
				// Error occured while creating pipe.
				std::string error = "Error creating pipe.\n";
				kiv_os_lib::printErr(error.c_str(), error.length());
				retVal = false;
			}
			break;
		case shell_parser::InOutType::FILE_NEW:
			if (!params.empty()) {
				std::string filename = params.back();
				*std_out = kiv_os_rtl::Create_File(filename.c_str(), 0);
			} else {
				// Missing argument (filename).
				incorrectSyntax();
				retVal = false;
			}
			break;
		case shell_parser::InOutType::FILE_APPEND:
			if (!params.empty()) {
				std::string filename = params.back();
				*std_out = kiv_os_rtl::Create_File(filename.c_str(), kiv_os::fmOpen_Always);
				// TODO: Klaus - We need to set position to the end of the file.
			} else {
				// Missing argument (filename).
				incorrectSyntax();
				retVal = false;
			}
			break;
		}

		return retVal;
	}

	bool setStdErr(shell_parser::InOutType err, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
		kiv_os::THandle *std_err) {
		bool retVal = true;
		switch (err) {
		case shell_parser::InOutType::STANDARD:
			*std_err = kiv_os::stdError;
			break;
		case shell_parser::InOutType::FILE_NEW:
			if (!params.empty()) {
				std::string filename = params.back();
				*std_err = kiv_os_rtl::Create_File(filename.c_str(), 0);
			} else {
				// Missing argument (filename).
				incorrectSyntax();
				retVal = false;
			}
			break;
		case shell_parser::InOutType::FILE_APPEND:
			if (!params.empty()) {
				std::string filename = params.back();
				*std_err = kiv_os_rtl::Create_File(filename.c_str(), kiv_os::fmOpen_Always);
				// TODO: Klaus - We need to set position to the end of the file.
			} else {
				// Missing argument (filename).
				incorrectSyntax();
				retVal = false;
			}
			break;
		}

		return retVal;
	}

	void stopCommands(std::vector<shell_executor::CommandExecute> commands, size_t num) {
		for (size_t i = 0; i < num; i++) {
			// TODO: Klaus - Stop processes / threads.
		}
	}

	void waitForCommands(std::vector<shell_executor::CommandExecute> toWait) {
		for (size_t i = 0; i < toWait.size(); i++) {

			if (toWait[i].handle == kiv_os::erInvalid_Handle)
			{
				continue; // pass invalid handler to kernel make no sense
			}

			bool ok = kiv_os_rtl::Join_One_Handle(toWait[i].handle);
			if (!ok) {
				// TODO: Klaus - Error occured while waiting.

				/*std::string error = "Error waiting for process or thread.\n";
				size_t written;
				kiv_os::printErr(error.c_str(), error.length());
				stopCommands(toWait, toWait.size());
				return;*/
			}
		}
	}

	void runCommands(std::vector<shell_executor::CommandExecute> toExecute) {
		for (size_t i = 0; i < toExecute.size(); i++) {
			shell_executor::CommandExecute *ce = &toExecute[i];
			std::vector<std::string> params = ce->parameters;
			std::string args = "";
			for (size_t j = 0; j < params.size(); j++) {
				if (j != 0) {
					args.append(" ");
				}
				args.append(params[i]);
			}
			if (ce->name == "cd") {
				ce->handle = kiv_os::erInvalid_Handle;
				bool ok = shell_cd::cd(*ce, args);
				if (!ok) {
					// TODO: Klaus - Error during cd.
				}
			} else {
				bool ok = kiv_os_rtl::Create_Process(&ce->handle, ce->name.c_str(), args.c_str(), ce->std_in, ce->std_out, ce->std_err);
				if (!ok) {
					std::string errorStr;
					const size_t error = kiv_os_rtl::Get_Last_Error();
					switch (error) {
					case kiv_os::erFile_Not_Found:
						errorStr = "\'" + ce->name + "\' is not recognized as an internal or external command.\n";
						break;
					case kiv_os::erProces_Not_Created:
						errorStr = "Error creating new process or thread.\n";
						break;
					default:
						errorStr = "Unspecified error during run program.\n";
					}
					kiv_os_lib::printErr(errorStr.c_str(), errorStr.length());
					stopCommands(toExecute, i);
					return;
				}
				// TODO: Klaus - Close handles if needed.
			}
		}

		waitForCommands(toExecute);
	}

	/**
	 * \brief Method execute commands parsed by shell_parser
	 * \param commands vector of kiv_os::Command structes
	 * \return flag if shell should continue in readinng (true) or false if shell should end
	 */
	bool executeCommands(std::vector<shell_parser::Command> commands) {
		kiv_os::THandle pipeHandles[2] = { NULL, NULL };
		std::vector<CommandExecute> toExecute = std::vector<CommandExecute>();
		for (size_t i = 0; i < commands.size(); i++) {
			shell_parser::Command command = commands[i];
			std::string name = command.name;
			if (checkName(name)) {

				if (name == "exit") { return false; }

				shell_parser::InOutType in = command.std_in;
				shell_parser::InOutType out = command.std_out;
				shell_parser::InOutType err = command.std_err;
				std::vector<std::string> params = command.parameters;

				kiv_os::THandle std_in;
				kiv_os::THandle std_out;
				kiv_os::THandle std_err;
				bool ok = setStdIn(in, pipeHandles, params, &std_in);
				ok = ok && setStdOut(out, pipeHandles, params, &std_out);
				ok = ok && setStdErr(err, pipeHandles, params, &std_err);
				if (!ok) {
					return true;
				}

				CommandExecute ce;
				ce.name = name;
				ce.parameters = params;
				ce.std_in = std_in;
				ce.std_out = std_out;
				ce.std_err = std_err;
				toExecute.push_back(ce);
			} else {
				// Unknown command.
				std::string error = "\'";
				error.append(command.parameters[0]);
				error.append("\' is not recognized as an internal or external command.\n");
				kiv_os_lib::printErr(error.c_str(), error.length());
				return true;
			}
		}

		runCommands(toExecute);
		return true;
	}

}
