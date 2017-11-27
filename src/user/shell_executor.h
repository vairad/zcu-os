#pragma once


#include "shell_parser.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace kiv_os {

	typedef struct shell_command_execute {
		std::string name;
		std::vector<std::string> parameters;
		kiv_os::THandle std_in;
		kiv_os::THandle std_out;
		kiv_os::THandle std_err;
		kiv_os::THandle handle = erInvalid_Handle;
	} CommandExecute;

	void executeCommands(std::vector<kiv_os::Command> commands, kiv_os::THandle shell_in, kiv_os::THandle shell_out, kiv_os::THandle shell_err);

}