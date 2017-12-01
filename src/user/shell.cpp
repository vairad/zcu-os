#include <ctype.h>
#include <cstring>
#include <string>
#include <vector>

#include "shell.h"
#include "common.h"
#include "shell_parser.h"
#include "shell_executor.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout

size_t __stdcall shell(const kiv_os::TRegisters &regs) {

	bool run = true;
	while (run) {
		char readLine[1024];

		if( !kiv_os_lib::getWD(readLine, sizeof(readLine)))
		{
			run = false;
			continue;
		}

		if( !kiv_os_lib::print(readLine, strlen(readLine)))
		{
			run = false;
			continue;
		}

		readLine[0] = '>';
		readLine[1] = 0;
		if (!kiv_os_lib::print(readLine, strlen(readLine)))
		{
			run = false;
			continue;
		}

		size_t read = kiv_os_lib::read(readLine, sizeof(readLine) - 1);

		if(read != -1)
		{
			readLine[read] = 0; // Terminate the line.
			std::string line = readLine;
			std::vector<kiv_os::Command> commands = kiv_os::parseCommands(line);
			run = kiv_os::executeCommands(commands);
		}
		else
		{
			readLine[0] = '\r';
			readLine[1] = '\n';
			readLine[2] = 0;
			kiv_os_lib::print(readLine, strlen(readLine));
			run = false;
		}
	}

	return 0;
}