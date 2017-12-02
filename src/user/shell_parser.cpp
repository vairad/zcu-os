#include <locale>

#include "shell_parser.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace shell_parser {

	Command createCommand(std::string command, std::vector<std::string> parameters, InOutType *in, InOutType *out, InOutType *err) {
		Command retVal;
		retVal.name = command;
		retVal.parameters = parameters;
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
		std::string command;
		size_t size = parts.size();
		InOutType in = InOutType::STANDARD;
		InOutType out = InOutType::STANDARD;
		InOutType err = InOutType::STANDARD;

		for (int i = 0; i < size; i++) {
			std::string p = parts[i];
			if (p == "echo" || p == "cd" || p == "dir" || p == "md" || p == "rd" || p == "type" || p == "wc"
				|| p == "sort" || p == "ps" || p == "rgen" || p == "freq" || p == "shutdown" || p == "shell" || p == "exit") {
				if (i != 0) {
					retVal.push_back(createCommand(command, params, &in, &out, &err));
					params.clear();
				}
				command = p;
			} else if (p == "|") {
				out = InOutType::PIPE;
			} else if (p == ">") {
				out = InOutType::FILE_NEW;
			} else if (p == ">>") {
				out = InOutType::FILE_APPEND;
			} else if (p == "<") {
				in = InOutType::FILE_NEW;
			} else if (p == "2>") {
				err = InOutType::FILE_NEW;
			} else if (p == "2>>") {
				err = InOutType::FILE_APPEND;
			} else {
				//p.erase(std::remove_if(p.begin(), p.end(), ](char c) {return c == '\"'; }), p.end());
				params.push_back(p);
			}
		}
		// Push last command to vector.
		if (size > 0) {
			retVal.push_back(createCommand(command, params, &in, &out, &err));
		}

		return retVal;
	}

	std::vector<Command> parseCommands(std::string line) {
		std::vector<std::string> parts = kiv_os_lib::parseLine(line);

		return splitToCommands(parts);
	}

}
