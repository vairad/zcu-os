#include <locale>

#include "shell_parser.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace shell_parser {

	const char commands[][16] = {
		"echo", "cd", "dir", "md", "rd", "type", "wc",
		"sort", "ps", "rgen", "freq", "shutdown", "shell", "exit",
		"repeater"
	};
	const size_t commandCount = sizeof(commands);

	bool isValidCommand(std::string &test) {
		for (uint8_t i = 0; i < commandCount; i++) {
			if (test == commands[i]) {
				return true;
			}
		}

		return false;
	}

	Command createCommand(std::string command, std::vector<std::string> parameters, std::string files[], InOutType *in, InOutType *out, InOutType *err) {
		Command retVal;
		retVal.name = command;
		retVal.parameters = parameters;
		retVal.files[0] = files[0];
		retVal.files[1] = files[1];
		retVal.files[2] = files[2];
		retVal.std_in = *in;
		retVal.std_out = *out;
		retVal.std_err = *err;
		if (*out == InOutType::PIPE) {
			*in = InOutType::PIPE;
		} else {
			*in = InOutType::STANDARD;
		}
		*out = InOutType::STANDARD;
		*err = InOutType::STANDARD;

		return retVal;
	}



	std::vector<Command> splitToCommands(std::vector<std::string> parts) {
		std::vector<Command> retVal = std::vector<Command>();
		std::vector<std::string> params = std::vector<std::string>();
		std::string files[3] = { "", "", "" };
		std::string command;
		size_t size = parts.size();
		InOutType in = InOutType::STANDARD;
		InOutType out = InOutType::STANDARD;
		InOutType err = InOutType::STANDARD;

		for (int i = 0; i < size; i++) {
			std::string p = parts[i];
			if (isValidCommand(p)) {
				if (i != 0) {
					retVal.push_back(createCommand(command, params, files, &in, &out, &err));
					params.clear();
					files[0] = "";
					files[1] = "";
					files[2] = "";
				}
				command = p;
			} else if (p == "|") {
				out = InOutType::PIPE;
			} else if (p == ">") {
				out = InOutType::FILE_NEW;
				if (i + 1 < size) {
					i++;
					files[1] = parts[i];
				}
			} else if (p == ">>") {
				out = InOutType::FILE_APPEND;
				if (i + 1 < size) {
					i++;
					files[1] = parts[i];
				}
			} else if (p == "<") {
				in = InOutType::FILE_NEW;
				if (i + 1 < size) {
					i++;
					files[0] = parts[i];
				}
			} else if (p == "2>") {
				err = InOutType::FILE_NEW;
				if (i + 1 < size) {
					i++;
					files[2] = parts[i];
				}
			} else if (p == "2>>") {
				err = InOutType::FILE_APPEND;
				if (i + 1 < size) {
					i++;
					files[2] = parts[i];
				}
			} else {
				params.push_back(p);
			}
		}
		// Push last command to vector.
		if (size > 0) {
			retVal.push_back(createCommand(command, params, files, &in, &out, &err));
		}

		return retVal;
	}

	std::vector<Command> parseCommands(std::string line) {
		std::vector<std::string> parts = kiv_os_lib::parseLine(line);

		return splitToCommands(parts);
	}

}
