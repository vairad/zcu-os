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
				routineCloneProcess(context);
			case kiv_os::scWait_For:
				routineWaitForProcess(context);
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
	process_table_lock.lock();

	//find next free PID value
	kiv_os::THandle pid = 0;
	while (process_table[pid] != nullptr && pid < MAX_PROCESS_COUNT) {
		pid++;
	}

	//there is no free PID
	if (MAX_PROCESS_COUNT == pid)
	{
		process_table_lock.unlock();
		Set_Err_Process(kiv_os::erInvalid_Handle, context); //TODO novy chybovy stav? ...
		
		return false;
	}

	//create new process record
	std::shared_ptr<PCB> new_pcb(new PCB());
	new_pcb->pid = pid;
	if (pid == 0)
	{
		new_pcb->parent_pid = pid; // process don't have parent
	}
	else
	{
		new_pcb->parent_pid = getPid();
	}
	process_table[pid] = new_pcb;
	
	//fill program name
	char * program_name = (char *)context.rdx.r;
	new_pcb->program_name = std::string(program_name);

	//fill working directory
	if (pid == 0)
	{
		new_pcb->working_directory = root_directory;
	} 
	else
	{
		new_pcb->working_directory = process_table[getPid()]->working_directory;
	}

	

	//fill io descriptors
	kiv_os::TProcess_Startup_Info *procInfo = (kiv_os::TProcess_Startup_Info *) context.rdi.r;
	new_pcb->io_devices = std::vector<kiv_os::THandle>();
	new_pcb->io_devices.push_back(procInfo->OSstdin);
	new_pcb->io_devices.push_back(procInfo->OSstdout);
	new_pcb->io_devices.push_back(procInfo->OSstderr);

	//prepare and run thread
	kiv_os::TEntry_Point program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, program_name);
	if (!program)
	{
		//TODO clear PCB table
		Set_Err_Process(kiv_os::erFile_Not_Found, context);
		return false;
	}

	// TODO run thread
	new_pcb->thread = std::thread(program, context);

	tid_map_lock.lock();
	tid_to_pid[new_pcb->thread.get_id()] = pid;
	tid_map_lock.unlock();


	process_table_lock.unlock();
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

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up PID in map. Using current thread ID instead of using HW scheduler values.
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getPid()
{
	kiv_os::THandle pid = -1;
	std::thread::id this_id = std::this_thread::get_id();

	tid_map_lock.lock();
	pid = tid_to_pid[this_id];
	tid_map_lock.unlock();

	return pid;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up Parent PID in proces table.
/// <see>getPid()</see>
/// <return>PID of running process -1 if error occured</return>
kiv_os::THandle getParentPid()
{
	kiv_os::THandle pid = getPid();
	kiv_os::THandle ppid = -1;
	process_table_lock.lock();
	ppid = process_table[pid]->parent_pid;
	process_table_lock.unlock();
	
	return ppid; 
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Function look up record based on PID and return working dir of running proces
/// <return>working directory of process</return>
std::string getWorkingDir() 
{
	kiv_os::THandle pid = getPid();
	process_table_lock.lock();
	std::string wd = process_table[pid]->working_directory;
	process_table_lock.unlock();

	return wd;
}
 


void Set_Err_Process(uint16_t ErrorCode, kiv_os::TRegisters & regs)
{
	regs.flags.carry = true;
	regs.rax.x = ErrorCode;
}
