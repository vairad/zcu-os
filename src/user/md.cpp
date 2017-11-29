#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

size_t md_main(int argc, char **argv) {
	if (argc == 1) {
		char *file = argv[0];
		kiv_os::THandle handle = kiv_os_rtl::Create_File(file, 0);
		// TODO: Klaus - Check for error in all application where creating file.
	} else {
		// TODO: Klaus - Error - wrong number of parameters.
	}

	return 0;
}

size_t __stdcall md(const kiv_os::TRegisters &regs) 
{
	int argc;
	char **argv = kiv_os::getArgs(regs, &argc);
	md_main(argc, argv);
	return 0;
}
