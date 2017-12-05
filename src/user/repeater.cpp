#include <string>

#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"


namespace repeater_program {

	const uint16_t bufferSize = 256; 

	size_t main(int argc, char *argv[]) {
		char buffer[bufferSize];
		
		size_t read, written;
		kiv_os_rtl::Read_File(kiv_os::stdInput, &buffer, bufferSize, read);
		while (read > 0) {

			kiv_os_rtl::Write_File(kiv_os::stdOutput, &buffer, read, written);
			if (written != read) {
				break;
			}

			kiv_os_rtl::Read_File(kiv_os::stdInput, &buffer, 128, read);
		}

		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall repeater(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("repeater", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return repeater_program::main(static_cast<int>(argv.size()), argv.data());
}