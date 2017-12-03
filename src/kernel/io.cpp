#include <Windows.h>

#include "io.h"

#include "filesystem/VFS.h"
#include "process/process_api.h"

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"

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


		kiv_os::THandle vfsHandle = kiv_os_vfs::openFile(fileName, flags, attrs);

		kiv_os::THandle processHandle = vfsHandle; // todo: use process handle mapping = process::setNewFD(vfsHandle);


		regs.flags.carry = processHandle == kiv_os::erInvalid_Handle;
		if (!regs.flags.carry) {

			const auto proc_handle = process::setNewFD(vfsHandle);
			regs.flags.carry = proc_handle == kiv_os::erInvalid_Handle;
			if (!regs.flags.carry) {
				regs.rax.x = proc_handle;
			}
			else {
				regs.rax.r = GetLastError();
			}
			
		}
		else {
			regs.rax.r = GetLastError();
		}
	}

	/*
	IN:		rdx je pointer na null-terminated ANSI char string udavajici file_name
	*/
	void deleteFile(kiv_os::TRegisters &regs) {
		char *fileName = (char*)regs.rdx.r;
		int error = kiv_os_vfs::delFile(fileName);

		regs.flags.carry = error;
		if (error) {
			regs.rax.r = error;
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
		kiv_os::THandle processFd = regs.rdx.x;
		void *buffer = reinterpret_cast<void *>(regs.rdi.r);
		uint64_t toBeRead = regs.rcx.r;

		kiv_os::THandle vfsFd = process::getSystemFD(processFd);

		uint64_t read = kiv_os_vfs::read(vfsFd, buffer, toBeRead);

		// error occured
		regs.flags.carry = read == -1;

		if (!regs.flags.carry || read == toBeRead) {
			regs.rax.r = read;
		}
		else {
			//regs.rax.r = GetLastError();
			regs.rax.r = kiv_os::erInvalid_Handle;
		}
	}

	/*
		IN:		dx je handle souboru
				rdi je pointer na buffer, kam zapsat
				rcx je velikost bufferu v bytech
		OUT:	rax je pocet prectenych bytu
	*/
	void writeFile(kiv_os::TRegisters &regs) {
		kiv_os::THandle processFd = regs.rdx.x;
		void *buffer = reinterpret_cast<void *>(regs.rdi.r);
		uint64_t toBeWritten = regs.rcx.r;

		kiv_os::THandle vfsFd = process::getSystemFD(processFd);

		uint64_t written = kiv_os_vfs::write(vfsFd, buffer, toBeWritten);

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
		IN:		dx je handle souboru
				rdi je nova pozice v souboru
				cl konstatna je typ pozice [fsBeginning|fsCurrent|fsEnd],
				ch == 0 jenom nastavn pozici (fsSet_Position)
				ch == 1 nastav pozici a nastav velikost souboru na tuto pozici (fsSet_Size)
	*/
	void setFilePos(kiv_os::TRegisters &regs) {
		kiv_os::THandle processFd = regs.rdx.x;
		size_t position = regs.rdi.r;
		uint8_t posType = regs.rcx.l;
		uint8_t setSize = regs.rcx.h;


		kiv_os::THandle vfsFd = process::getSystemFD(processFd);

		int error = kiv_os_vfs::setPos(vfsFd, position, posType, setSize);

		regs.flags.carry = error;
		if (error) {
			regs.rax.r = GetLastError();
		}
	}

	/*
		IN:		dx je handle souboru
				rcx je typ pozice [fsBeginning|fsCurrent|fsEnd]
		OUT:	rax je pozice v souboru
	*/
	void getFilePos(kiv_os::TRegisters &regs) {
		kiv_os::THandle processFd = regs.rdx.x;
		uint8_t posType = regs.rcx.l;

		kiv_os::THandle vfsFd = process::getSystemFD(processFd);

		size_t position;
		int error = kiv_os_vfs::getPos(vfsFd, &position, posType);

		regs.flags.carry = error;
		if (!error) {
			regs.rax.r = position;
		}
		else {
			regs.rax.r = GetLastError();
		}
	}

	/*
		IN:		dx  je handle libovolneho typu k zavreni
	*/
	void closeHandle(kiv_os::TRegisters &regs) {
		kiv_os::THandle handle = regs.rdx.x;
		const auto sys_handle = process::getSystemFD(handle);

		int error = kiv_os_vfs::close(sys_handle);

		regs.flags.carry = error;
		if (error) {
			regs.rax.r = GetLastError();
		}

		process::removeProcessFD(handle);
	}

	/*
		IN:		rdx je pointer na ANSI char buffer, rcx je velikost buffer
		OUT:	rax pocet zapsanych znaku
	*/
	void getWorkDir(kiv_os::TRegisters &regs) {
		char * buffer = reinterpret_cast<char *>(regs.rdx.r);
		size_t size = regs.rcx.r;

		std::string wd = process::getWorkingDir();
		size_t wd_size = wd.size();

		if( wd_size > size )
		{
			regs.flags.carry = true;
			regs.rax.r = kiv_os::erInvalid_Handle; //TODO RVA consider beter retval
		}

		if ( strcpy_s(buffer, size, wd.c_str()))
		{
			//non zero return strcpy_s represent fail
			regs.flags.carry = true;
			regs.rax.r = kiv_os::erInvalid_Handle; //TODO RVA consider beter retval
		}
		regs.flags.carry = false;
	}

	/*
		IN:		rdx je pointer na null-terminated ANSI char string udavajici novy adresar (muze byt relativni cesta)
	*/
	void setWorkDir(kiv_os::TRegisters &regs) {
		const auto c_path = reinterpret_cast<char *>(regs.rdx.r);

		//TODO RVA absolutize path
		bool success = false;
		//if ( kiv_os_vfs::checkDireExist(path)) //TODO RVA check on vfs changed folder
		{
			const std::string path = c_path;
			success = process::changeWorkingDir(path);
		}

		regs.flags.carry = !success;
	}

	/*
		IN:		rdx je pointer na pole dvou Thandle - prvni zapis a druhy pro cteni z pipy
	*/
	void createPipe(kiv_os::TRegisters &regs) {
		kiv_os::THandle *handles = reinterpret_cast<kiv_os::THandle *>(regs.rdx.r);
		kiv_os::THandle in;
		kiv_os::THandle out;

		const auto err = kiv_os_vfs::openPipe(&in, &out);
		if(err == 0)
		{
			handles[0] = process::setNewFD(in);
			handles[1] = process::setNewFD(out);
		}
		regs.flags.carry = (err != 0);
	}

	/*
		IN:		rdx - pointer

		OUT:	rax.x - file attributes according to api.h
	*/
	void getFileAttributes(kiv_os::TRegisters &regs) {
		kiv_os::THandle procFd = regs.rdx.x;
		kiv_os::THandle vfsHandle = process::getSystemFD(procFd);

		const auto err = kiv_os_vfs::getFileAttributes(vfsHandle, &regs.rax.x);

		regs.flags.carry = err != 0;
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
	case kiv_os::scGetFileAttributes:
		return kiv_os_io::getFileAttributes(regs);

	default:
		return kiv_os_io::illegalAL(regs);
	}

}