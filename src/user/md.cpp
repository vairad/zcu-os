#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace md_program {

	size_t md_main(int argc, char **argv) {
		if (argc == 2) {
			char *file = argv[1];
			kiv_os::THandle handle = kiv_os_rtl::Create_File(file, kiv_os::faDirectory);
			// TODO: Klaus - Check for error all over the application where creating file.
			kiv_os_rtl::Close_File(handle);
		} else {
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall md(const kiv_os::TRegisters &regs) {
	int argc;
	char **argv = kiv_os_lib::getArgs("md", regs, &argc);
	return md_program::md_main(argc, argv);
}
