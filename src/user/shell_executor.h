#pragma once


#include "shell_parser.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace shell_executor {

	typedef struct shell_command_execute {
		std::string name;
		std::vector<std::string> parameters;
		kiv_os::THandle std_in;
		kiv_os::THandle std_out;
		kiv_os::THandle std_err;
		kiv_os::THandle handle = kiv_os::erInvalid_Handle;
	} CommandExecute;

	bool executeCommands(std::vector<shell_parser::Command> commands);

}