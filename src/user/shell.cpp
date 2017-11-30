#include <ctype.h>
#include <cstring>
#include <string>
#include <vector>

#include "shell.h"
#include "rtl.h"
#include "shell_parser.h"
#include "shell_executor.h"

#undef stdin
#undef stderr
#undef stdout

size_t __stdcall shell(const kiv_os::TRegisters &regs) {

	bool run = true;
	while (run) {
		std::string line;
		char readLine[256]; // TODO: Klaus - What size should the buffer be?
		size_t read;
		// TODO: Klaus - Handle bad read.
		bool ok = kiv_os_rtl::Read_File(kiv_os::stdInput, &readLine, sizeof(readLine) - 1, read);
		readLine[read] = 0; // Terminate the line.
		line = readLine;
		std::vector<kiv_os::Command> commands = kiv_os::parseLine(line);
		kiv_os::executeCommands(commands, kiv_os::stdInput, kiv_os::stdOutput, kiv_os::stdError);
	}

	return 0;
}