#include "rtl.h"

#include <atomic>

extern "C" __declspec(dllimport) void __stdcall Sys_Call(kiv_os::TRegisters &context);


std::atomic<size_t> Last_Error;

size_t Get_Last_Error() {
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

bool kiv_os_rtl::Close_File(const kiv_os::THandle file_handle) {
	kiv_os::TRegisters regs = Prepare_SysCall_Context(kiv_os::scIO, kiv_os::scClose_Handle);
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	return Do_SysCall(regs);
}