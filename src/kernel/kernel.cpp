#pragma once

#include <Windows.h>

#include "kernel.h"
#include "io.h"
#include "process.h"

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
}

void Shutdown_Kernel() {
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


void runProcess()
{
	kiv_os::TRegisters regs;
	regs.rax.h = kiv_os::scProc;
	regs.rax.l = kiv_os::scClone;
	regs.rcx.x = kiv_os::clCreate_Process;

	char * program = "md";

	regs.rdx.r = (uint64_t)program;

	kiv_os::TProcess_Startup_Info procInfo;
	procInfo.OSstderr = kiv_os::erInvalid_Handle;
	procInfo.OSstdin = kiv_os::erInvalid_Handle;
	procInfo.OSstdout = kiv_os::erInvalid_Handle;
	procInfo.arg = "no args";

	regs.rdi.r = (uint64_t) &procInfo;

	Sys_Call(regs);

	regs.rax.r = (uint64_t)0;
}

/// ///////////////////////////////////////////////////////////////////
///  Kernel entry point
///
void __stdcall Run_VM() {
	Initialize_Kernel();

	runProcess();

	//spustime shell - v realnem OS bychom ovsem spousteli login

	kiv_os::TEntry_Point shell = (kiv_os::TEntry_Point)GetProcAddress(User_Programs, "shell");
	if (shell) {
		//spravne se ma shell spustit pres clone!
		kiv_os::TRegisters regs{ 0 };
		shell(regs);
	}

	Shutdown_Kernel();
}