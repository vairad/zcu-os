#include "io.h"
#include "kernel.h"
#include "../api/api.h"
#include <Windows.h>

#include "filesystem\VFS.h"

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
		char *fileName = (char*)regs.rdx.r;
		uint64_t flags = regs.rcx.r;
		uint8_t attrs = regs.rdi.e;


		kiv_os::THandle handle = kiv_os_vfs::openFile(fileName, flags, attrs);

		regs.flags.carry = handle == kiv_os::erInvalid_Handle;
		if (!regs.flags.carry) {
			regs.rax.x = handle;
		}
		else {
			regs.rax.r = GetLastError();
		}
	}

	/*
		IN:		dx je handle souboru,
				rdi je pointer na buffer
				rcx je pocet bytu v bufferu k zapsani
		OUT:	rax je pocet zapsanych bytu
	*/
	void readFile(kiv_os::TRegisters &regs) {
		kiv_os::THandle fd = regs.rdx.x;
		void *buffer = reinterpret_cast<void *>(regs.rdi.r);
		uint64_t toBeRead = regs.rcx.r;

		uint64_t read = kiv_os_vfs::read(fd, buffer, toBeRead);

		// error occured
		regs.flags.carry = read == -1;

		if (!regs.flags.carry || read == toBeRead) {
			regs.rax.r = read;
		}
		else {
			regs.rax.r = GetLastError();
		}
	}

	/*
		IN:		dx je handle souboru
				rdi je pointer na buffer, kam zapsat
				rcx je velikost bufferu v bytech
		OUT:	rax je pocet prectenych bytu
	*/
	void writeFile(kiv_os::TRegisters &regs) {
		kiv_os::THandle fd = regs.rdx.x;
		void *buffer = reinterpret_cast<void *>(regs.rdi.r);
		uint64_t toBeWritten = regs.rcx.r;

		uint64_t written = kiv_os_vfs::write(fd, buffer, toBeWritten);

		// error occured
		regs.flags.carry = written == -1 || written != toBeWritten;
		if (!regs.flags.carry) {
			regs.rax.r = written;
		}
		else {
			regs.rax.r = GetLastError();
		}
	}

	/*
		IN:		rdx je pointer na null-terminated ANSI char string udavajici file_name
	*/
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
		kiv_os::THandle handle = regs.rdx.x;

		int error = kiv_os_vfs::close(handle);

		if (regs.flags.carry = error) {
			regs.rax.r = GetLastError();
		}
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