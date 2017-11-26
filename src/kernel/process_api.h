#pragma once

#include "../api/api.h"

#include <string>

bool HandleProcess(kiv_os::TRegisters &context);

namespace process
{
	class TStartBlock
	{
	public:
		bool is_process;
		kiv_os::THandle tid;
		union
		{
			kiv_os::TEntry_Point proc;
			kiv_os::TThread_Proc thread;
		} entry_point;
		union
		{
			kiv_os::TRegisters proc;
			void * thread;
		} context;

		TStartBlock(const kiv_os::THandle tid, const kiv_os::TEntry_Point entry_point, kiv_os::TRegisters &context)
			: is_process(true)
			, tid(tid)
		{
			this->entry_point.proc = entry_point;
			this->context.proc = context;
		}

		TStartBlock(const kiv_os::THandle tid, const kiv_os::TThread_Proc entry_point, void *context)
			: is_process(false)
			, tid(tid)
		{
			this->entry_point.thread = entry_point;
			this->context.thread = context;
		}
	};


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
	void wakeUpThreadHandle(const kiv_os::THandle handle);
	void wakeUpProcessHandle(const kiv_os::THandle handle);
}