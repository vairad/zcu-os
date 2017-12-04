#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace md_program {

	bool createFile(char * file)
	{
		kiv_os::THandle handle = kiv_os::erInvalid_Handle;
		const bool succes = kiv_os_rtl::Create_File(file, 0, kiv_os::faDirectory, handle);
		if (succes)
		{
			kiv_os_rtl::Close_File(handle);
			return true;
		}
		std::string error = "Directory " + std::string(file) + " could not be created";
		kiv_os_lib::printErrLn(error.c_str(), error.length());
		return false;
	}

	size_t md_main(int argc, char *argv[]) {
		if (argc > 1) {
			bool success = true;
			for (int i = 1; i < argc; ++i)
			{
				success &= createFile(argv[i]);
			}
			if (!success)
			{
				return kiv_os_lib::FILE_NOT_CREATED;
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

size_t __stdcall md(const kiv_os::TRegisters &regs) {
	std::vector<std::string> args = kiv_os_lib::getArgs("md", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return md_program::md_main(static_cast<int>(argv.size()), argv.data());
}
