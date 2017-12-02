#pragma once


#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"


bool HandleProcess(kiv_os::TRegisters &context);

namespace process
{
//base functions process management
	// get TID
	kiv_os::THandle getTid();
	// get PID
	kiv_os::THandle getPid();
	// get Parent pid
	kiv_os::THandle getParentPid();
	// get Working directory
	std::string getWorkingDir();
	// notify selected handle
	void wakeUpThreadHandle(const kiv_os::THandle handle);
	// notify selected handle
	void wakeUpProcessHandle(const kiv_os::THandle handle);

//proceses init and destruct for kernel start and end
	bool createInit();
	bool destructInit();


//interface for io module
	// method changes (aprooved) working directory of program
	bool changeWorkingDir(const std::string new_dir);

	//FD interface for io
	kiv_os::THandle getSystemFD(const kiv_os::THandle program_handle);
	kiv_os::THandle setNewFD(const kiv_os::THandle system_handle);
	void removeProcessFD(const kiv_os::THandle program_handle);

}
