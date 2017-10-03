#include "io.h"
#include "kernel.h"
#include "handles.h"

void HandleIO(kiv_os::TRegisters &regs) {

	//V ostre verzi pochopitelne do switche dejte volani funkci a ne primo vykonny kod
	

	switch (regs.rax.l) {
			case kiv_os::scCreate_File: {
				HANDLE result = CreateFileA((char*)regs.rdx.r, GENERIC_READ | GENERIC_WRITE, (DWORD)regs.rcx.r, 0, OPEN_EXISTING, 0, 0);
				//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
				regs.flags.carry = result == INVALID_HANDLE_VALUE;
				if (!regs.flags.carry) regs.rax.x = Convert_Native_Handle(result);
					else regs.rax.r = GetLastError();
			}
			break;	//scCreateFile


		case kiv_os::scWrite_File: {
				DWORD written;
				HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
				regs.flags.carry = hnd == INVALID_HANDLE_VALUE;
				if (!regs.flags.carry) regs.flags.carry = !WriteFile(hnd, reinterpret_cast<void*>(regs.rdi.r), (DWORD)regs.rcx.r, &written, NULL);
				if (!regs.flags.carry) regs.rax.r = written;
					else regs.rax.r = GetLastError();

			}
			break; //scWriteFile


		case kiv_os::scClose_Handle: {
				HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
				regs.flags.carry = !CloseHandle(hnd);
				if (!regs.flags.carry) Remove_Handle(regs.rdx.x);				
					else regs.rax.r = GetLastError();
			}

			break;	//CloseFile
	}
}