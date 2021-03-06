#pragma once
#include <vector>

#include "waiting_queue.h"
#include "thread_state.h"
#include "retval.h"

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"


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


namespace process
{
	class TStartProcessBlock
	{
	public:
		kiv_os::THandle tid;
		std::string args;
		kiv_os::TEntry_Point entry_point;
		kiv_os::TRegisters proc;
		kiv_os::TProcess_Startup_Info startup_info;

		TStartProcessBlock(const kiv_os::THandle tid, const kiv_os::TEntry_Point entry_point, kiv_os::TRegisters &context)
			: tid(tid)
		{
			this->entry_point = entry_point;
			this->proc = context;
			kiv_os::TProcess_Startup_Info * startup_info = reinterpret_cast<kiv_os::TProcess_Startup_Info *>(context.rdi.r);
			this->startup_info.stdin = startup_info->stdin;
			this->startup_info.stdout = startup_info->stdout;
			this->startup_info.stderr = startup_info->stderr;

			args = startup_info->arg;
			this->startup_info.arg = const_cast<char *>(args.c_str());
		}

		kiv_os::TProcess_Startup_Info *getProcInfo()
		{
			this->startup_info.arg = (char *)(args.c_str());
			return &(this->startup_info);
		}
	};

	class TStartThreadBlock
	{
	public:
		kiv_os::THandle tid;
		std::string args;

		kiv_os::TThread_Proc entry_point;
		void * context;

		TStartThreadBlock(const kiv_os::THandle tid, const kiv_os::TThread_Proc entry_point, void *context)
			: tid(tid)
		{
			this->entry_point = entry_point;
			this->context = context;
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Working routines - process creation
bool routineCloneProcess(kiv_os::TRegisters &context);
bool subroutineCreateProcess(kiv_os::TRegisters & context);
bool subroutineCreateThread(kiv_os::TRegisters & context);

// Working routines - wait for handle
bool routineWaitForProcess(kiv_os::TRegisters &context);
bool addToWaitingQueue(kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done);
bool waitForProcess(const kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done);
bool waitForThread(const kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done);
size_t GetRetVal(kiv_os::THandle handle);

// Working routines - shutodwn
bool routineShutdown(const kiv_os::TRegisters& context);
bool stopHandle(const kiv_os::THandle handle);
bool stopThread(const kiv_os::THandle tid);
bool stopProcess(const kiv_os::THandle pid);

//helpful subroutines - create process
kiv_os::THandle createProcessThread(kiv_os::THandle pid, kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context);
kiv_os::THandle createThread(kiv_os::THandle pid, kiv_os::TThread_Proc entry_point, void * data);
void initialisePCB(std::shared_ptr<PCB>new_pcb, char * program_name, kiv_os::TProcess_Startup_Info * startup_info);
std::shared_ptr<PCB> createFreePCB(kiv_os::THandle pid);
std::shared_ptr<TCB> createFreeTCB(kiv_os::THandle tid, kiv_os::THandle pid);
kiv_os::THandle getNextFreePid();
kiv_os::THandle getNextFreeTid();

// Help Routines
void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & context);
bool validateHandle(const kiv_os::THandle handle);
kiv_os::THandle TidToTableIndex(const kiv_os::THandle tid);
void addRecordToThreadMap(const kiv_os::THandle tid);
kiv_os::THandle resolveAndDuplicateFD(const kiv_os::THandle system_FD);

//wrapper for run thread/process
void process0(process::TStartProcessBlock &procInfo);
void thread0(process::TStartThreadBlock &threadInfo);

// cleaning methods
void cleanProcess(const kiv_os::THandle handle);
void cleanThread(const kiv_os::THandle table_index);

// procfs methods
bool addProcessToProcFS(std::string name, kiv_os::THandle pid);
bool remProcessFromProcFS(kiv_os::THandle pid);
