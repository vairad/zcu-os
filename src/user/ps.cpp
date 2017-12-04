#include "common.h"
#include "rtl.h"

#include <string>
#include <vector>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace ps_program
{
	int printProcesses(bool pid_flag) 
	{
		std::string path = "procfs:/";
		kiv_os::THandle handle;
		const bool success = kiv_os_rtl::Create_File(path.c_str(), kiv_os::fmOpen_Always, handle);
		if (!success) {
			// Error - File does not exist.
			std::string error = "File not found.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return kiv_os_lib::FILE_NOT_FOUND;
		}

		bool isDir;
		uint16_t attrs;
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
		}
		else {
			std::string s = std::to_string(attrs);
			s.append("\t");
			kiv_os_lib::print(s.c_str(), s.length());
			kiv_os_lib::printLn(path.c_str(), path.length());
		}
	}

	void printHelp()
	{
		std::string help_message = "Print all running processes";
		help_message += "\r\n";
		help_message += "Parameter -pid show pid of processes";
		help_message += "\r\n";
		help_message += "Parameter -h show help.";

		kiv_os_lib::printLn(help_message.c_str(), help_message.size());
	}

	size_t ps_main(int argc, char **argv)
	{
		if (argc >= 1 && argc <= 2) {
			return printProcesses(false);
		}
		else {
			if (strcmp(argv[1], "-pid") == 0 ) {
				return printProcesses(true);
			}
			if (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0) {
				printHelp();
				return 0;
			}
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}
	}
}

size_t __stdcall ps(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("ps", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return ps_program::ps_main(static_cast<int>(argv.size()), argv.data());
}
