#pragma once


#include <string>

#undef stdin
#undef stderr
#undef stdout
#include "../api/api.h"


bool HandleProcess(kiv_os::TRegisters &context);

namespace process
{
	// get TID
	kiv_os::THandle getTid();

	// get PID
	kiv_os::THandle getPid();

	// get Parent pid
	kiv_os::THandle getParentPid();

	// get Working directory
	std::string getWorkingDir();

	// method changes (aprooved) working directory of program
	bool changeWorkingDir(const std::string new_dir);

	bool createInit();
	bool destructInit();
	void wakeUpThreadHandle(const kiv_os::THandle handle);
	void wakeUpProcessHandle(const kiv_os::THandle handle);

	//FD interface for io
	kiv_os::THandle getSystemFD(const kiv_os::THandle program_handle);
	kiv_os::THandle setNewFD(const kiv_os::THandle system_handle);
	void removeProcessFD(const kiv_os::THandle program_handle);

	class TStartProcessBlock
	{
	public:
		kiv_os::THandle tid;
		std::string args;
		kiv_os::TEntry_Point entry_point;
		kiv_os::TRegisters proc;
		kiv_os::TProcess_Startup_Info startup_info;

		TStartProcessBlock(const kiv_os::THandle tid, const kiv_os::TEntry_Point entry_point, kiv_os::TRegisters &context)
			: tid(tid)
		{
			this->entry_point = entry_point;
			this->proc = context;
			kiv_os::TProcess_Startup_Info * startup_info = reinterpret_cast<kiv_os::TProcess_Startup_Info *>(context.rdi.r);
			this->startup_info.stdin = startup_info->stdin;
			this->startup_info.stdout = startup_info->stdout;
			this->startup_info.stderr = startup_info->stderr;

			args = startup_info->arg;
			this->startup_info.arg = const_cast<char *>(args.c_str());
		}

		kiv_os::TProcess_Startup_Info *getProcInfo()
		{
			this->startup_info.arg = (char *)(args.c_str());
			return &(this->startup_info);
		}
	};

	class TStartThreadBlock
	{
	public:
		kiv_os::THandle tid;
		std::string args;

		kiv_os::TThread_Proc entry_point;
		void * context;

		TStartThreadBlock(const kiv_os::THandle tid, const kiv_os::TThread_Proc entry_point, void *context)
			: tid(tid)
		{
			this->entry_point = entry_point;
			this->context = context;
		}
	};
}
