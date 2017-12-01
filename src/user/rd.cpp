#include "common.h"
#include "rtl.h"

#include <vector>
#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace rd_program
{
	const size_t SUCCESS = 0;
	const size_t INCORRECT_SYNTAX = 1;
	const size_t DIR_IS_NOT_EMPTY = 2;


	void incorrectSyntax() {
		std::string error = "The syntax of the command is incorrect.";
		kiv_os_lib::printErr(error.c_str(), error.length());
	}

	void deleteDir(kiv_os::THandle dir, char *dirname) {
		// TODO: Klaus - Handle errors.
		kiv_os_rtl::Delete_File(dirname);
		kiv_os_rtl::Close_File(dir);
	}

	size_t rd_main(int argc, char **argv) {
		if (argc > 0 && argc <= 3) {
			bool quiet = true;
			bool recurive = false;
			char *dirname = nullptr;

			for (size_t i = 0; i < argc; i++) {
				// TODO: Klaus - Test this.
				char *str = argv[i];
				if (str == "/S" || str == "/s") {
					recurive = true;
				}
				else if (str == "/Q" || str == "/q") {
					quiet = false;
				}
				else {
					dirname = str;
				}
			}

			if (dirname == nullptr) {
				incorrectSyntax();
				return INCORRECT_SYNTAX;
			}

			// TODO: Klaus - Handle errors.
			kiv_os::THandle dir = kiv_os_rtl::Create_File(dirname, kiv_os::fmOpen_Always);
			size_t read;
			char buffer[5];
			bool ok = kiv_os_rtl::Read_File(dir, buffer, sizeof(buffer) - 1, read);
			buffer[read] = 0; // Terminate the string.
			if (read == 0 || recurive) {
				if (!quiet) {
					std::string question = "Do you want to delete \'";
					question.append(dirname);
					question.append("\'? (Y / N): ");
					kiv_os_lib::print(question.c_str(), question.length());
					read = kiv_os_lib::read(buffer, sizeof(buffer) - 1);
					buffer[read] = 0; // Terminate the string.

					if (buffer == "Y" || buffer == "y") {
						deleteDir(dir, dirname);
					}
				}
				else {
					deleteDir(dir, dirname);
				}
			}
			else {
				// Error - folder is not empty.
				std::string error = "The directory is not empty.";
				kiv_os_lib::printErr(error.c_str(), error.length());
				return DIR_IS_NOT_EMPTY;
			}
		}
		else {
			// Error - wrong number of parameters.
			incorrectSyntax();
			return INCORRECT_SYNTAX;
		}
		return SUCCESS;
	}
}


size_t __stdcall rd(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os_lib::getArgs("rd", regs, &argc);
	rd_program::rd_main(argc, argv);
	return 0;
}
