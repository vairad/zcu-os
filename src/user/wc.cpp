#include "common.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace wc_program {

	size_t wc_main(int argc, char **argv) {
		if (argc == 1 || argc == 2) {
			bool reverse = false;
			if (argc == 2 && (argv[1] == "/R" || argv[1] == "/r")) {
				reverse = true;
			}
			std::string line = "";
			size_t read;
			size_t buffer_size = 255;
			char buffer[256];
			bool first = true;
			while ((read = kiv_os_lib::read(buffer, buffer_size)) != -1) {
				buffer[read] = 0; // Terminate the string.
				if (!first) {
					line.append(" ");
				} else {
					first = false;
				}
				line.append(buffer);
			}

			int count = 0;
			if (!reverse) {
				for (size_t i = 0; i < line.length(); i++) {
					if (line[i] == ' ') {
						count++;
					}
				}
			} else {
				for (size_t i = line.length() - 1; i >= 0; i--) {
					if (line[i] == ' ') {
						count++;
					}
				}
			}

			line = std::to_string(count);
			kiv_os_lib::printLn(line.c_str(), line.length());
		} else {
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall wc(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("wc", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return wc_program::wc_main(static_cast<int>(argv.size()), argv.data());
}
