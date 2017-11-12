#include <ctype.h>
#include <cstring>
#include <string>
#include <vector>

#include "shell.h"
#include "rtl.h"
#include "shell_parser.h"

size_t __stdcall shell(const kiv_os::TRegisters &regs) {
	kiv_os::THandle std_in = kiv_os_rtl::Create_File("CONIN$", /*FILE_SHARE_READ*/1);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	kiv_os::THandle std_out = kiv_os_rtl::Create_File("CONOUT$", /*FILE_SHARE_WRITE*/2);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	
	bool run = true;
	while (run) {
		std::string line;
		char readLine[256];
		size_t read;
		bool status = kiv_os_rtl::Read_File(std_in, &readLine, sizeof(readLine) - 1 /*TODO: Klaus - What size should the buffer be?*/, read);
		readLine[read] = 0; // Terminate the line
		line = readLine;
		std::vector<kiv_os::Command> commands = kiv_os::parseLine(line);
	}
	kiv_os_rtl::Close_File(std_in);
	kiv_os_rtl::Close_File(std_out);

	return 0;
}