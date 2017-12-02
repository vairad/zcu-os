#include "common.h"
#include "rtl.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace dir_program {

	void dir_main(int argc, char **argv) {
		if (argc >= 1 && argc <= 2) {
			std::string path = "";
			if (argc == 1) {
				path = ".";
			} else {
				path = argv[1];
			}

			kiv_os::THandle handle = kiv_os_rtl::Create_File(path.c_str(), kiv_os::fmOpen_Always);
			if (handle == kiv_os::erInvalid_Handle) {
				// Error - File does not exist.
				std::string error = "File not found.";
				kiv_os_lib::printErr(error.c_str(), error.length());
			}

			size_t read = 1;
			char buffer[256];
			while (read != -1) {
				// TODO: Klaus - Handle listing file.
				bool ok = kiv_os_rtl::Read_File(handle, buffer, sizeof(buffer) - 1, read);
				if (!ok) {
					// TODO: Handle read error.
					return;
				}
				buffer[read] = 0; // Terminate the string.
				kiv_os_lib::printLn(buffer, strlen(buffer));
			}
		} else {
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
		}
	}

}

size_t __stdcall md(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("dir", regs, &argc);
	dir_program::dir_main(argc, argv);
	return 0;
}
