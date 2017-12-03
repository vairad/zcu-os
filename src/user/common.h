#pragma once

#include <string>
#include <vector>

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

namespace kiv_os_lib {

	char **getArgs(char *program_name, const kiv_os::TRegisters &context, int *argc);

	std::vector<std::string> parseLine(std::string line);

	size_t read(const char *buffer, size_t buffer_size);

	size_t print(const char *buffer, size_t buffer_size);
	size_t printLn(const char *buffer, size_t buffer_size);

	size_t printErr(const char *buffer, size_t buffer_size);

	bool getWD(const char *buffer, size_t buffer_size);

	bool isDir(kiv_os::THandle handle, bool &isDir, uint8_t *attrs);
}
