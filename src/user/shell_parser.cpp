#include <locale>

#include "shell_parser.h"

kiv_os::Command createCommand(std::string command, std::vector<std::string> parameters, kiv_os::InOutType *in, kiv_os::InOutType *out, kiv_os::InOutType *err) {
	kiv_os::Command retVal;
	retVal.name = command;
	retVal.parameters = parameters;
	retVal.std_in = *in;
	retVal.std_out = *out;
	retVal.std_err = *err;
	if (*out == kiv_os::InOutType::PIPE) {
		*in = kiv_os::InOutType::PIPE;
	} else {
		*in = kiv_os::InOutType::STANDARD;
	}
	*out = kiv_os::InOutType::STANDARD;
	*err = kiv_os::InOutType::STANDARD;

	return retVal;
}

std::vector<kiv_os::Command> splitToCommands(std::vector<std::string> parts) {
	std::vector<kiv_os::Command> retVal = std::vector<kiv_os::Command>();
	std::vector<std::string> params = std::vector<std::string>();
	std::string command;
	size_t size = parts.size();
	kiv_os::InOutType in = kiv_os::InOutType::STANDARD;
	kiv_os::InOutType out = kiv_os::InOutType::STANDARD;
	kiv_os::InOutType err = kiv_os::InOutType::STANDARD;

	for (int i = 0; i < size; i++) {
		std::string p = parts[i];
		if (p == "echo" || p == "cd" || p == "dir" || p == "md" || p == "rd" || p == "type" || p == "wc" || p == "sort" || p == "ps" || p == "rgen" || p == "freq" || p == "shutdown") {
			if (i != 0) {
				retVal.push_back(createCommand(command, params, &in, &out, &err));
				params.clear();
			}
			command = p;
		}
		else if (p == "|") {
			out = kiv_os::InOutType::PIPE;
		} else if (p == ">") {
			out = kiv_os::InOutType::FILE_NEW;
		} else if (p == ">>") {
			out = kiv_os::InOutType::FILE_APPEND;
		} else if (p == "<") {
			in = kiv_os::InOutType::FILE_NEW;
		} else if (p == "2>") {
			err = kiv_os::InOutType::FILE_NEW;
		} else if (p == "2>>") {
			err = kiv_os::InOutType::FILE_APPEND;
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

std::vector<kiv_os::Command> kiv_os::parseLine(std::string line) {
	std::vector<std::string> parts = std::vector<std::string>();
	std::string token = "";
	bool inQuotes = false;
	for (size_t i = 0; i < line.length(); i++) {
		char c = line[i];
		if (!inQuotes) {
			if (c == ' ') {
				parts.push_back(token);
				token = "";
			} else if (isspace(c, std::locale{})) {
				continue;
			} else if (c == '\"') {
				token.append(1, c);
				inQuotes = true;
			} else {
				token.append(1, c);
			}
		} else {
			if (isspace(c, std::locale{}) && c != ' ') {
				continue;
			} else if (c == '\"') {
				token.append(1, c);
				inQuotes = false;
			} else {
				token.append(1, c);
			}
		}
	}
	// Push last part to vector
	if (!line.empty()) {
		parts.push_back(token);
	}

	return splitToCommands(parts);
}
