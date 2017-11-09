#pragma once

#include <vector>
#include <string>

namespace kiv_os {

	typedef struct shell_command {
		std::string name;
		std::vector<std::string> parameters;
	} Command;

	std::vector<kiv_os::Command> parseLine(std::string line);

}