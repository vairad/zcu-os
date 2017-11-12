#pragma once

#include <vector>
#include <string>

namespace kiv_os {

	enum class Type {
		STANDARD,
		PIPE,
		FILE
	};

	typedef struct shell_command {
		std::string name;
		std::vector<std::string> parameters;
		// TODO: Klaus - Create enum to set in, out, err.
		Type std_in;
		Type std_out;
		Type std_err;
	} Command;

	std::vector<kiv_os::Command> parseLine(std::string line);

}