#pragma once

#include <string>
#include <vector>

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace kiv_os_lib {

	const size_t SUCCESS = 0;
	const size_t FILE_NOT_FOUND = 1;
	const size_t ATTRS_ERROR = 2;
	const size_t INCORRECT_SYNTAX = 3;
	const size_t READ_ERROR = 4;
	const size_t DIR_NOT_EMPTY = 5;
	const size_t CANNOT_CREATE_THREAD = 6;

	char **getArgs(char *program_name, const kiv_os::TRegisters &context, int *argc);

	std::vector<std::string> parseLine(std::string line);

	size_t read(const char *buffer, size_t buffer_size);

	size_t print(const char *buffer, size_t buffer_size);
	size_t printLn(const char *buffer, size_t buffer_size);

	size_t printErr(const char *buffer, size_t buffer_size);

	bool getWD(const char *buffer, size_t buffer_size);

	bool isDir(kiv_os::THandle handle, bool &isDir, uint16_t *attrs);
}
