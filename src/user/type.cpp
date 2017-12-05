#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace type_program {

	const size_t BUFFER_SIZE = 1024;

	void printErr(char * filename)
	{
		std::string message = "Error occured while processing: " + std::string(filename);
		kiv_os_lib::printErrLn(message.c_str(), message.size());
	}

	size_t type_main(int argc, char *argv[]) {
		if (argc > 1) {
			for (size_t i = 1; i < argc; i++)
			{
				kiv_os::THandle file;
				const bool success = kiv_os_rtl::Create_File(argv[i], kiv_os::fmOpen_Always, 0, file);
				if (!success) {
					printErr(argv[i]);
					continue;
				}

				size_t read = 1;
				size_t written = 0;
				char buffer[BUFFER_SIZE];
				while (read != -1) {
					bool ok = kiv_os_rtl::Read_File(file, buffer, sizeof(buffer) - 1, read);
					if ( !ok || read == 0) {
						if (written == 0) {
							printErr(argv[i]);
						}
						break;
					}
					buffer[read] = 0; // Terminate the string.
					written += kiv_os_lib::print(buffer, read); //do not break line print what exactly is in file
					memset(buffer, 0, sizeof(buffer)); // null buffer for sure
				}
				kiv_os_lib::printLn(buffer, 0);
				kiv_os_rtl::Close_File(file);
			}
		} else {
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall type(const kiv_os::TRegisters &regs)
{ 
	std::vector<std::string> args = kiv_os_lib::getArgs("type", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return type_program::type_main(static_cast<int>(argv.size()), argv.data());
}