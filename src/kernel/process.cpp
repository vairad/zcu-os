#include "process.h"

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
				//todo throw not implemented err
				break;
		}
		return false;
}

/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine create new process
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool routineCloneProcess(kiv_os::TRegisters & context)
{
	//TODO not implemented yet
	return false;
}


/// /////////////////////////////////////////////////////////////////////////////////////////
/// Routine create new thread
///<param name='context'>Reference to processor registers</param>
/// <return>Success flag</return>
bool routineWaitForProcess(kiv_os::TRegisters & context)
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
