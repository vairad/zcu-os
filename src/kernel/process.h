#pragma once

#include "../api/api.h"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

#include "process_api.h"

#include "waiting_queue.h"
#include "thread_state.h"
#include "retval.h"

/// One line from process control block table
struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle parent_pid;
	std::string program_name;
	std::string working_directory;
	std::vector<kiv_os::THandle> io_devices;
	process::process_waiting_queue waiting;
	process::retval retval;
};

/// One line from thread control block table
struct TCB {
	kiv_os::THandle tid;
	kiv_os::THandle pid;
	process::thread_waiting_queue waiting;
	process::thread_state state;
	process::retval retval;
	std::thread thread;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

bool subroutineCreateProcess(kiv_os::TRegisters & context);
bool subroutineCreateThread(kiv_os::TRegisters & context);

bool addToWaitingQueue(kiv_os::THandle handle);

bool waitForProcess(const kiv_os::THandle handle);

bool waitForThread(const kiv_os::THandle handle);

bool validateHandle(const kiv_os::THandle handle);

//helpful subroutines
kiv_os::THandle createProcessThread(kiv_os::THandle pid, kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context);
kiv_os::THandle createThread(kiv_os::THandle pid, kiv_os::TThread_Proc entry_point, void * data);
kiv_os::THandle getNextFreePid();
kiv_os::THandle getNextFreeTid();
kiv_os::THandle TidToTableIndex(const kiv_os::THandle tid);
std::shared_ptr<PCB> createFreePCB(kiv_os::THandle pid);
std::shared_ptr<TCB> createFreeTCB(kiv_os::THandle tid, kiv_os::THandle pid);
void initialisePCB(std::shared_ptr<PCB>new_pcb , char * program_name, kiv_os::TProcess_Startup_Info * startup_info);

void addRecordToThreadMap(const kiv_os::THandle tid);


// Help Routines
void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & context);


size_t GetRetVal(kiv_os::THandle handle);

void process0(process::TStartBlock &procInfo);
void thread0(process::TStartBlock &threadInfo);

void cleanProcess(const kiv_os::THandle handle);
void cleanThread(const kiv_os::THandle table_index);
