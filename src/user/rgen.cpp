#include "common.h"
#include "rtl.h"

#include <cstdlib>
#include <ctime>
#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace rgen_program {

	bool run_generator;

	size_t __stdcall generateNumbers(const void *data) {
		bool * run = (bool *) data;
		
		srand((unsigned) time(0));
		while ( *run ) {
			double r = rand();
			double num = r / RAND_MAX;
			std::string s = std::to_string(num);
			kiv_os_lib::printLn(s.c_str(), s.length());
		}

		return kiv_os_lib::SUCCESS;
	}

	size_t rgen_main(int argc, char *argv[]) {
		bool run = true;

		kiv_os::THandle handle;
		bool ok = kiv_os_rtl::Create_Thread(&handle, &generateNumbers, &run);
		if (!ok) {
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::CANNOT_CREATE_THREAD;
		}

		size_t read;
		size_t buffer_size = 255;
		char buffer[256];
		while ((read = kiv_os_lib::read(buffer, buffer_size)) != -1) {
			// Nothing, wait for EOF.
		}
		run = false;

		kiv_os_rtl::Join_One_Handle(handle);

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall rgen(const kiv_os::TRegisters &regs) {
	std::vector<std::string> args = kiv_os_lib::getArgs("rgen", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return rgen_program::rgen_main(static_cast<int>(argv.size()), argv.data());
}
