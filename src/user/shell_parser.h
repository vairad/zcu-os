#pragma once

#include <vector>
#include <string>

namespace kiv_os {

	// TODO: Klaus - Consider renaming.
	enum class Type {
		STANDARD,
		PIPE,
		FILE
	};

	typedef struct shell_command {
		std::string name;
		std::vector<std::string> parameters;
		Type std_in;
		Type std_out;
		Type std_err;
	} Command;

	std::vector<kiv_os::Command> parseLine(std::string line);

}