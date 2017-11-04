#pragma once

#include <vector>
#include <string>
#include <thread>


#include "../api/api.h"

size_t next_pid_value = 0;

typedef struct program_control_block {
	size_t pid;
	kiv_os::TRegisters register_state;
	std::vector<kiv_os::THandle> io_devices;
	std::string working_directory;

	std::thread th;
} PCB ;
