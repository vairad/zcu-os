#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace sort_program {

	size_t sort_main(int argc, char *argv[]) {
		if (argc == 1 || argc == 2) {
			bool reverse = false;
			if (argc == 2) {
				std::string arg = argv[1];
				if (arg == "/R" || arg == "/r") {
					reverse = true;
				}
			}
			std::string line = "";
			size_t read;
			size_t buffer_size = 1023;
			char buffer[1024];
			read = kiv_os_lib::read(buffer, buffer_size);
			while (read != 0 && read != -1) {
				buffer[read] = 0; // Terminate the line.
				line.append(buffer);
				read = kiv_os_lib::read(buffer, buffer_size);
			}

			std::vector<std::string> lines = std::vector<std::string>();
			std::stringstream ss(line);
			std::string segment;
			while (std::getline(ss, segment, '\n')) {
				if (segment.size() > 0) {
					lines.push_back(segment);
				}
			}

			std::sort(lines.begin(), lines.end());

			if (!reverse) {
				for (size_t i = 0; i < lines.size(); i++) {
					std::string line = lines[i];
					kiv_os_lib::printLn(line.c_str(), line.length());
				}
			} else {
				for (size_t i = lines.size() - 1; i > 0; i--) {
					std::string line = lines[i];
					kiv_os_lib::printLn(line.c_str(), line.length());
				}
			}
		} else {
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall sort(const kiv_os::TRegisters &regs) {
	std::vector<std::string> args = kiv_os_lib::getArgs("sort", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return sort_program::sort_main(static_cast<int>(argv.size()), argv.data());
}
