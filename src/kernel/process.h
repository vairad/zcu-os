#pragma once

#include "../api/api.h"

#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include "waiting_queue.h"
#include "thread_state.h"

namespace process
{
	class TStartBlock
	{
	public:
		bool is_process;
		union
		{
			kiv_os::TEntry_Point proc;
			kiv_os::TThread_Proc thread;
		} entry_point;
		union
		{
			kiv_os::TRegisters proc;
			void * thread;
		} context;
		
		TStartBlock(const kiv_os::TEntry_Point entry_point, kiv_os::TRegisters &context)
			: is_process(true)
		{
			this->entry_point.proc = entry_point;
			this->context.proc = context;
		}

		TStartBlock(const kiv_os::TThread_Proc entry_point, void *context)
			: is_process(false)
		{
			this->entry_point.thread = entry_point;
			this->context.thread = context;
		}
	};

	void wakeUpHandle(kiv_os::THandle handle);

	// get TID
	kiv_os::THandle getTid();

	// get PID
	kiv_os::THandle getPid();

	// get Parent pid
	kiv_os::THandle getParentPid();

	// get Working directory
	std::string getWorkingDir();
}

/// One line from process control block table
struct PCB {
	kiv_os::THandle pid;
	kiv_os::THandle parent_pid;
	std::string program_name;
	std::string working_directory;
	std::vector<kiv_os::THandle> io_devices;
	process::waiting_queue waiting;
};

/// One line from thread control block table
struct TCB {
	kiv_os::THandle tid;
	kiv_os::THandle pid;
	process::waiting_queue waiting;
	process::thread_state state;
	std::thread thread;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction
bool HandleProcess(kiv_os::TRegisters &context);

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

bool subroutineCreateProcess(kiv_os::TRegisters & context);
bool subroutineCreateThread(kiv_os::TRegisters & context);

kiv_os::THandle addToWaitingQueue(kiv_os::THandle handle);

kiv_os::THandle waitForProcess(const kiv_os::THandle handle);

kiv_os::THandle waitForThread(const kiv_os::THandle handle);

bool validateHandle(const kiv_os::THandle handle);

//helpful subroutines
kiv_os::THandle createProcessThread(kiv_os::THandle pid, kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context);
kiv_os::THandle createThread(kiv_os::THandle pid, kiv_os::TThread_Proc entry_point, void * data);
kiv_os::THandle getNextFreePid();
kiv_os::THandle getNextFreeTid();
std::shared_ptr<PCB> createFreePCB(kiv_os::THandle pid);
std::shared_ptr<TCB> createFreeTCB(kiv_os::THandle tid, kiv_os::THandle pid);
void initialisePCB(std::shared_ptr<PCB>new_pcb , char * program_name, kiv_os::TProcess_Startup_Info * startup_info);

// Help Routines
void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & context);


void process0(process::TStartBlock &procInfo);

void thread0(process::TStartBlock &threadInfo);
