#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"


size_t shutdown_main(int argc, char *argv[]) {
	if (argc == 1) {
		kiv_os_rtl::Shutdown();
	}
	else {
		// Error - wrong number of parameters.
		std::string error = "The syntax of the command is incorrect. \n";
		kiv_os_lib::printErr(error.c_str(), error.length());
	}

	return 0;
}


size_t __stdcall shutdown(const kiv_os::TRegisters &regs) 
{
	std::vector<std::string> args = kiv_os_lib::getArgs("shutdown", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return shutdown_main(static_cast<int>(argv.size()), argv.data());
}


