#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace type_program {

	size_t type_main(int argc, char **argv) {
		if (argc > 1) {
			for (size_t i = 1; i < argc; i++) {
				kiv_os::THandle file = kiv_os_rtl::Create_File(argv[i], kiv_os::fmOpen_Always);
				if (file == kiv_os::erInvalid_Handle) {
					// TODO: Klaus - Handle error.
					continue;
				}

				size_t read = 1;
				char buffer[1024];
				while (read != -1) {
					bool ok = kiv_os_rtl::Read_File(file, buffer, sizeof(buffer) - 1, read);
					if (!ok) {
						// TODO: Handle read error.
						continue;
					}
					buffer[read] = 0; // Terminate the string.
					kiv_os_lib::printLn(buffer, read);
				}

				size_t written = kiv_os_lib::print(buffer, read);
			}
		} else {
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall type(const kiv_os::TRegisters &regs)
{ 
	int argc;
	char **argv = kiv_os_lib::getArgs("type", regs, &argc);
	return type_program::type_main(argc, argv);
}