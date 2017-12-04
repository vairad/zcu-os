#include "process_api.h"
#include "process.h"

#include <mutex>
#include <vector>
#include <map>

#include <Windows.h>

#include "../filesystem/VFS.h"

#undef stdin
#undef stderr
#undef stdout

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


/// next PID value
kiv_os::THandle next_avaliable_pid = 0;

/// next TID value
kiv_os::THandle next_avaliable_tid = 0;


/// user.dll ref initialised by initialise_kernel()
extern HMODULE User_Programs;

/**
 * \brief Interface funnction
 * Function handle and call routines connected with creating process
 * \param context Processor context prepared for system call
 * \return Success flag
 */
bool HandleProcess(kiv_os::TRegisters &context)
{
		switch (context.rax.l)
		{
			case kiv_os::scClone:
				return routineCloneProcess(context);
		
			case kiv_os::scWait_For:
				return routineWaitForProcess(context);

			case kiv_os::scShutdown:
				return routineShutdown(context);
		
			default:
				Set_Err_Process(kiv_os::erInvalid_Argument, context);
				break;
		}
		return false;
}


/**
 * \brief Routine choose between createProcess and createThread
 * depends on rcx value is equal with clCreateProcess or clCreateThread
 * \see kiv_os::clCreateProcess
 * \see kiv_os::clCreateThread
 * \param context Reference to processor registers
 * \return Success flag
 */
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



/**
 * \brief  Routine wait wait for another process
 * IN: rdx is pointer to array of THandle, which we are waitong for
 *   rcx is handle count
 *   returns when first handle is signalised
 * OUT:rax is signalized handle
 * \param context Reference to processor registers
 * \return Success flag
 */
bool routineWaitForProcess(kiv_os::TRegisters & context)
{
	const auto handles = reinterpret_cast<kiv_os::THandle*>(context.rdx.r);
	const size_t handlesCount = context.rcx.x;
	kiv_os::THandle tid;
	std::vector<kiv_os::THandle> already_done;
	
	{ //critical section block
		std::unique_lock<std::mutex> proc_lock(process_table_lock);
		std::unique_lock<std::mutex> thread_lock(thread_table_lock);

		for (size_t iter = 0; iter < handlesCount; iter++)
		{
			addToWaitingQueue(handles[iter], already_done);
		}
	    tid = process::getTid();
	}

	if (tid == kiv_os::erInvalid_Handle)
	{
		return false;
	}
	kiv_os::THandle handle;
	size_t retval;
	if(already_done.empty())
	{
		thread_table[TidToTableIndex(tid)]->state.sleep();
		handle = thread_table[TidToTableIndex(tid)]->state.get_wake_by();
		retval = GetRetVal(handle);
	}
	else
	{
		handle = already_done.back();
		retval = GetRetVal(handle);
	}
	
	context.rax.x = handle;
	context.rcx.r = retval;
	return true;
}


bool routineShutdown(const kiv_os::TRegisters& context);

/**
 * \brief 
 *        rdx contains pointer to null-terminated string with name of running command
 *        rdi is pointer to kiv_os::TProcess_Startup_Info
 * \see kiv_os::TProcess_Startup_Info
 * \param context Reference to processor registers
 * \return Success flag
 */
bool subroutineCreateProcess(kiv_os::TRegisters & context)
{	
	//find next free PID value
	const auto pid = getNextFreePid();

	//there is no free PID
	if (MAX_PROCESS_COUNT == pid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context); 
		return false;
	}

	//create new process record
	const auto pcb = createFreePCB(pid);

	kiv_os::TProcess_Startup_Info * startupInfo = (kiv_os::TProcess_Startup_Info *) context.rdi.r;
	//initialise values in pcb
	initialisePCB(pcb, (char *)context.rdx.r, startupInfo);

	// load program entry point
	const auto program = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, pcb->program_name.c_str());
	if (!program)
	{
		process_table[pcb->pid] = nullptr;
		Set_Err_Process(kiv_os::erFile_Not_Found, context);
		return false;
	}
	
	//add procfs record
	addProcessToProcFS(pcb->program_name, pid);

	//prepare and run thread
	const auto tid = createProcessThread(pid, program, context);
	if ( -1 == tid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context);
		return false;
	}

	context.rax.x = pid;
	return true;
}


/**
 * \brief 
 *        rdx is TThreadProc -> thread entry point
 *        rdi is *data -> data for thread routine
 * \param context Reference to processor registers
 * \return Success flag
 */
bool subroutineCreateThread(kiv_os::TRegisters & context)
{
	kiv_os::THandle pid = process::getPid();

	kiv_os::TThread_Proc program = (kiv_os::TThread_Proc) context.rdx.r;
	void *data = (void *)context.rdi.r;

	const auto tid = createThread(pid, program, data);
	if (-1 == tid)
	{
		Set_Err_Process(kiv_os::erProces_Not_Created, context);
		return false;
	}

	context.rax.x = tid;
	return true;
}


/**
 * \brief Add handle to waiting queue handle process
 * parrent should have lock for processes and threads
 * \param handle handle for we are waiting
 * \return false when some error occurs 
 */
bool addToWaitingQueue(const kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done)
{
	if (handle < BASE_TID_INDEX) {
		return waitForProcess(handle, already_done);
	}
	else {
		return waitForThread(handle, already_done);
	}
}

/**
 * \brief Add this handle to process waiting queue
 * parrent should have lock for processes
 * \param handle pid of process which we are waiting for
 * \return success flag
 */
bool waitForProcess(const kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done)
{
	if(!validateHandle(handle))
	{
		return false;
	}
	const auto tid = process::getTid();
	const auto to_block = process_table[handle]->waiting.wait(tid);
	if (!to_block) { already_done.push_back(handle); }
	return true;
}


/**
 * \brief  Add this handle to thread waiting queue
 * parrent should have lock for threads
 * \param handle tid of thread which we are waiting for
 * \return success flag
 */
bool waitForThread(const kiv_os::THandle handle, std::vector<kiv_os::THandle> & already_done)
{
	if(!validateHandle(handle))
	{
		return false;
	}
	const auto tid = process::getTid();
	const auto to_block = thread_table[TidToTableIndex(handle)]->waiting.wait(tid);
	if (!to_block) { already_done.push_back(handle); }
	return true;
}

/**
 * \brief Validate if handle ID is valid process handle (PID or TID)
 * \param handle 
 * \return success flag
 */
bool validateHandle(const kiv_os::THandle handle)
{
	if (handle < BASE_TID_INDEX) {
		return process_table[handle] != nullptr;
	}
	else {
		return thread_table[TidToTableIndex(handle)] != nullptr;
	}
}


/** 
 * \brief Routine create and run main thread for process
 * \param pid PID running process
 * \param entry_point Entry point of process kiv_os::TEntry_Point
 * \param context Registers of processor for 
 * \return TID of created thread or kiv_os::erInvalidHandle when error occurs
 */
kiv_os::THandle createProcessThread(const kiv_os::THandle pid, const kiv_os::TEntry_Point entry_point, kiv_os::TRegisters context)
{
	std::unique_lock<std::mutex> lck(thread_table_lock);

	//find next free TID value
	const auto tid = getNextFreeTid();
	if ((MAX_THREAD_COUNT + BASE_TID_INDEX) == tid)
	{
		return kiv_os::erInvalid_Handle;
	}
	auto tcb = createFreeTCB(tid, pid);
	process::TStartProcessBlock procInfo(tid ,entry_point, context);

	tcb->thread = std::thread(&process0, procInfo);
	return tid;
}

/**
 * \brief Routine create and run new thread for process
 * \param pid PID running process
 * \param entry_point Entry point of thread kiv_os::TThread_Proc
 * \param data Pointer to data for thread entry_point
 * \return TID of created thread or kiv_os::erInvalidHandle when error occurs
 */
kiv_os::THandle createThread(const kiv_os::THandle pid, const kiv_os::TThread_Proc entry_point, void * data)
{
	std::shared_ptr<TCB> tcb;
	kiv_os::THandle tid;
	{
		std::unique_lock<std::mutex> lck(thread_table_lock);

		//find next free TID value
		tid = getNextFreeTid();
		if ((MAX_THREAD_COUNT + BASE_TID_INDEX) == tid)
		{
			return kiv_os::erInvalid_Handle;
		}

		tcb = createFreeTCB(tid, pid);
	}

	if(tcb == nullptr)
	{
		return kiv_os::erInvalid_Handle;
	}
	process::TStartThreadBlock threadInfo(tid, entry_point, data);

	tcb->thread = std::thread(&thread0, threadInfo);
	return tid;
}

/**
 * \brief Method finds first empty slot in process table WARNING USE WITH LOCK FOR THREAD TABLE
 * \return Unused pid in PCB table
 */
kiv_os::THandle getNextFreePid()
{
	kiv_os::THandle pid = next_avaliable_pid;
	size_t counter = 0;

	while (process_table[pid] != nullptr && pid < MAX_PROCESS_COUNT) {
		pid = ++pid % MAX_PROCESS_COUNT;

		if(counter > MAX_PROCESS_COUNT) // in case PCB table is full
		{
			pid = kiv_os::erInvalid_Handle;
			next_avaliable_pid = 0;
			return pid;
		}
		counter++;
	}
	next_avaliable_pid = (pid + 1) % MAX_PROCESS_COUNT;
	return pid;
}

/**
 * \brief Method finds first empty slot in thread table WARNING USE WITH LOCK FOR THREAD TABLE
 * \return Unused tid in TCB table
 */
kiv_os::THandle getNextFreeTid()
{
	kiv_os::THandle tid = next_avaliable_tid;
	size_t counter = 0;

	while (thread_table[tid] != nullptr && tid < MAX_THREAD_COUNT) {
		tid = ++tid % MAX_THREAD_COUNT;

		if (counter > MAX_THREAD_COUNT) // in case PCB table is full
		{
			tid = kiv_os::erInvalid_Handle;
			next_avaliable_tid = 0;
			return tid;
		}
		counter++;
	}

	next_avaliable_tid = (tid + 1) % MAX_THREAD_COUNT;
	return BASE_TID_INDEX + tid;
}

/**
 * \brief Resolve TID to index to thread table
 * \return index to thread table
 */
inline kiv_os::THandle TidToTableIndex(const kiv_os::THandle tid)
{
	return tid - BASE_TID_INDEX;
}

/**
 * \brief Create free PCB structure WARNING USE WITH LOCKED LOCKS.
 * 
 * Method setup PID and set std streams as kiv_os::erInvalidHandle
 * Assign this structure to process table at pid index
 * 
 * \param pid process id of creating process
 * \return smart poiner of new structutre
 */
std::shared_ptr<PCB> createFreePCB(const kiv_os::THandle pid)
{
	std::shared_ptr<PCB> empty_pcb(new PCB());
	empty_pcb->pid = pid;
	if (pid == 0)
	{
		empty_pcb->parent_pid = pid; // process don't have parent
	}
	else
	{
		empty_pcb->parent_pid = process::getPid();
	}

	empty_pcb->io_devices.resize(4); // Hard coded size for max index of std handler 3
	std::fill(empty_pcb->io_devices.begin(), empty_pcb->io_devices.end(), kiv_os::erInvalid_Handle); // make all handles invalid

	process_table[pid] = empty_pcb;
	
	return empty_pcb;
}

/**
 * \brief Create free TCB structure WARNING USE WITH LOCKED LOCKS
 * \param tid thread id of new thread
 * \param pid process id of creating process
 * \return smart pointer of new structure
 */
std::shared_ptr<TCB> createFreeTCB(const kiv_os::THandle tid, const kiv_os::THandle pid)
{
	std::shared_ptr<TCB> tcb(new TCB());
	tcb->pid = pid;
	tcb->tid = tid;

	thread_table[TidToTableIndex(tid)] = tcb;

	return tcb;
}

/**
 * \brief Method initialise PCB structure by gived values
 * \param pcb smart pointer to created PCB
 * \param program_name name of program
 * \param startup_info info about std streams
 */
void initialisePCB(std::shared_ptr<PCB> pcb, char * program_name, kiv_os::TProcess_Startup_Info * startup_info)
{
//fill program name
	pcb->program_name = std::string(program_name);

//fill working directory
	pcb->working_directory = process_table[process::getPid()]->working_directory;

//fill io descriptors
	pcb->io_devices[kiv_os::stdInput] = resolveAndDuplicateFD(startup_info->stdin);
	pcb->io_devices[kiv_os::stdOutput] = resolveAndDuplicateFD(startup_info->stdout);
	pcb->io_devices[kiv_os::stdError] = resolveAndDuplicateFD(startup_info->stderr);
}

/**
 * \brief Method add record to map where value is simulated thread id and key is std::this_thread.get_id()
 * \param tid thread id to pair with this thread
 */
void addRecordToThreadMap(const kiv_os::THandle tid)
{
	std::unique_lock<std::mutex> lck(tid_map_lock);
	thread_to_tid[std::this_thread::get_id()] = tid;
}



/**
 * \brief Method check value of parent proc fd opens it and assign to child process
 * \param parent_proc_fd id of parent process file descriptor
 * \return value of system fd
 */
kiv_os::THandle resolveAndDuplicateFD(const kiv_os::THandle parent_proc_fd)
{
	kiv_os::THandle handle;
	if (parent_proc_fd == kiv_os::erInvalid_Handle)
	{
		handle = process::getSystemFD(kiv_os::stdOutput);
	}
	else
	{
		handle = process::getSystemFD(parent_proc_fd);
	}
	kiv_os_vfs::increaseFDescOpenCounter(handle);
	return handle;
}


/**
* \brief Set up error processor state
* \param ErrorCode \see api.h and api_extensions.h
* \param regs processor state
*/
void Set_Err_Process(const uint16_t ErrorCode, kiv_os::TRegisters & regs)
{
	regs.flags.carry = true;
	regs.rax.x = ErrorCode;
}

size_t GetRetVal(const kiv_os::THandle handle)
{
	if(handle < BASE_TID_INDEX)
	{
		size_t retval;
		const auto result = process_table[handle]->retval.red_ret_val(retval);
		if(result)
		{
			cleanProcess(handle);
		}
		return retval;
	}
	else
	{
		size_t retval;
		const auto thread_index = TidToTableIndex(handle);
		const auto result = thread_table[thread_index]->retval.red_ret_val(retval);
		if (result)
		{
			cleanThread(thread_index);
		}
		return retval;
	}
}

/**
 * \brief Method kill and release all processes except init
 * \param context in 
 * \return 
 */
bool routineShutdown(const kiv_os::TRegisters& context)
{
	bool successFlag = true;
	for(int pid = 1; pid < MAX_PROCESS_COUNT; pid++)
	{
		successFlag &= stopProcess(pid);
	}
	return successFlag;
}

/**
 * \brief Method recognize if Handle is thread or proces and pass handle to correct routine to process 
 * \param handle handle tid/pid to stop
 * \return success flag
 */
bool stopHandle(const kiv_os::THandle handle)
{
	if(!validateHandle(handle))
	{
		return false;
	}
	if(handle < BASE_TID_INDEX)
	{
		return stopProcess(handle);
	}
	else
	{
		return stopThread(handle);
	}
}


/**
* \brief Method stop thread, clean its structures and release tid
* \param tid handle tid to stop
* \return success flag
*/
bool stopThread(const kiv_os::THandle tid)
{
	return false; //TODO RVA implement kill thread algorithm
}


/**
* \brief Method stop all process threads and clean its structures and release tid
* \param pid handle pid to stop
* \return success flag
*/
bool stopProcess(const kiv_os::THandle pid)
{
	const auto my_pid = process::getPid();
	if(process_table[pid] != nullptr && pid != my_pid)
	{
		process_table[pid]->retval.make_done(99, 0);
		cleanProcess(pid);
	}
	return true; 
}

/**
* \brief Function run thread and after thread ens, it will notify all waiting processes
* \param procInfo process startup informarion
*/
void process0(process::TStartProcessBlock &procInfo)
{
	addRecordToThreadMap(procInfo.tid);
	kiv_os::TRegisters context = procInfo.proc;
	context.rdi.r = reinterpret_cast<uint64_t>(procInfo.getProcInfo());


	// do entry point work
	const size_t retval = procInfo.entry_point(context);

	{
		std::lock_guard<std::mutex> lock(process_table_lock);
		//I'm done, notify others
		const auto pid = process::getPid();


		const size_t size = process_table[pid]->waiting.size();
		process_table[pid]->retval.make_done(retval, size);

		process_table[pid]->waiting.close();
		process_table[pid]->waiting.notifyAll();
	}
}

/**
* \brief Function run thread and after thread ens, it will notify all waiting processes
* \param threadInfo thread startup information
*/
void thread0(process::TStartThreadBlock& threadInfo)
{
	addRecordToThreadMap(threadInfo.tid);
	// do entry point work
	const size_t retval = threadInfo.entry_point(threadInfo.context);

	//I'm done, notify others
	{
		std::unique_lock<std::mutex> lck(thread_table_lock);

		const auto threadIndex = TidToTableIndex(process::getTid());

		const size_t size = thread_table[threadIndex]->waiting.size();
		thread_table[threadIndex]->retval.make_done(retval, size);

		thread_table[threadIndex]->waiting.close();
		thread_table[threadIndex]->waiting.notifyAll();
	}
}

void cleanProcess(const kiv_os::THandle handle)
{
	std::lock_guard<std::mutex> lock(process_table_lock);

	//rm procfs record
	remProcessFromProcFS(handle);

	//check stop and clean all threads
	for (size_t index = 0; index < MAX_THREAD_COUNT; index++)
	{
		if(thread_table[index]!= nullptr && thread_table[index]->pid == handle)
		{
			cleanThread(kiv_os::THandle(index));
		}
	}

	//close all open handles 
	for (auto iterator = process_table[handle]->io_devices.begin(); iterator != process_table[handle]->io_devices.end(); ++iterator) 
	{
		const auto fd_index = *iterator;
		kiv_os_vfs::close(fd_index);
	}

	//notify waiting
	process_table[handle]->waiting.notifyAll();

	//clean pcb table - release record memory and release pid
	std::shared_ptr<PCB> pcb = process_table[handle];
	process_table[handle] = nullptr;
}

void cleanThread(const kiv_os::THandle table_index)
{
	std::lock_guard<std::mutex> lock(thread_table_lock);
	
	//notify waiting
	thread_table[table_index]->waiting.notifyAll();
	
	//clean pcb table - release record memory and release pid
	std::shared_ptr<TCB> tcb = thread_table[table_index];
	void cleanProcess(const kiv_os::THandle handle);
	if(table_index == 0)
	{
		return; //init has no record in thread structure
	}
	tcb->thread.detach();
	thread_table[table_index] = nullptr;
}


/**
 * \brief Function look up TID in map. Using current thread ID instead of using HW scheduler values.
 * \return TID of running process kiv_os::erInvalidHandle if error occured
 */
kiv_os::THandle process::getTid()
{
	const auto this_id = std::this_thread::get_id();
	const auto tid = thread_to_tid[this_id];
	if(tid < BASE_TID_INDEX || tid > (BASE_TID_INDEX+MAX_THREAD_COUNT))
	{
		return kiv_os::erInvalid_Handle;
	}
	return tid;
}

/**
 * \brief Function look up PID in map.Using current thread ID instead of using HW scheduler values.
 * \see getTid()
 * \return PID of running process kiv_os::erInvalidHandle if error occured
 */
kiv_os::THandle process::getPid()
{
	const auto tid = getTid();
	
	if(tid == kiv_os::erInvalid_Handle || tid < BASE_TID_INDEX)
	{
		return kiv_os::erInvalid_Handle;
	}
	const auto thread_index = TidToTableIndex(tid);
	if(thread_table[thread_index] == nullptr)
	{
		return kiv_os::erInvalid_Handle;
	}
	const auto pid = thread_table[thread_index]->pid;
	return pid;
}


/**
 * \brief Function look up Parent PID in proces table.
 * \see getPid()
 * \return PID of running process kiv_os::erInvalidHandle if error occured
 */
kiv_os::THandle process::getParentPid()
{
	const auto pid = getPid();

	if(pid == kiv_os::erInvalid_Handle)
	{
		return kiv_os::erInvalid_Handle;
	}

	const auto ppid = process_table[pid]->parent_pid;
	
	return ppid; 
}

/**
  * \brief Function look up record based on PID and return working dir of running proces
  * \return  working directory of process
  */
std::string process::getWorkingDir()
{
	const auto pid = getPid();
	if(pid == kiv_os::erInvalid_Handle)
	{
		return "";
	}
	auto wd = process_table[pid]->working_directory;
	return wd;
}


/**
 * \brief Method changes working dir of running process
 * \param new_dir aprooved dir by VFS
 * \return success flag (error occurs when getPID() fails)
 */
bool process::changeWorkingDir(const std::string new_dir)
{
	std::lock_guard<std::mutex> lock(process_table_lock);
	const auto pid = getPid();
	if (pid == kiv_os::erInvalid_Handle)
	{
		return false;
	}
	
	process_table[pid]->working_directory = new_dir;
	return true;
}

/**
 * \brief Method initialise init process in process table and in thread table
 * \return success flag
 */
bool process::createInit()
{
	std::unique_lock<std::mutex> lck(thread_table_lock);
	std::unique_lock<std::mutex> lck1(process_table_lock);

	//find next free PID value
	const auto pid = getNextFreePid();

	//there is no free PID
	if (MAX_PROCESS_COUNT == pid)
	{
		return false;
	}

	//create new process record
	const auto pcb = createFreePCB(pid);

	//initialise values in pcb
	pcb->program_name = "init";
	pcb->working_directory = root_directory;
	//open default streams
	pcb->io_devices[kiv_os::stdInput] = kiv_os_vfs::openFile("stdio:/CONIN$", 1, 0);
	pcb->io_devices[kiv_os::stdOutput] = kiv_os_vfs::openFile("stdio:/CONOUT$", 1, 0);
	pcb->io_devices[kiv_os::stdError] = kiv_os_vfs::openFile("stdio:/CONOUT$", 1, 0);

	//find next free TID value
	const auto tid = getNextFreeTid();
	if ((MAX_THREAD_COUNT + BASE_TID_INDEX) == tid)
	{
		return false;
	}

	auto tcb = createFreeTCB(tid, pid);

	addRecordToThreadMap(tid);
		
	return true;
}

bool process::destructInit()
{
	const auto pid = getPid();
	cleanProcess(pid);
	return true;
}


/**
 * \brief wakes up everything that waits on selected handle state (cond var)
 * \param handle handle to wake up
 */
void process::wakeUpThreadHandle(const kiv_os::THandle handle)
{
	const auto tid = process::getTid();
	if (thread_table[TidToTableIndex(handle)] && tid < BASE_TID_INDEX + MAX_THREAD_COUNT)
	{
		thread_table[TidToTableIndex(handle)]->state.wake_up(tid);
	}
}

/**
* \brief wakes up everything that waits on selected handle state (cond var)
* \param handle handle to wake up
*/
void process::wakeUpProcessHandle(const kiv_os::THandle handle)
{
	std::lock_guard<std::mutex> lock(thread_table_lock);

	const auto pid = process::getPid();
	if (thread_table[TidToTableIndex(handle)] && pid < MAX_THREAD_COUNT)
	{
		thread_table[TidToTableIndex(handle)]->state.wake_up(pid);
	}
}


/**
 * \brief Method returns value of system FD on position defined by program handler
 * When error occurs returns invalid handler
 * \param program_handle index to program FD array
 * \return System FD or kiv_os::erInvalid_Handler when error occurs 
 */
kiv_os::THandle process::getSystemFD(const kiv_os::THandle program_handle)
{
	std::lock_guard<std::mutex> lock(process_table_lock);

	const auto pid = process::getPid();
	if (pid == kiv_os::erInvalid_Handle)
	{
		return kiv_os::erInvalid_Handle;
	}
	kiv_os::THandle sys_handle;
	try
	{
		sys_handle = process_table[pid]->io_devices.at(program_handle);
	}
	catch (std::out_of_range)
	{
		sys_handle = kiv_os::erInvalid_Handle;
	}
	return sys_handle;
}

/**
* \brief Method add value of system FD and return value of program handler
* When error occurs returns invalid handler
* \param system_handle index to system FD array
* \return Program FD or kiv_os::erInvalid_Handler when error occurs
*/
kiv_os::THandle process::setNewFD(const kiv_os::THandle system_handle)
{
	std::lock_guard<std::mutex> lock(process_table_lock);

	const auto pid = process::getPid();
	kiv_os::THandle pr_handle;
	try
	{
		process_table[pid]->io_devices.push_back(system_handle);
		const auto new_index = ( process_table[pid]->io_devices.size() - 1 ) ; //decrement size by  ... vec is indexed from zero
		if(new_index >= kiv_os::erInvalid_Handle)
		{
			pr_handle = kiv_os::erInvalid_Handle;
		}else
		{
			pr_handle = kiv_os::THandle(new_index);
		}
	 

	}
	catch (std::exception) // undefined problems with call push_back
	{
		pr_handle = kiv_os::erInvalid_Handle;
	}
	return pr_handle;
}


/**
 * \brief Method disable program FD at specified index
 * \param program_handle index of program FD
 * \return success flag
 */
void process::removeProcessFD(const kiv_os::THandle program_handle)
{
	std::lock_guard<std::mutex> lock(process_table_lock);

	const auto pid = process::getPid();
	process_table[pid]->io_devices.at(program_handle) = kiv_os::erInvalid_Handle;
}

bool addProcessToProcFS(std::string name, kiv_os::THandle pid)
{
	std::string path = "procfs:/" + std::to_string(pid);

	auto handle = kiv_os_vfs::openFile(const_cast<char*>(path.c_str()), 0, 0);
	if(handle == kiv_os::erInvalid_Handle)
	{
		return false;
	}
	kiv_os_vfs::write(handle, (void *)name.c_str(), name.size());
	kiv_os_vfs::close(handle);

	return true;
}

bool remProcessFromProcFS(const kiv_os::THandle pid)
{
	if (pid == kiv_os::erInvalid_Handle)
	{
		return false;
	}

	std::string path = "procfs:/" + std::to_string(pid);
	auto success = kiv_os_vfs::delFile(const_cast<char*>(path.c_str()));
	if (success)
	{
		return false;
	}

	return true;
}