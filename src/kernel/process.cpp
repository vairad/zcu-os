#include "process.h"

#include <mutex>
#include <vector>
#include <Windows.h>

const kiv_os::THandle MAX_PROCESS_COUNT = 1024;

/// Lock for critical section of PCB Table
std::mutex process_table_lock;

/// Lock for critical section of tid to pid map
std::mutex tid_map_lock;

/// Map of TID (simulator machine) to PID (simulated in OS)
std::map<std::thread::id, kiv_os::THandle> tid_to_pid;

/// Table of running processes - declare global variable
std::shared_ptr<PCB> process_table[MAX_PROCESS_COUNT];

/// Root directory of first process
const std::string root_directory = "C:/";

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
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool routineWaitForProcess(kiv_os::TRegisters & context)
{
	//TODO not implemented yet
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
		Set_Err_Process(kiv_os::erInvalid_Handle, context); //TODO novy chybovy stav? ...
		
		return false;
	}

	//create new process record
	std::shared_ptr<PCB> new_pcb = createFreePCB(pid);

	//initialise values in pcb
	initialisePCB(new_pcb, (char *)context.rdx.r, (kiv_os::TProcess_Startup_Info *) context.rdi.r);

	//prepare and run thread
	kiv_os::TEntry_Point program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, new_pcb->program_name.c_str());
	if (!program)
	{
		//TODO clear PCB table
		Set_Err_Process(kiv_os::erFile_Not_Found, context);
		return false;
	}

	new_pcb->thread = std::thread(program, context);

	std::lock_guard<std::mutex> lck2(tid_map_lock);
	tid_to_pid[new_pcb->thread.get_id()] = pid;

	return true;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine choose between createProcess and createThread
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool subroutineCreateThread(kiv_os::TRegisters & context)
{
	//TODO not implemented yet
	return false;
}

kiv_os::THandle getNextFreePid()
{
	kiv_os::THandle pid = 0;
	while (process_table[pid] != nullptr && pid < MAX_PROCESS_COUNT) {
		pid++;
	}
	return pid;
}

/// TODO thread safety?? :D
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
	pcb->io_devices.push_back(startup_info->OSstdin);
	pcb->io_devices.push_back(startup_info->OSstdout);
	pcb->io_devices.push_back(startup_info->OSstderr);
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up PID in map. Using current thread ID instead of using HW scheduler values.
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getPid()
{
	kiv_os::THandle pid = -1;
	std::thread::id this_id = std::this_thread::get_id();

	std::lock_guard<std::mutex> lck2(tid_map_lock);
	pid = tid_to_pid[this_id];

	return pid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up Parent PID in proces table.
/// <see>getPid()</see>
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getParentPid()
{
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
	std::lock_guard<std::mutex> lck(process_table_lock);
	kiv_os::THandle pid = getPid();
	std::string wd = process_table[pid]->working_directory;

	return wd;
}
 


void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & regs)
{
	regs.flags.carry = true;
	regs.rax.x = ErrorCode;
}
