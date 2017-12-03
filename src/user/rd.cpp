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
	const size_t FILE_NOT_FOUND = 2;
	const size_t READ_ERROR = 3;
	const size_t ATTRS_ERROR = 4;
	const size_t DIR_NOT_EMPTY = 5;


	void incorrectSyntax() {
		std::string error = "The syntax of the command is incorrect.";
		kiv_os_lib::printErr(error.c_str(), error.length());
	}

	void deleteDir(kiv_os::THandle dir, char *dirname) {
		// TODO: Klaus - Handle errors.
		kiv_os_rtl::Close_File(dir);
		kiv_os_rtl::Delete_File(dirname);
	}

	size_t ask(char *dirname) {
		std::string question = "Do you want to delete \'";
		question.append(dirname);
		question.append("\'? (Y / N): ");
		kiv_os_lib::print(question.c_str(), question.length());
		char buffer[5];
		size_t read = kiv_os_lib::read(buffer, sizeof(buffer) - 1);
		if (read == -1) {
			// Error - Bad read.
			std::string error = "Error reading from standard input.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return READ_ERROR;
		}
		buffer[read] = 0; // Terminate the string.

		if (buffer == "Y" || buffer == "y") {
			return 1;
		}
		return 0;
	}

	size_t rd_main(int argc, char **argv) {
		if (argc > 1 && argc <= 4) {
			bool quiet = true;
			bool recursive = false;
			char *dirname = nullptr;

			for (size_t i = 1; i < argc; i++) {
				// TODO: Klaus - Test this.
				char *str = argv[i];
				if (str == "/S" || str == "/s") {
					recursive = true;
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

			kiv_os::THandle dir = kiv_os_rtl::Create_File(dirname, kiv_os::fmOpen_Always);
			if (dir == kiv_os::erInvalid_Handle) {
				// Error - File not found.
				std::string error = "File not found.";
				kiv_os_lib::printErr(error.c_str(), error.length());
				return FILE_NOT_FOUND;
			}

			bool isDir;
			bool ok = kiv_os_lib::isDir(dir, isDir, nullptr);
			if (!ok) {
				// Error - Cannot get attrs.
				std::string error = "Could not get file attributes.";
				kiv_os_lib::printErr(error.c_str(), error.length());
				return ATTRS_ERROR;
			}

			if (isDir) {
				size_t read;
				kiv_os::TDir_Entry tdir;
				bool ok = kiv_os_rtl::Read_File(dir, &tdir, sizeof(kiv_os::TDir_Entry), read);
				if (!ok) {
					// Error - Bad read.
					std::string error = "Error reading file.";
					kiv_os_lib::printErr(error.c_str(), error.length());
					return READ_ERROR;
				}
				bool empty = read == -1;
				size_t result = ask(dirname);
				if (result == READ_ERROR) {
					// Error - Bad read.
					std::string error = "Error reading from standard input.";
					kiv_os_lib::printErr(error.c_str(), error.length());
					return READ_ERROR;
				}

				bool answer = result;
				if ((empty || recursive) && (quiet || answer)) {
					deleteDir(dir, dirname);
				}
				if (!empty && !recursive) {
					// Error - folder is not empty. 
					std::string error = "The directory is not empty.";
					kiv_os_lib::printErr(error.c_str(), error.length());
					return DIR_NOT_EMPTY;
				}
			} else {
				if (quiet || ask(dirname)) {
					deleteDir(dir, dirname);
				}
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
	return rd_program::rd_main(argc, argv);
}
