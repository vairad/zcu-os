#pragma once

#include <Windows.h>

#include "kernel.h"
#include "io.h"
#include "process_api.h"

#include "filesystem\VFS_startup.h"

#undef stdin
#undef stderr
#undef stdout

HMODULE User_Programs;

void Set_Error(const bool failed, kiv_os::TRegisters &regs) {
	if (failed) {
		regs.flags.carry = true;
		regs.rax.r = GetLastError();
	}
	else
		regs.flags.carry = false;
}


void Initialize_Kernel() {
	User_Programs = LoadLibrary(L"user.dll");	
	process::createInit();
}

void Shutdown_Kernel() {

	//TODO RVA kill all processes
	// process::killAll();
	FreeLibrary(User_Programs);
}

void __stdcall Sys_Call(kiv_os::TRegisters &regs)
{
	switch (regs.rax.h) 
	{
		case kiv_os::scProc:	HandleProcess(regs);
			break;
		case kiv_os::scIO:		HandleIO(regs);
			break;
		default:
			// todo set up error state
			break;
	}

}

/**
 * \brief Run and wait for first program
 */
void runFirstProgram()
{
	const char * programName = "shell";

	kiv_os::TRegisters regs;
	regs.rax.h = kiv_os::scProc;
	regs.rax.l = kiv_os::scClone;
	regs.rcx.x = kiv_os::clCreate_Process;

	regs.rdx.r = reinterpret_cast<uint64_t>(programName);

	kiv_os::TProcess_Startup_Info procInfo;
	procInfo.stderr = kiv_os::stdError;
	procInfo.stdin = kiv_os::stdInput;
	procInfo.stdout = kiv_os::stdOutput;
	procInfo.arg = "";

	regs.rdi.r = reinterpret_cast<uint64_t>(&procInfo);

	Sys_Call(regs);
	
	if(regs.flags.carry == true)
	{
		return; //something went terribly wrong you can add break point here
	}

	kiv_os::THandle handles[1];
	handles[0] = regs.rax.x;

	regs.rax.h = kiv_os::scProc;
	regs.rax.l = kiv_os::scWait_For;
	regs.rdx.r = uint64_t(handles);
	regs.rcx.r = 1;

	Sys_Call(regs);
}

/// ///////////////////////////////////////////////////////////////////
///  Kernel entry point
///
void __stdcall Run_VM() {
	Initialize_Kernel();
	
	kiv_os_vfs::startUp();

	//spustime shell
	runFirstProgram();

	Shutdown_Kernel();
}