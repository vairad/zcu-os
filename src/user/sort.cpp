#include "common.h"

#include <string>
#include <vector>
#include <algorithm>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace sort_program {

	size_t sort_main(int argc, char *argv[]) {
		if (argc == 1 || argc == 2) {
			bool reverse = false;
			if (argc == 2 && (argv[1] == "/R" || argv[1] == "/r")) {
				reverse = true;
			}
			std::vector<std::string> lines = std::vector<std::string>();
			size_t read;
			size_t buffer_size = 255;
			char buffer[256];
			while ((read = kiv_os_lib::read(buffer, buffer_size)) != 0) {
				buffer[read] = 0; // Terminate the line.
				std::string line = buffer;
				lines.push_back(line);
			}

			std::sort(lines.begin(), lines.end());

			if (!reverse) {
				for (size_t i = 0; i < lines.size(); i++) {
					std::string line = lines[i];
					kiv_os_lib::printLn(line.c_str(), line.length());
				}
			} else {
				for (size_t i = lines.size(); i >= 0; i--) {
					std::string line = lines[i];
					kiv_os_lib::printLn(line.c_str(), line.length());
				}
			}
		} else {
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
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
