#include "shell_cd.h"
#include "rtl.h"

bool kiv_os::cd(kiv_os::CommandExecute command) {
	std::vector<std::string> params = command.parameters;
	kiv_os::THandle std_out = command.std_out;

	if (params.empty()) {
		char wd[256]; // TODO: Klaus - What size should the buffer be?
		size_t read;
		// TODO: Klaus - Handle bad read.
		bool ok = kiv_os_rtl::Get_Working_Dir(wd, sizeof(wd) - 1, read);
		wd[read] = 0; // Terminate the line.
		size_t written;
		kiv_os_rtl::Write_File(std_out, wd, strlen(wd), written);
	} else {
		// TODO: Klaus - Change working directory.
	}
	return false;
}
