#pragma once

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <map>

#include "../api/api.h"

/// One line from process control block table
struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle parent_pid;
	std::thread thread;
	std::string program_name;
	std::string working_directory;
	std::vector<kiv_os::THandle> io_devices;
};

/// Table of running processes
extern std::shared_ptr<PCB> process_table[];

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction
bool HandleProcess(kiv_os::TRegisters &context);

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

bool subroutineCreateProcess(kiv_os::TRegisters & context);

bool subroutineCreateThread(kiv_os::TRegisters & context);

// get PID
kiv_os::THandle getPid();

// get Parent pid
kiv_os::THandle getParentPid();

// get Working directory
std::string getWorkingDir();

// Help Routines
void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & context);
