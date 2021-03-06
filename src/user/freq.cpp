#include <map>
#include <string>
#include <sstream>

#include "common.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace freq_program {

	size_t freq_main(int argc, char *argv[]) {
		std::map<char, int> freqs;
		size_t buffer_size = 255;
		char buffer[256];
		size_t read = kiv_os_lib::read(buffer, buffer_size);
		while (read != 0 && read != -1) {
			buffer[read] = 0; // Terminate the string.
			for (size_t i = 0; i < read; i++) {
				char c = buffer[i];
				std::map<char, int>::iterator it = freqs.find(c);
				if (it != freqs.end()) {
					freqs[c] = freqs[c] + 1;
				} else {
					freqs[c] = 1;
				}
			}

			read = kiv_os_lib::read(buffer, buffer_size);
		}

		for (std::map<char, int>::iterator it = freqs.begin(); it != freqs.end(); ++it) {
			char c = it->first;
			int count = it->second;

			std::string code;
			std::stringstream ss;
			ss << std::hex << (int)c;
			ss >> code;

			std::string line = "0x";
			line.append(code);
			line.append(" : ");
			line.append(std::to_string(count));
			kiv_os_lib::printLn(line.c_str(), line.length());
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall freq(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("echo", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return freq_program::freq_main(static_cast<int>(argv.size()), argv.data());
}
