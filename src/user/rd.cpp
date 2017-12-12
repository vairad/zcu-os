#include <vector>
#include <string>

#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace rd_program {

	void incorrectSyntax() {
		std::string error = "The syntax of the command is incorrect.";
		kiv_os_lib::printErrLn(error.c_str(), error.length());
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
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::READ_ERROR;
		}
		buffer[read] = 0; // Terminate the string.

		if (buffer[0] == 'Y' || buffer[0] == 'y') {
			return 1;
		}
		return 0;
	}

	size_t rd_main(int argc, char *argv[]) {
		if (argc > 1) {
			bool quiet = false;
			bool recursive = false;
			std::vector<char *> dirnameVector = std::vector<char *>();

			for (size_t i = 1; i < argc; i++) {
				std::string str = argv[i];
				if (str == "/S" || str == "/s") {
					recursive = true;
				}
				else if (str == "/Q" || str == "/q") {
					quiet = true;
				}
				else {
					dirnameVector.push_back(argv[i]);
				}
			}

			if (dirnameVector.empty()) {
				incorrectSyntax();
				return kiv_os_lib::INCORRECT_SYNTAX;
			}

			for (size_t i = 0; i < dirnameVector.size(); i++) {
				kiv_os::THandle dir;
				const bool success = kiv_os_rtl::Create_File(dirnameVector[i], kiv_os::fmOpen_Always, kiv_os::faDirectory, dir);
				if (!success) {
					// Error - File not found.
					std::string error = "File not found.";
					kiv_os_lib::printErrLn(error.c_str(), error.length());
					return kiv_os_lib::FILE_NOT_FOUND;
				}

				bool isDir;
				const bool ok = kiv_os_lib::isDir(dir, isDir, nullptr);
				if (!ok) {
					// Error - Cannot get attrs.
					std::string error = "Could not get file attributes.";
					kiv_os_lib::printErrLn(error.c_str(), error.length());
					return kiv_os_lib::ATTRS_ERROR;
				}

				if (isDir) {
					size_t read;
					kiv_os::TDir_Entry tdir;
					const bool read_ok = kiv_os_rtl::Read_File(dir, &tdir, sizeof(kiv_os::TDir_Entry), read);
					if (!read_ok) {
						// Error - Bad read.
						std::string error = "Error reading file.";
						kiv_os_lib::printErrLn(error.c_str(), error.length());
						return kiv_os_lib::READ_ERROR;
					}
					bool empty = read == 0;
					size_t result = false;
					if(!quiet)
					{
						result = ask(dirnameVector[i]);
						if (result == kiv_os_lib::READ_ERROR) {
							// Error - Bad read.
							std::string error = "Error reading from standard input.";
							kiv_os_lib::printErrLn(error.c_str(), error.length());
							return kiv_os_lib::READ_ERROR;
						}
					}

					bool answer = result;
					if ((empty || recursive) && (quiet || answer)) {
						deleteDir(dir, dirnameVector[i]);
					}
					if (!empty && !recursive) {
						// Error - folder is not empty. 
						std::string error = "The directory is not empty.";
						kiv_os_lib::printErrLn(error.c_str(), error.length());
						return kiv_os_lib::DIR_NOT_EMPTY;
					}
				} else {
					if (quiet || ask(dirnameVector[i])) {
						deleteDir(dir, dirnameVector[i]);
					}
				}
			}
		}
		else {
			// Error - wrong number of parameters.
			incorrectSyntax();
			return kiv_os_lib::INCORRECT_SYNTAX;
		}
		return kiv_os_lib::SUCCESS;
	}
}


size_t __stdcall rd(const kiv_os::TRegisters &regs) {
	std::vector<std::string> args = kiv_os_lib::getArgs("rd", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return rd_program::rd_main(static_cast<int>(argv.size()), argv.data());
}
