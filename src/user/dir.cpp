#include "common.h"
#include "rtl.h"

#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace dir_program {

	size_t dir_main(int argc, char **argv) {
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
				return kiv_os_lib::FILE_NOT_FOUND;
			}

			bool isDir;
			uint8_t attrs;
			bool ok = kiv_os_lib::isDir(handle, isDir, &attrs);
			if (!ok) {
				// Error - Get args fail.
				std::string error = "Could not get file attributes.";
				kiv_os_lib::printErr(error.c_str(), error.length());
				return kiv_os_lib::ATTRS_ERROR;
			}

			if (isDir) {
				size_t read;
				kiv_os::TDir_Entry tdir;
				bool ok;
				while ((ok = kiv_os_rtl::Read_File(handle, &tdir, sizeof(kiv_os::TDir_Entry), read)) && read != -1) {
					std::string s = std::to_string(tdir.file_attributes);
					s.append("\t");
					kiv_os_lib::print(s.c_str(), s.length());
					kiv_os_lib::printLn(tdir.file_name, strlen(tdir.file_name));
				}
			} else {
				std::string s = std::to_string(attrs);
				s.append("\t");
				kiv_os_lib::print(s.c_str(), s.length());
				kiv_os_lib::printLn(path.c_str(), path.length());
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

size_t __stdcall dir(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("dir", regs, &argc);
	return dir_program::dir_main(argc, argv);
}
