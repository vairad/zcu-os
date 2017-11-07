#include "io.h"
#include "kernel.h"
#include "handles.h"

namespace kiv_os_io {
	typedef void(*IoHandle)(kiv_os::TRegisters &regs);

	void illegalAL(kiv_os::TRegisters &regs) {
		// todo:? handle errorneous state
	}

	/*
	*	IN:		rdx je pointer na null-terminated ANSI char string udavajici file_name
				rcx jsou flags k otevreni souboru - viz fm konstanty
				rdi jsou atributy vytvareneho souboru
		OUT:	ax je handle nove otevreneho souboru
	*/
	void createFile(kiv_os::TRegisters &regs) {
		HANDLE result = CreateFileA((char*)regs.rdx.r, GENERIC_READ | GENERIC_WRITE, (DWORD)regs.rcx.r, 0, OPEN_EXISTING, 0, 0);
		//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
		regs.flags.carry = result == INVALID_HANDLE_VALUE;
		if (!regs.flags.carry) regs.rax.x = Convert_Native_Handle(result);
		else regs.rax.r = GetLastError();
	}

	/*
		IN:		dx je handle souboru,
				rdi je pointer na buffer
				rcx je pocet bytu v bufferu k zapsani
		OUT:	rax je pocet zapsanych bytu
	*/
	void readFile(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		dx je handle souboru
				rdi je pointer na buffer, kam zapsat
				rcx je velikost bufferu v bytech
		OUT:	rax je pocet prectenych bytu
	*/
	void writeFile(kiv_os::TRegisters &regs) {
		DWORD written;
		HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
		regs.flags.carry = hnd == INVALID_HANDLE_VALUE;
		if (!regs.flags.carry) regs.flags.carry = !WriteFile(hnd, reinterpret_cast<void*>(regs.rdi.r), (DWORD)regs.rcx.r, &written, NULL);
		if (!regs.flags.carry) regs.rax.r = written;
		else regs.rax.r = GetLastError();
	}

	void deleteFile(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		dx je handle souboru
				rdi je nova pozice v souboru
				cl konstatna je typ pozice [fsBeginning|fsCurrent|fsEnd],
				ch == 0 jenom nastavn pozici (fsSet_Position)
				ch == 1 nastav pozici a nastav velikost souboru na tuto pozici (fsSet_Size)
	*/
	void setFilePos(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		dx je handle souboru
				rcx je typ pozice [fsBeginning|fsCurrent|fsEnd]
		OUT:	rax je pozice v souboru
	*/
	void getFilePos(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		dx  je handle libovolneho typu k zavreni
	*/
	void closeHandle(kiv_os::TRegisters &regs) {
		HANDLE hnd = Resolve_kiv_os_Handle(regs.rdx.x);
		regs.flags.carry = !CloseHandle(hnd);
		if (!regs.flags.carry) Remove_Handle(regs.rdx.x);
		else regs.rax.r = GetLastError();
	}

	/*
		IN:		rdx je pointer na ANSI char buffer, rcx je velikost buffer
		OUT:	rax pocet zapsanych znaku
	*/
	void getWorkDir(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		rdx je pointer na null-terminated ANSI char string udavajici novy adresar (muze byt relativni cesta)
	*/
	void setWorkDir(kiv_os::TRegisters &regs) {

	}

	/*
		IN:		rdx je pointer na pole dvou Thandle - prvni zapis a druhy pro cteni z pipy
	*/
	void createPipe(kiv_os::TRegisters &regs) {

	}
}

void HandleIO(kiv_os::TRegisters &regs) {
	switch (regs.rax.l)
	{
	case kiv_os::scCreate_File:
		return kiv_os_io::createFile(regs);
	case kiv_os::scRead_File:
		return kiv_os_io::readFile(regs);
	case kiv_os::scWrite_File:
		return kiv_os_io::writeFile(regs);
	case kiv_os::scDelete_File:
		return kiv_os_io::deleteFile(regs);
	case kiv_os::scSet_File_Position:
		return kiv_os_io::setFilePos(regs);
	case kiv_os::scGet_File_Position:
		return kiv_os_io::getFilePos(regs);
	case kiv_os::scClose_Handle:
		return kiv_os_io::closeHandle(regs);
	case kiv_os::scSet_Current_Directory:
		return kiv_os_io::setWorkDir(regs);
	case kiv_os::scGet_Current_Directory:
		return kiv_os_io::getWorkDir(regs);
	case kiv_os::scCreate_Pipe:
		return kiv_os_io::createPipe(regs);

	default:
		return kiv_os_io::illegalAL(regs);
	}
	
}