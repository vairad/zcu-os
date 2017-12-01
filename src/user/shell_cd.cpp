#include "shell_cd.h"
#include "rtl.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout

bool kiv_os::cd(kiv_os::CommandExecute command, std::string args) {
	std::vector<std::string> params = command.parameters;
	kiv_os::THandle std_out = command.std_out;
	bool retVal = false;

	if (params.empty()) {
		char wd[256];
		size_t read;
		// TODO: Klaus - Handle bad read.
		retVal = kiv_os_rtl::Get_Working_Dir(wd, sizeof(wd) - 1, read);
		wd[read] = 0; // Terminate the line.
		kiv_os_lib::print(wd, strlen(wd));
	} else if (params.size() == 1) {
		retVal = kiv_os_rtl::Change_Working_Dir(args.c_str());
		// TODO: Klaus - Handle error.
	} else {
		std::string error = "Invalid parameters.";
		kiv_os_lib::printErr(error.c_str(), error.length());
		retVal = false;
	}
	return retVal;
}
