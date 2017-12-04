#include "common.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace wc_program {

	size_t wc_main(int argc, char **argv) {
		std::string line = "";
		size_t read;
		size_t buffer_size = 255;
		char buffer[256];
		while ((read = kiv_os_lib::read(buffer, buffer_size)) != -1) {
			buffer[read] = 0; // Terminate the string.
			line.append(buffer);
		}

		int count = 0;
		for (size_t i = 0; i < line.length(); i++) {
			if (line[i] == ' ') {
				count++;
			}
		}

		line = std::to_string(count);
		kiv_os_lib::printLn(line.c_str(), line.length());

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall wc(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("wc", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return wc_program::wc_main(static_cast<int>(argv.size()), argv.data());
}
