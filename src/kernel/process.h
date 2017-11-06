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

/// Table of running processes
extern std::vector<PCB> process_table;

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Interface funnction
bool HandleProcess(kiv_os::TRegisters &context);

// Working routines
bool routineCloneProcess(kiv_os::TRegisters &context);
bool routineWaitForProcess(kiv_os::TRegisters &context);

kiv_os::THandle getPid();
