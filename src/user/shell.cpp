#include <ctype.h>
#include <cstring>
#include <string>
#include <vector>

#include "shell.h"
#include "common.h"
#include "shell_parser.h"
#include "shell_executor.h"

#undef stdin
#undef stderr
#undef stdout

size_t __stdcall shell(const kiv_os::TRegisters &regs) {

	bool run = true;
	while (run) {
		std::string line;
		char readLine[1024];
		// TODO: Klaus - Handle bad read.
		size_t read = kiv_os::read(readLine, sizeof(readLine) - 1);
		readLine[read] = 0; // Terminate the line.
		line = readLine;
		std::vector<kiv_os::Command> commands = kiv_os::parseCommands(line);
		run = kiv_os::executeCommands(commands);
	}

	return 0;
}