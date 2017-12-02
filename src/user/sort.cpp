#include "common.h"

#include <string>
#include <vector>
#include <algorithm>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace sort_program {

	void sort_main(int argc, char **argv) {
		std::vector<std::string> lines = std::vector<std::string>();
		size_t read;
		size_t buffer_size = 255;
		char buffer[256];
		while ((read = kiv_os_lib::read(buffer, buffer_size)) != -1) {
			buffer[read] = 0;
			std::string line = buffer;
			lines.push_back(line);
		}

		std::sort(lines.begin(), lines.end());
		
		for (size_t i = 0; i < lines.size(); i++) {
			std::string line = lines[i];
			kiv_os_lib::printLn(line.c_str(), line.length());
		}
	}

}

size_t __stdcall sort(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("sort", regs, &argc);
	sort_program::sort_main(argc, argv);
	return 0;
}
