#pragma once


#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"


bool HandleProcess(kiv_os::TRegisters &context);

namespace process
{
	// get TID
	kiv_os::THandle getTid();

	// get PID
	kiv_os::THandle getPid();

	// get Parent pid
	kiv_os::THandle getParentPid();

	// get Working directory
	std::string getWorkingDir();

	// method changes (aprooved) working directory of program
	bool changeWorkingDir(const std::string new_dir);

	bool createInit();
	bool destructInit();
	void wakeUpThreadHandle(const kiv_os::THandle handle);
	void wakeUpProcessHandle(const kiv_os::THandle handle);

	//FD interface for io
	kiv_os::THandle getSystemFD(const kiv_os::THandle program_handle);
	kiv_os::THandle setNewFD(const kiv_os::THandle system_handle);
	void removeProcessFD(const kiv_os::THandle program_handle);

}
