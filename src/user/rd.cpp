#include "common.h"
#include "rtl.h"

#include <vector>
#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

void incorrectSyntax() {
	std::string error = "The syntax of the command is incorrect.";
	kiv_os::printErr(error.c_str(), error.length());
}

void deleteDir(kiv_os::THandle dir, char *dirname) {
	// TODO: Klaus - Handle errors.
	kiv_os_rtl::Delete_File(dirname);
	kiv_os_rtl::Close_File(dir);
}

void rd_main(int argc, char **argv) {
	if (argc > 0 && argc <= 3) {
		bool quiet = true;
		bool recurive = false;
		char *dirname = nullptr;

		for (size_t i = 0; i < argc; i++) {
			// TODO: Klaus - Test this.
			char *str = argv[i];
			if (str == "/S" ||str == "/s") {
				recurive = true;
			} else if (str == "/Q" || str == "/q") {
				quiet = false;
			} else {
				dirname = str;
			}
		}

		if (dirname == nullptr) {
			incorrectSyntax();
			return;
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
				kiv_os::print(question.c_str(), question.length());
				read = kiv_os::read(buffer, sizeof(buffer) - 1);
				buffer[read] = 0; // Terminate the string.

				if (buffer == "Y" || buffer == "y") {
					deleteDir(dir, dirname);
				}
			} else {
				deleteDir(dir, dirname);
			}
		} else {
			// Error - folder is not empty.
			std::string error = "The directory is not empty.";
			kiv_os::printErr(error.c_str(), error.length());
		}
	} else {
		// Error - wrong number of parameters.
		incorrectSyntax();
	}
}

size_t __stdcall rd(const kiv_os::TRegisters &regs)
{
	int argc;
	char **argv = kiv_os::getArgs(regs, &argc);
	rd_main(argc, argv);
	return 0;
}
