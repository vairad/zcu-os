#include <locale>
#include <vector>

#include "common.h"
#include "rtl.h"

#undef stdin
#undef stderr
#undef stdout

std::vector<char *> kiv_os_lib::getArgsDataPointer(std::vector<std::string> &args)
{
	std::vector<char *> argv;

	for (auto i = 0; i < args.size(); ++i) {
		argv.push_back( const_cast<char *>(args.at(i).c_str()) );
	}
	return argv;
}

std::vector<std::string> kiv_os_lib::getArgs(char* program_name, const kiv_os::TRegisters& context) {
	kiv_os::TProcess_Startup_Info *info = (kiv_os::TProcess_Startup_Info *)context.rdi.r;
	std::string line = info->arg;
	std::vector<std::string> parts = parseLine(line);
	std::vector<std::string> argv{};

	argv.push_back(program_name); //add program name at first place
	for (auto iterator = parts.begin(); iterator != parts.end(); ++iterator) {
		argv.push_back( *iterator );
	}
	return argv;
}

std::vector<std::string> kiv_os_lib::parseLine(std::string line) {
	std::vector<std::string> parts = std::vector<std::string>();
	std::string token = "";
	bool inQuotes = false;
	for (size_t i = 0; i < line.length(); i++) {
		char c = line[i];
		if (!inQuotes) {
			if (c == ' ') {
				parts.push_back(token);
				token = "";
			} else if (isspace(c, std::locale{})) {
				continue;
			} else if (c == '\"') {
				token.append(1, c);
				inQuotes = true;
			} else {
				token.append(1, c);
			}
		} else {
			if (isspace(c, std::locale{}) && c != ' ') {
				continue;
			} else if (c == '\"') {
				token.append(1, c);
				inQuotes = false;
			} else {
				token.append(1, c);
			}
		}
	}
	// Push last part to vector
	if (!line.empty()) {
		parts.push_back(token);
	}

	return parts;
}

size_t kiv_os_lib::read(const char *buffer, size_t buffer_size) {
	size_t read = -1;
	
	bool ok = kiv_os_rtl::Read_File(kiv_os::stdInput, buffer, buffer_size, read);
	
	if (!ok) { return -1; }
	return read;
}

size_t kiv_os_lib::print(const char *buffer, size_t buffer_size) {
	size_t written = -1;

	bool ok = kiv_os_rtl::Write_File(kiv_os::stdOutput, buffer, buffer_size, written);
	
	if (!ok) { return -1; }
	return written;
}

size_t kiv_os_lib::printLn(const char* buffer, size_t buffer_size)
{
	const size_t written = print(buffer, buffer_size);
	print("\r\n", 2);
	return written;
}

size_t kiv_os_lib::printErr(const char *buffer, size_t buffer_size) {
	size_t written = -1;
	// TODO: Klaus - Handle bad write.
	bool ok = kiv_os_rtl::Write_File(kiv_os::stdError, buffer, buffer_size, written);
	return written;
}

size_t kiv_os_lib::printErrLn(const char* buffer, size_t buffer_size)
{
	const size_t written = printErr(buffer, buffer_size);
	printErr("\r\n", 2);
	return written;
}

bool kiv_os_lib::getWD(const char* buffer, size_t buffer_size)
{
	size_t readed = -1;
	bool ok = kiv_os_rtl::Get_Working_Dir(buffer, buffer_size, readed);
	ok &= (readed <= buffer_size);
	return ok;
}

bool kiv_os_lib::isDir(kiv_os::THandle handle, bool &isDir, uint16_t *attrs) {
	uint16_t attributes;
	bool ok = kiv_os_rtl::Get_File_Attributes(handle, attributes);
	if (ok) {
		isDir = (attributes & kiv_os::faDirectory) == kiv_os::faDirectory;
		if (attrs != nullptr) {
			*attrs = attributes;
		}
	}
	return ok;
}
