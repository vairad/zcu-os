#include "process.h"

#include <mutex>
#include <vector>
#include <map>
#include <Windows.h>

// THandle 1 - 65535 
//    - 0 - 1024 PID
//    - 1024 - 2048 TID
const kiv_os::THandle MAX_PROCESS_COUNT = 1024;
const kiv_os::THandle MAX_THREAD_COUNT = 1024;
const kiv_os::THandle BASE_TID_INDEX = MAX_PROCESS_COUNT;

/// Lock for critical section of PCB Table
std::mutex process_table_lock;

/// Lock for critical section of TCB Table
std::mutex thread_table_lock;

/// Lock for critical section of tid to pid map
std::mutex tid_map_lock;

/// Map of TID (simulator machine) to PID (simulated in OS)
std::map<std::thread::id, kiv_os::THandle> thread_to_tid;

/// Table of running processes - declare global variable
std::shared_ptr<PCB> process_table[MAX_PROCESS_COUNT];

/// Table of running threads - declare global variable
std::shared_ptr<TCB> thread_table[MAX_THREAD_COUNT];

/// Root directory of first process
const std::string root_directory = "C:/";


/// user.dll ref initialised by initialise_kernel()
extern HMODULE User_Programs;

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Interface funnction
/// Function handle and call routines connected with creating process
/// <param name='context'>Processor context prepared for system call</param>
/// <return>Success flag</return>
bool HandleProcess(kiv_os::TRegisters &context)
{
		switch (context.rax.l)
		{
			case kiv_os::scClone:
				return routineCloneProcess(context);
		
			case kiv_os::scWait_For:
				return routineWaitForProcess(context);
		
			default:
				Set_Err_Process(kiv_os::erInvalid_Argument, context);
				break;
		}
		return false;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine choose between createProcess and createThread
/// depends on rcx value is equal with clCreateProcess or clCreateThread
///
/// <see>kiv_os::clCreateProcess</see>
/// <see>kiv_os::clCreateThread</see>
/// <param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool routineCloneProcess(kiv_os::TRegisters & context)
{
	switch (context.rcx.l)
	{
		case kiv_os::clCreate_Process:
			return subroutineCreateProcess(context);

		case kiv_os::clCreate_Thread:  
			return subroutineCreateThread(context);
		
		default:
			Set_Err_Process(kiv_os::erInvalid_Argument, context);
			break;
	}
	return false;
}


/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine wait wait for another process
///IN: rdx is pointer to array of THandle, which we are waitong for
///    rcx is handle count
///    returns when first handle is signalised
///OUT:rax is signalized handle
///
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool routineWaitForProcess(kiv_os::TRegisters & context)
{
	kiv_os::THandle* handles = (kiv_os::THandle*)context.rdx.r;
	const size_t handlesCount = context.rcx.x;

	for (size_t iter = 0; iter < handlesCount; iter++)
	{
		kiv_os::THandle handle = waitForSingleHandle(handles[iter]);
	}
	return false;
}


/// /////////////////////////////////////////////////////////////////////////////////////////
/// rdx contains pointer to null-terminated string with name of running command
/// rdi is pointer to kiv_os::TProcess_Startup_Info
/// <see cref="kiv_os::TProcess_Startup_Info">kiv_os::TProcess_Startup_Info</see>
/// <param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool subroutineCreateProcess(kiv_os::TRegisters & context)
{	
	std::lock_guard<std::mutex> lck(process_table_lock);

	//find next free PID value
	kiv_os::THandle pid = getNextFreePid();

	//there is no free PID
	if (MAX_PROCESS_COUNT == pid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context); 
		return false;
	}

	//create new process record
	std::shared_ptr<PCB> pcb = createFreePCB(pid);

	//initialise values in pcb
	initialisePCB(pcb, (char *)context.rdx.r, (kiv_os::TProcess_Startup_Info *) context.rdi.r);

	// load program entry point
	kiv_os::TEntry_Point program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, pcb->program_name.c_str());
	if (!program)
	{
		process_table[pcb->pid] = nullptr;
		Set_Err_Process(kiv_os::erFile_Not_Found, context);
		return false;
	}
	
	//prepare and run thread
	kiv_os::THandle tid = createProcessThread(pid, program, context);
	if ( -1 == tid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context);
		return false;
	}

	context.rax.x = pid;
	return true;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// rdx is TThreadProc -> thread entry point
/// rdi is *data -> data for thread routine
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool subroutineCreateThread(kiv_os::TRegisters & context)
{
	std::lock_guard<std::mutex> lck(process_table_lock);

	kiv_os::THandle pid = getPid();

	kiv_os::TThread_Proc program = (kiv_os::TThread_Proc) context.rdx.r;
	void *data = (void *)context.rdi.r;

	kiv_os::THandle tid = createThread(pid, program, data);
	if (-1 == tid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context);
		return false;
	}

	context.rax.x = tid;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


kiv_os::THandle waitForSingleHandle(kiv_os::THandle handle)
{
	if (handle < BASE_TID_INDEX) {
		return waitForProcess(handle);
	}
	else {
		return waitForThread(handle);
	}
}


kiv_os::THandle waitForProcess(const kiv_os::THandle handle)
{
	std::lock_guard<std::mutex> lck(process_table_lock);
	if(!validateHandle(handle))
	{
		return -1;
	}
	process_table[handle]->waiting.wait();
	//TODO RVA clean PCB table
	return handle;
}

/// //////////////////////////////////////////////////////////////////////////////////////////
/// Function joins thread depends of 
///
kiv_os::THandle waitForThread(const kiv_os::THandle handle)
{
	if(!validateHandle(handle))
	{
		return -1;
	}
	thread_table[handle - BASE_TID_INDEX]->thread.join();

	//TODO RVA clean TCB table
	return handle;
}

/// //////////////////////////////////////////////////////////////////////////////////////////
/// Function validate
///
bool validateHandle(const kiv_os::THandle handle)
{
	if (handle < BASE_TID_INDEX) {
		return process_table[handle] != nullptr;
	}
	else {
		return thread_table[handle - BASE_TID_INDEX] != nullptr;
	}
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine create and run main thread for process
///<param name='context'>PID running process</param>
///<param name='entry_point'>Entry point of process kiv_os::TEntry_Point</param>
///<param name='data'>Registers of processor for </param>
/// <return>TID of created thread</return>
kiv_os::THandle createProcessThread(kiv_os::THandle pid, kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context)
{
	std::lock_guard<std::mutex> lck(thread_table_lock);

	//find next free TID value
	kiv_os::THandle tid = getNextFreeTid();
	if ((MAX_THREAD_COUNT + BASE_TID_INDEX) == tid)
	{
		return -1;
	}
	process::TStartBlock procInfo(entry_point, context);
	std::shared_ptr<TCB> tcb = createFreeTCB(tid, pid);

	tcb->thread = std::thread(&cr0, procInfo);
	std::lock_guard<std::mutex> lck2(tid_map_lock);
	thread_to_tid[tcb->thread.get_id()] = tid;

	return tid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine create and run new thread for process
///<param name='context'>PID running process</param>
///<param name='entry_point'>Entry point of thread kiv_os::TThread_Proc</param>
///<param name='data'>Pointer to data for thread entry_point</param>
/// <return>TID of created thread</return>
kiv_os::THandle createThread(kiv_os::THandle pid, kiv_os::TThread_Proc entry_point, void * data)
{
	std::lock_guard<std::mutex> lck(thread_table_lock);

	//find next free TID value
	kiv_os::THandle tid = getNextFreeTid();
	if ((MAX_THREAD_COUNT + BASE_TID_INDEX) == tid)
	{
		return -1;
	}

	std::shared_ptr<TCB> tcb = createFreeTCB(tid, pid);

	tcb->thread = std::thread(entry_point, data);
	std::lock_guard<std::mutex> lck2(tid_map_lock);
	thread_to_tid[tcb->thread.get_id()] = tid;

	return tid;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// /////////////////////////////////////////////////////////////////////////////////////////
/// <return>Unused pid in PCB table</return>
kiv_os::THandle getNextFreePid()
{
	kiv_os::THandle pid = 0;
	while (process_table[pid] != nullptr && pid < MAX_PROCESS_COUNT) {
		pid++;
	}
	return pid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// <return>Unused tid in TCB table</return>
kiv_os::THandle getNextFreeTid()
{
	kiv_os::THandle tid = 0;
	while (thread_table[tid] != nullptr && tid < MAX_THREAD_COUNT) {
		tid++;
	}
	return BASE_TID_INDEX + tid;
}

/// TODO RVA thread safety?
/// !!! ACCESS PCB TABLE !!!!
/// !!! THREAD UNSAFE !!!!!
std::shared_ptr<PCB> createFreePCB(kiv_os::THandle pid)
{
	std::shared_ptr<PCB> empty_pcb(new PCB());
	empty_pcb->pid = pid;
	if (pid == 0)
	{
		empty_pcb->parent_pid = pid; // process don't have parent
	}
	else
	{
		empty_pcb->parent_pid = getPid();
	}
	process_table[pid] = empty_pcb;
	
	return empty_pcb;
}

/// TODO RVA thread safety?
/// !!! ACCESS TCB TABLE !!!!
/// !!! THREAD UNSAFE !!!!!
std::shared_ptr<TCB> createFreeTCB(kiv_os::THandle tid, kiv_os::THandle pid)
{
	std::shared_ptr<TCB> tcb(new TCB());
	tcb->pid = pid;
	tcb->tid = tid;

	thread_table[pid] = tcb;

	return tcb;
}


void initialisePCB(std::shared_ptr<PCB> pcb, char * program_name, kiv_os::TProcess_Startup_Info * startup_info)
{
	//fill program name
	pcb->program_name = std::string(program_name);

	//fill working directory
	if (pcb->pid == 0)
	{
		pcb->working_directory = root_directory;
	}
	else
	{
		pcb->working_directory = process_table[getPid()]->working_directory;
	}

	//fill io descriptors
	pcb->io_devices = std::vector<kiv_os::THandle>();
	if(startup_info->OSstdin == kiv_os::erInvalid_Handle)
	{
		pcb->io_devices.push_back(kiv_os::stdInput);
	}
	else
	{
		pcb->io_devices.push_back(startup_info->OSstdin);
	}

	if (startup_info->OSstdin == kiv_os::erInvalid_Handle)
	{
		pcb->io_devices.push_back(kiv_os::stdOutput);
	}
	else
	{
		pcb->io_devices.push_back(startup_info->OSstdout);
	}

	if (startup_info->OSstdin == kiv_os::erInvalid_Handle)
	{
		pcb->io_devices.push_back(kiv_os::stdError);
	}
	else
	{
		pcb->io_devices.push_back(startup_info->OSstderr);
	}

}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up TID in map. Using current thread ID instead of using HW scheduler values.
/// <return>TID of running process -1 if error occured</return>
kiv_os::THandle getTid()
{
	//TODO RVA check retval
	kiv_os::THandle tid = -1;
	std::thread::id this_id = std::this_thread::get_id();

	std::lock_guard<std::mutex> lck(tid_map_lock);
	tid = thread_to_tid[this_id];

	return tid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up PID in map. Using current thread ID instead of using HW scheduler values.
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getPid()
{
	//todo RVA check retval
	kiv_os::THandle tid = -1 , pid = -1;
	std::thread::id this_id = std::this_thread::get_id();

	tid = getTid();
	
	std::lock_guard<std::mutex> lck1(thread_table_lock);

	pid = thread_table[tid - BASE_TID_INDEX]->pid;

	return pid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up Parent PID in proces table.
/// <see>getPid()</see>
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getParentPid()
{
	//todo RVA check retval
	std::lock_guard<std::mutex> lck(process_table_lock);
	kiv_os::THandle pid = getPid();
	kiv_os::THandle ppid = -1;
	ppid = process_table[pid]->parent_pid;
	
	return ppid; 
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up record based on PID and return working dir of running proces
/// <return>working directory of process</return>
std::string getWorkingDir() 
{
	//todo RVA check retval
	std::lock_guard<std::mutex> lck(process_table_lock);	
	kiv_os::THandle pid = getPid();
	std::string wd = process_table[pid]->working_directory;

	return wd;
}


/// /////////////////////////////////////////////////////////////////////////////////////////
///
void Set_Err_Process(const uint16_t ErrorCode, kiv_os::TRegisters & regs)
{
	regs.flags.carry = true;
	regs.rax.x = ErrorCode;
}


void cr0(process::TStartBlock &procInfo)
{
	// do entry point work
	procInfo.entry_point(procInfo.context);

	//I'm done, notify others
	std::lock_guard<std::mutex> lck(process_table_lock);
	const auto pid = getPid();
	process_table[pid]->waiting.notifyAll();
}