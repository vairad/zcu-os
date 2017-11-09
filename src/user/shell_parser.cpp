#include <string.h>
#include <algorithm>
#include <locale>

#include "shell_parser.h"

kiv_os::Command createCommand(std::string command, std::vector<std::string> parameters) {
	kiv_os::Command retVal;
	retVal.name = command;
	retVal.parameters = parameters;

	return retVal;
}

std::vector<kiv_os::Command> splitToCommands(std::vector<std::string> parts) {
	std::vector<kiv_os::Command> retVal = std::vector<kiv_os::Command>();
	std::vector<std::string> params = std::vector<std::string>();
	std::string command;
	size_t size = parts.size();

	for (int i = 0; i < size; i++) {
		std::string p = parts[i];
		// TODO: Klaus - Handle pipes and redirects
		if (p == "echo" || p == "cd" || p == "dir" || p == "md" || p == "rd" || p == "type" || p == "wc" || p == "sort" || p == "ps" || p == "rgen" || p == "freq" || p == "shutdown") {
			if (i != 0) {
				retVal.push_back(createCommand(command, params));
				params.clear();
			}
			command = p;
		} else {
			params.push_back(p);
		}
	}
	if (size > 0) {
		retVal.push_back(createCommand(command, params));
	}

	return retVal;
}

std::vector<kiv_os::Command> kiv_os::parseLine(std::string line) {
	std::vector<std::string> parts = std::vector<std::string>();
	const char delim[] = " ";
	char *cstr = const_cast<char *>(line.c_str());
	char *nextToken;
	char *token = strtok_s(cstr, delim, &nextToken);
	while (token != NULL) {
		std::string s = token;
		s.erase(std::remove_if(s.begin(), s.end(), [l = std::locale{}](char c) {return std::isspace<char>(c, l); }), s.end());
		parts.push_back(s);
		token = strtok_s(NULL, delim, &nextToken);
	}

	return splitToCommands(parts);
}
