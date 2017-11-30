#include "common.h"
#include "rtl.h"

#include <locale>

#undef stdin
#undef stderr
#undef stdout

char **kiv_os::getArgs(const kiv_os::TRegisters &context, int *argc) {
	kiv_os::TProcess_Startup_Info *info = (kiv_os::TProcess_Startup_Info *)context.rdi.r;
	std::string line = info->arg;
	std::vector<std::string> parts = parseLine(line);
	*argc = static_cast<int>(parts.size());
	std::vector<char *> argv{};
	for (size_t i = 0; i < *argc; i++) {
		argv.push_back(&parts[i].front());
	}

	// TODO: Klaus - Test this!!
	return argv.data();
}

std::vector<std::string> kiv_os::parseLine(std::string line) {
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

	return parts;
}

size_t kiv_os::read(const char *buffer, size_t buffer_size) {
	size_t read = -1;
	// TODO: Klaus - Handle bad read.
	bool ok = kiv_os_rtl::Read_File(kiv_os::stdInput, buffer, buffer_size, read);
	return read;
}

size_t kiv_os::print(const char *buffer, size_t buffer_size) {
	size_t written = -1;
	// TODO: Klaus - Handle bad write.
	bool ok = kiv_os_rtl::Write_File(kiv_os::stdOutput, buffer, buffer_size, written);
	return written;
}

size_t kiv_os::printErr(const char *buffer, size_t buffer_size) {
	size_t written = -1;
	// TODO: Klaus - Handle bad write.
	bool ok = kiv_os_rtl::Write_File(kiv_os::stdError, buffer, buffer_size, written);
	return written;
}
