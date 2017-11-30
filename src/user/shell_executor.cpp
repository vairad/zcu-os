#include "shell_executor.h"
#include "shell_cd.h"
#include "rtl.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

bool checkName(std::string name) {
	bool retVal = true;
	if (name.empty()) {
		retVal = false;
	}

	return retVal;
}

void incorrectSyntax() {
	std::string error = "The syntax of the command is incorrect.\n";
	kiv_os::printErr(error.c_str(), error.length());
}

bool setStdIn(kiv_os::InOutType in, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle *std_in) {
	bool retVal = true;
	switch (in) {
	case kiv_os::InOutType::STANDARD:
		*std_in = kiv_os::stdInput;
		break;
	case kiv_os::InOutType::PIPE:
		*std_in = pipeHandles[1];
		if (*std_in == NULL) {
			// Missing pipe.
			incorrectSyntax();
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_NEW:
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

bool setStdOut(kiv_os::InOutType out, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle *std_out) {
	bool retVal = true;
	bool ok;
	switch (out) {
	case kiv_os::InOutType::STANDARD:
		*std_out = kiv_os::stdOutput;
		break;
	case kiv_os::InOutType::PIPE:
		ok = kiv_os_rtl::Create_Pipe(pipeHandles);
		if (ok) {
			*std_out = pipeHandles[0];
		} else {
			// Error occured while creating pipe.
			std::string error = "Error creating pipe.\n";
			kiv_os::printErr(error.c_str(), error.length());
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_NEW:
		if (!params.empty()) {
			std::string filename = params.back();
			*std_out = kiv_os_rtl::Create_File(filename.c_str(), 0);
		} else {
			// Missing argument (filename).
			incorrectSyntax();
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_APPEND:
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

bool setStdErr(kiv_os::InOutType err, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle *std_err) {
	bool retVal = true;
	switch (err) {
	case kiv_os::InOutType::STANDARD:
		*std_err = kiv_os::stdError;
		break;
	case kiv_os::InOutType::FILE_NEW:
		if (!params.empty()) {
			std::string filename = params.back();
			*std_err = kiv_os_rtl::Create_File(filename.c_str(), 0);
		} else {
			// Missing argument (filename).
			incorrectSyntax();
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_APPEND:
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

void stopCommands(std::vector<kiv_os::CommandExecute> commands, size_t num) {
	for (size_t i = 0; i < num; i++) {
		// TODO: Klaus - Stop processes / threads.
	}
}

void waitForCommands(std::vector<kiv_os::CommandExecute> toWait) {
	for (size_t i = 0; i < toWait.size(); i++) {
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

void runCommands(std::vector<kiv_os::CommandExecute> toExecute) {
	for (size_t i = 0; i < toExecute.size(); i++) {
		kiv_os::CommandExecute *ce = &toExecute[i];
		std::vector<std::string> params = ce->parameters;
		std::string args = "";
		for (size_t j = 0; j < params.size(); j++) {
			if (j != 0) {
				args.append(" ");
			}
			args.append(params[i]);
		}
		if (ce->name == "cd") {
			bool ok = kiv_os::cd(*ce, args);
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
				kiv_os::printErr(errorStr.c_str(), errorStr.length());
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
bool kiv_os::executeCommands(std::vector<kiv_os::Command> commands) {
	THandle pipeHandles[2] = {NULL, NULL};
	std::vector<CommandExecute> toExecute = std::vector<CommandExecute>();
	for (size_t i = 0; i < commands.size(); i++) {
		Command command = commands[i];
		std::string name = command.name;
		if (checkName(name)) {

			if(name == "exit"){ return false;}

			InOutType in = command.std_in;
			InOutType out = command.std_out;
			InOutType err = command.std_err;
			std::vector<std::string> params = command.parameters;

			THandle std_in;
			THandle std_out;
			THandle std_err;
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
			kiv_os::printErr(error.c_str(), error.length());
			return true;
		}
	}

	runCommands(toExecute);
	return true;
}
