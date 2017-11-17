#pragma once

#include "../api/api.h"

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <map>


/// One line from process control block table
struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle parent_pid;
	std::string program_name;
	std::string working_directory;
	std::vector<kiv_os::THandle> io_devices;
};

/// One line from thread control block table
struct TCB {
	kiv_os::THandle tid;
	kiv_os::THandle pid;
	std::thread thread;
};

/// Table of running processes
extern std::shared_ptr<PCB> process_table[];

/// Table of running threads - declare global variable
extern std::shared_ptr<TCB> thread_table[];

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction
bool HandleProcess(kiv_os::TRegisters &context);

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

bool subroutineCreateProcess(kiv_os::TRegisters & context);
bool subroutineCreateThread(kiv_os::TRegisters & context);

//helpful subroutines
kiv_os::THandle createProcessThread(kiv_os::THandle pid, kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context);
kiv_os::THandle createThread(kiv_os::THandle pid, kiv_os::TThread_Proc entry_point, void * data);
kiv_os::THandle getNextFreePid();
kiv_os::THandle getNextFreeTid();
std::shared_ptr<PCB> createFreePCB(kiv_os::THandle pid);
std::shared_ptr<TCB> createFreeTCB(kiv_os::THandle tid, kiv_os::THandle pid);
void initialisePCB(std::shared_ptr<PCB>new_pcb , char * program_name, kiv_os::TProcess_Startup_Info * startup_info);

// get TID
kiv_os::THandle getTid();

// get PID
kiv_os::THandle getPid();

// get Parent pid
kiv_os::THandle getParentPid();

// get Working directory
std::string getWorkingDir();

// Help Routines
void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & context);
