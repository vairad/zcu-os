#include "common.h"

#include <sstream>
#include <string>
#include <vector>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace wc_program {

	void str_replace(std::string& str,
		const std::string& oldStr,
		const std::string& newStr)
	{
		std::string::size_type pos = 0u;
		while ((pos = str.find(oldStr, pos)) != std::string::npos) {
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
	}

	size_t wc_main(int argc, char *argv[]) {
		std::string line = "";
		size_t read;
		size_t buffer_size = 255;
		char buffer[256];
		while ((read = kiv_os_lib::read(buffer, buffer_size)) != 0) {
			buffer[read] = 0; // Terminate the string.
			line.append(buffer);
		}

		int count = 0;
		str_replace(line, "\n", " ");
		str_replace(line, "\r", " ");
		str_replace(line, "\t", " ");

		std::stringstream line_stream(line);
		std::string segment;
		std::vector<std::string> seglist;

		while (std::getline(line_stream, segment, ' '))
		{
			if(segment.size() > 0)
			{
				seglist.push_back(segment);
			}
		}
		line = std::to_string(seglist.size());
		kiv_os_lib::printLn(line.c_str(), line.length());
		return kiv_os_lib::SUCCESS;
	}

}

size_t __stdcall wc(const kiv_os::TRegisters &regs)
{
	std::vector<std::string> args = kiv_os_lib::getArgs("wc", regs);
	std::vector<char *> argv = kiv_os_lib::getArgsDataPointer(args);

	return wc_program::wc_main(static_cast<int>(argv.size()), argv.data());
}
