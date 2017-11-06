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
	std::thread th;
	std::string program_name;
	std::vector<kiv_os::THandle> io_devices;
	std::string working_directory;

	kiv_os::TRegisters register_state; // not needed??
};

/// global variable for next PID (Use in critical section only of PCB table !!!)
kiv_os::THandle next_pid_value = 0;

/// Lock for critical section of PCB Table
std::mutex process_table_lock;

/// Lock for critical section of tid to pid map
std::mutex tid_map_lock;

/// Table of running processes
std::vector<PCB> process_table;

/// Map of TID (simulator machine) to PID (simulated in OS)
std::map<std::thread::id , kiv_os::THandle> tid_to_pid;

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction
bool HandleProcess(kiv_os::TRegisters &context);

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

kiv_os::THandle getPid();
