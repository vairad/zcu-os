#pragma once

#include <Windows.h>

#include "kernel.h"
#include "io.h"

#include "process/process_api.h"
#include "filesystem/VFS_startup.h"

#undef stdin
#undef stderr
#undef stdout

#define CREATE_INIT_ERROR 11
#define INITIALISE_VFS_ERROR 12
#define USER_DLL_ERROR 13
#define CREATING_SHELL_ERROR 14
#define WAITING_SHELL_ERROR 15

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

	if (!User_Programs)
	{
		exit(USER_DLL_ERROR);
	}

	if( !kiv_os_vfs::startUp() )
	{
		exit(INITIALISE_VFS_ERROR);
	}

	if( !process::createInit() )
	{
		exit(CREATE_INIT_ERROR);
	}

	// todo: run this code for FS testing
	//kiv_os_vfs::prefillDriveC();
}

void Shutdown_Kernel() {

	process::destructInit();
	kiv_os_vfs::shutdown();
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
		//something went terribly wrong
		exit(CREATING_SHELL_ERROR);
	}

	kiv_os::THandle handles[1];
	handles[0] = regs.rax.x;

	regs.rax.h = kiv_os::scProc;
	regs.rax.l = kiv_os::scWait_For;
	regs.rdx.r = uint64_t(handles);
	regs.rcx.r = 1;

	Sys_Call(regs);

	if (regs.flags.carry == true)
	{
		//something went terribly wrong
		exit(WAITING_SHELL_ERROR);
	}
}

/// ///////////////////////////////////////////////////////////////////
///  Kernel entry point
///
void __stdcall Run_VM() {
	Initialize_Kernel();

	//spustime shell
	runFirstProgram();

	Shutdown_Kernel();
}