#include "common.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace wc_program {

	void wc_main(int argc, char **argv) {
		std::string line = "";
		for (size_t i = 1; i < argc; i++) {
			if (i != 1) {
				line.append(" ");
			}
			line.append(argv[i]);
		}

		int count = 0;
		for (size_t i = 0; i < line.length(); i++) {
			if (line[i] == ' ') {
				count++;
			}
		}

		line = std::to_string(count);
		kiv_os_lib::printLn(line.c_str(), line.length());
	}

}

size_t __stdcall wc(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("wc", regs, &argc);
	wc_program::wc_main(argc, argv);
	return 0;
}
