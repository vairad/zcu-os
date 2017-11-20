#pragma once

#include "shell_parser.h"

namespace kiv_os {

	typedef struct shell_command_execute {
		std::string name;
		std::vector<std::string> parameters;
		THandle std_in;
		THandle std_out;
		THandle std_err;
		THandle handle = erInvalid_Handle;
	} CommandExecute;

	void executeCommands(std::vector<kiv_os::Command> commands, kiv_os::THandle shell_in, kiv_os::THandle shell_out, kiv_os::THandle shell_err);

}