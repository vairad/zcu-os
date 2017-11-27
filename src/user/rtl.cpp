#include "rtl.h"

#include <atomic>

extern "C" __declspec(dllimport) void __stdcall Sys_Call(kiv_os::TRegisters &context);


std::atomic<size_t> Last_Error;

size_t kiv_os_rtl::Get_Last_Error() {
	return Last_Error;
}

kiv_os::TRegisters Prepare_SysCall_Context(uint8_t major, uint8_t minor) {
	kiv_os::TRegisters regs;
	regs.rax.h = major;
	regs.rax.l = minor;
	return regs;
}

bool Do_SysCall(kiv_os::TRegisters &regs) {
	Sys_Call(regs);

	if (regs.flags.carry) Last_Error = regs.rax.r;
		else Last_Error = kiv_os::erSuccess;

	return !regs.flags.carry;
}



kiv_os::THandle kiv_os_rtl::Create_File(const char* file_name, size_t flags) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scCreate_File);
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(file_name);
	regs.rcx.r = flags;
	Do_SysCall(regs);
	return static_cast<kiv_os::THandle>(regs.rax.x);
}

bool kiv_os_rtl::Write_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scWrite_File);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = buffer_size;	

	const bool result = Do_SysCall(regs);
	written = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Read_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &read) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scRead_File);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = buffer_size;

	const bool result = Do_SysCall(regs);
	read = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Close_File(const kiv_os::THandle file_handle) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scClose_Handle);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	return Do_SysCall(regs);
}


bool kiv_os_rtl::Create_Process(kiv_os::THandle* returned, const char * program, const char * args) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scProc, kiv_os::scClone);
	kiv_os::TProcess_Startup_Info procInfo;
	
	regs.rcx.x = kiv_os::clCreate_Process;

	regs.rdx.r = uint64_t(program);

	//TODO RVA handles as parameter of function
	procInfo.OSstderr = kiv_os::erInvalid_Handle;
	procInfo.OSstdin = kiv_os::erInvalid_Handle;
	procInfo.OSstdout = kiv_os::erInvalid_Handle;
	procInfo.arg = (char*)args; //TODO RVA should be const in api?
	
	regs.rdi.r = uint64_t(&procInfo);
	if(Do_SysCall(regs))
	{
		*returned = regs.rax.x;
		return true;
	}
	*returned = kiv_os::erInvalid_Handle;
	Last_Error = regs.rax.x;
	return false;
}


bool kiv_os_rtl::Join_One_Handle(kiv_os::THandle wait_for) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scProc, kiv_os::scWait_For);
	
	kiv_os::THandle handles[1];
	handles[0] = wait_for;

	regs.rdx.r = uint64_t(handles);
	regs.rcx.r = 1;

	if (Do_SysCall(regs))
	{
		return true;
	}
	return false;
}

bool kiv_os_rtl::Create_Pipe(kiv_os::THandle handles[]) {
	// TODO: Klaus - Implement.
	return false;
}

bool kiv_os_rtl::Get_Working_Dir(const void *wd, const size_t wd_size, size_t &read) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scGet_Current_Directory);

	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(wd);
	regs.rcx.r = wd_size;

	const bool result = Do_SysCall(regs);
	read = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Change_Working_Dir(const void  *path)
{
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scSet_Current_Directory);

	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(path);	

	const bool result = Do_SysCall(regs);
	return result;
}
