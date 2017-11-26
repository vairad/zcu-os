#include "..\api\api.h"
#include "shell_executor.h"
#include "shell_cd.h"
#include "rtl.h"

bool checkName(std::string name) {
	bool retVal = true;
	if (name.empty()) {
		retVal = false;
	}

	return retVal;
}

void incorrectSyntax(kiv_os::THandle shell_err) {
	std::string error = "The syntax of the command is incorrect.\n";
	size_t written;
	kiv_os_rtl::Write_File(shell_err, error.c_str(), error.size(), written);
}

bool setStdIn(kiv_os::InOutType in, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle shell_in, kiv_os::THandle shell_err, kiv_os::THandle *std_in) {
	bool retVal = true;
	switch (in) {
	case kiv_os::InOutType::STANDARD:
		*std_in = shell_in;
		break;
	case kiv_os::InOutType::PIPE:
		*std_in = pipeHandles[1];
		if (*std_in == NULL) {
			// Missing pipe.
			incorrectSyntax(shell_err);
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_NEW:
		if (!params.empty()) {
			std::string filename = params.back();
			*std_in = kiv_os_rtl::Create_File(filename.c_str(), 0);
		} else {
			// Missing argument (filename).
			incorrectSyntax(shell_err);
			retVal = false;
		}
		break;
	}

	return retVal;
}

bool setStdOut(kiv_os::InOutType out, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle shell_out, kiv_os::THandle shell_err, kiv_os::THandle *std_out) {
	bool retVal = true;
	bool ok;
	switch (out) {
	case kiv_os::InOutType::STANDARD:
		*std_out = shell_out;
		break;
	case kiv_os::InOutType::PIPE:
		ok = kiv_os_rtl::Create_Pipe(pipeHandles);
		if (ok) {
			*std_out = pipeHandles[0];
		} else {
			// Error occured while creating pipe.
			std::string error = "Error creating pipe.\n";
			size_t written;
			kiv_os_rtl::Write_File(shell_err, error.c_str(), error.size(), written);
			retVal = false;
		}
		break;
	case kiv_os::InOutType::FILE_NEW:
		if (!params.empty()) {
			std::string filename = params.back();
			*std_out = kiv_os_rtl::Create_File(filename.c_str(), 0);
		} else {
			// Missing argument (filename).
			incorrectSyntax(shell_err);
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
			incorrectSyntax(shell_err);
			retVal = false;
		}
		break;
	}

	return retVal;
}

bool setStdErr(kiv_os::InOutType err, kiv_os::THandle pipeHandles[], std::vector<std::string> params,
	kiv_os::THandle shell_err, kiv_os::THandle *std_err) {
	bool retVal = true;
	switch (err) {
	case kiv_os::InOutType::STANDARD:
		*std_err = shell_err;
		break;
	case kiv_os::InOutType::FILE_NEW:
		if (!params.empty()) {
			std::string filename = params.back();
			*std_err = kiv_os_rtl::Create_File(filename.c_str(), 0);
		} else {
			// Missing argument (filename).
			incorrectSyntax(shell_err);
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
			incorrectSyntax(shell_err);
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

void waitForCommands(std::vector<kiv_os::CommandExecute> toWait, kiv_os::THandle shell_err) {
	for (size_t i = 0; i < toWait.size(); i++) {
		bool ok = kiv_os_rtl::Join_One_Handle(toWait[i].handle);
		if (!ok) {
			// TODO: Klaus - Error occured while waiting.

			/*std::string error = "Error waiting for process or thread.\n";
			size_t written;
			kiv_os_rtl::Write_File(shell_err, error.c_str(), error.size(), written);
			stopCommands(toWait, toWait.size());
			return;*/
		}
	}
}

void runCommands(std::vector<kiv_os::CommandExecute> toExecute, kiv_os::THandle shell_err) {
	for (size_t i = 0; i < toExecute.size(); i++) {
		kiv_os::CommandExecute ce = toExecute[i];
		std::vector<std::string> params = ce.parameters;
		if (ce.name == "cd") {
			bool ok = kiv_os::cd(ce);
			if (!ok) {
				// TODO: Klaus - Error during cd.
			}
		} else {
			std::string args = "";
			for (size_t j = 0; j < params.size(); j++) {
				args.append(params[i]);
			}
			bool ok = kiv_os_rtl::Create_Process(&ce.handle, ce.name.c_str(), args.c_str());
			if (!ok) {
				std::string errorStr;
				const size_t error = kiv_os_rtl::Get_Last_Error();
				switch (error) {
				case kiv_os::erFile_Not_Found:
					errorStr = "\'" + ce.name + "\' is not recognized as an internal or external command.\n";
					break;
				case kiv_os::erProces_Not_Created:
					errorStr = "Error creating new process or thread.\n";
					break;
				default:
					errorStr = "Unspecified error during run program.\n";
				}
				size_t written;
				kiv_os_rtl::Write_File(shell_err, errorStr.c_str(), errorStr.size(), written);
				stopCommands(toExecute, i);
				return;
			}
			toExecute[i] = ce; //hotfix issue #24 (RVA)
		}
	}

	waitForCommands(toExecute, shell_err);
}

void kiv_os::executeCommands(std::vector<kiv_os::Command> commands, kiv_os::THandle shell_in, kiv_os::THandle shell_out, kiv_os::THandle shell_err) {
	THandle pipeHandles[2] = {NULL, NULL};
	std::vector<CommandExecute> toExecute = std::vector<CommandExecute>();
	for (size_t i = 0; i < commands.size(); i++) {
		Command command = commands[i];
		std::string name = command.name;
		if (checkName(name)) {
			InOutType in = command.std_in;
			InOutType out = command.std_out;
			InOutType err = command.std_err;
			std::vector<std::string> params = command.parameters;

			THandle std_in;
			THandle std_out;
			THandle std_err;
			bool ok = setStdIn(in, pipeHandles, params, shell_in, shell_err, &std_in);
			ok = ok && setStdOut(out, pipeHandles, params, shell_out, shell_err, &std_out);
			ok = ok && setStdErr(err, pipeHandles, params, shell_err, &std_err);
			if (!ok) {
				return;
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
			size_t written;
			bool status = kiv_os_rtl::Write_File(shell_err, error.c_str(), error.size(), written);
			return;
		}
	}

	runCommands(toExecute, shell_err);
}
