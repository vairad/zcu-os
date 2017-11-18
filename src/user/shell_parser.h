#pragma once

#include <vector>
#include <string>

namespace kiv_os {

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

	std::vector<kiv_os::Command> parseLine(std::string line);

}