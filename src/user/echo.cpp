#include "common.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace echo_program {

	void echo_main(int argc, char **argv) {
		std::string line = "";
		for (size_t i = 1; i < argc; i++) {
			if (i != 1) {
				line.append(" ");
			}

			line.append(argv[i]);
		}

		kiv_os_lib::printLn(line.c_str(), line.length());
	}

}

size_t __stdcall echo(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("md", regs, &argc);
	echo_program::echo_main(argc, argv);
	return 0;
}
