#include "shell.h"

#include "rtl.h"

size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	kiv_os::THandle stdin = kiv_os_rtl::Create_File("CONOUT$", /*FILE_SHARE_WRITE*/2);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	const char* hello = "Hello world!\n";
	size_t written;
	kiv_os_rtl::Write_File(stdin, hello, /*strlen(hello)*/13, written);
	kiv_os_rtl::Close_File(stdin);
	return 0;
}