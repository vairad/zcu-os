#include <string>
#include <vector>

#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"


namespace ps_program
{
	const std::string proc_fs_path = "procfs:/";

	void printProcess(std::string name, std::string pid, bool pid_flag)
	{
		std::string line = "\t";
		line += name;
		if(pid_flag)
		{
			line += "\t";
			line += pid;
		}
		kiv_os_lib::printLn(line.c_str(), line.size());
	}

	bool browseAndPrint(const kiv_os::THandle handle, const bool pid_flag)
	{
		bool whole_success = true;
		size_t read;
		kiv_os::TDir_Entry tdir;
		bool ok = kiv_os_rtl::Read_File(handle, &tdir, sizeof(kiv_os::TDir_Entry), read);
		whole_success &= ok;
		ok &= (read != -1);
		while (ok)
		{
			std::string processPid = tdir.file_name;
			std::string processPath = proc_fs_path + processPid;
			kiv_os::THandle handle_file;
			char buffer[128];
			bool success = kiv_os_rtl::Create_File(processPath.c_str(), 0, 0, handle_file);
			whole_success &= success;
			std::string processLn = "";
			while (success)
			{
				success = kiv_os_rtl::Read_File(handle_file, &buffer, sizeof(buffer) - 1, read);
				if (read == 0 || read >= sizeof(buffer))
				{
					break;
				}
				buffer[read] = 0;
				processLn += buffer;
				memset(buffer, 0, sizeof(buffer)); // clear buffer
			}
			printProcess(processLn, processPid, pid_flag);

			ok = kiv_os_rtl::Read_File(handle, &tdir, sizeof(kiv_os::TDir_Entry), read);
			ok &= (read != -1);
		}
		return whole_success;
	}

	bool openProcfsRoot(kiv_os::THandle &handle)
	{
		bool success = kiv_os_rtl::Create_File(proc_fs_path.c_str(), 0, kiv_os::faDirectory, handle);
		if (!success) {
			// Error - File does not exist.
			std::string error = "File not found.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
		}
		return success;
	}

	bool verifyProcfsRoot(const kiv_os::THandle handle)
	{
		bool isDir;
		uint16_t attrs;
		bool ok = kiv_os_lib::isDir(handle, isDir, &attrs);
		if (!ok) {
			// Error - Get args fail.
			std::string error = "Could not get file attributes.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::ATTRS_ERROR;
		}
		return ok;
	}

	int printProcesses(bool pid_flag) 
	{
		kiv_os::THandle handle;
		
		bool success = openProcfsRoot(handle);
		
		success &= verifyProcfsRoot(handle);
		
		if (!success) { return kiv_os_lib::FILE_NOT_FOUND; }

		success = browseAndPrint(handle, pid_flag);
		
		if (!success) { return kiv_os_lib::FILE_NOT_FOUND; }
		return kiv_os_lib::SUCCESS;
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

	size_t ps_main(int argc, char *argv[])
	{
		if (argc == 1) {
			return printProcesses(false);
		}
		
		if( argc == 2)
		{
			if (strcmp(argv[1], "-pid") == 0 ) {
				return printProcesses(true);
			}
			if (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0) {
				printHelp();
				return 0;
			}
			// Error - wrong number of parameters.
			std::string error = "The syntax of the command is incorrect.";
			kiv_os_lib::printErrLn(error.c_str(), error.length());
			return kiv_os_lib::INCORRECT_SYNTAX;
		}
		// Error - wrong number of parameters.
		std::string error = "The syntax of the command is incorrect.";
		kiv_os_lib::printErrLn(error.c_str(), error.length());
		return kiv_os_lib::INCORRECT_SYNTAX;
	}
}

size_t __stdcall ps(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("ps", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return ps_program::ps_main(static_cast<int>(argv.size()), argv.data());
}
