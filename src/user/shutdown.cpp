#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"


size_t shutdown_main(int argc, char **argv) {
	if (argc == 1) {
		char *file = argv[0];
		kiv_os_rtl::Shutdown();
	}
	else {
		// Error - wrong number of parameters.
		std::string error = "The syntax of the command is incorrect. \n";
		kiv_os::printErr(error.c_str(), error.length());
	}

	return 0;
}


size_t __stdcall shutdown(const kiv_os::TRegisters &regs) 
{
	int argc;
	char **argv = kiv_os::getArgs("shutdown", regs, &argc);
	return shutdown_main(argc, argv);
}


