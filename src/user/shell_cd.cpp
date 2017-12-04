#include "shell_cd.h"
#include "rtl.h"
#include "common.h"

#undef stdin
#undef stderr
#undef stdout

namespace shell_cd {

	bool cd(shell_executor::CommandExecute command, std::string args) {
		std::vector<std::string> params = command.parameters;
		bool success;

		if (params.empty()) {
			char wd[256];
			success = kiv_os_lib::getWD(wd, sizeof(wd));

			kiv_os_lib::printLn(wd, strlen(wd));
		} else if (params.size() == 1) {
			success = kiv_os_rtl::Change_Working_Dir(args.c_str());
			if( ! success )
			{
				std::string error = "System could not find selected path";
				kiv_os_lib::printErr(error.c_str(), error.length());
			}
		} else {
			std::string error = "Invalid parameters.";
			kiv_os_lib::printErr(error.c_str(), error.length());
			success = false;
		}
		return success;
	}

}
