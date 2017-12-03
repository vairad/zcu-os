#include "common.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace echo_program {

	size_t echo_main(int argc, char **argv) {
		std::string line = "";
		for (size_t i = 1; i < argc; i++) {
			if (i != 1) {
				line.append(" ");
			}

			line.append(argv[i]);
		}

		kiv_os_lib::printLn(line.c_str(), line.length());

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall echo(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("echo", regs, &argc);
	return echo_program::echo_main(argc, argv);
}
