#pragma once

#include <vector>
#include <string>

#undef stdin
#undef stderr
#undef stdout

namespace shell_parser {

	enum class InOutType {
		STANDARD,
		PIPE,
		FILE_NEW,
		FILE_APPEND
	};

	typedef struct shell_command {
		std::string name;
		std::vector<std::string> parameters;
		InOutType std_in;
		InOutType std_out;
		InOutType std_err;
	} Command;

	std::vector<Command> parseCommands(std::string line);

}