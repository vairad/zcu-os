#include "waiting_queue.h"
#include "process_api.h"

#undef stdin
#undef stderr
#undef stdout

process::base_waiting_queue::base_waiting_queue(const bool is_process)
	: is_process(is_process)
	, is_locked(false)
{
}

void process::base_waiting_queue::notifyAll()
{
	while(!waiting_handles.empty())
	{
		this->notifyOne();
	}
}

void process::base_waiting_queue::notifyOne()
{
	std::unique_lock<std::mutex> lck(queue_lock);
	if(!waiting_handles.empty())
	{
		const auto handle = waiting_handles.front();
		waiting_handles.pop_front();
		if(is_process)
		{
			process::wakeUpProcessHandle(handle);
		}
		else
		{
			process::wakeUpThreadHandle(handle);
		}
		
	}
}

size_t process::base_waiting_queue::size()
{
	std::unique_lock<std::mutex> lck(queue_lock);
	return waiting_handles.size();
}

void process::base_waiting_queue::close()
{
	std::unique_lock<std::mutex> lck(queue_lock);
	is_locked = true;
}

; bool process::base_waiting_queue::wait(const kiv_os::THandle handle)
{
	std::unique_lock<std::mutex> lck(queue_lock);
	if(is_locked)
	{
		return false;
	}
	waiting_handles.push_back(handle);
	return true;
}


process::process_waiting_queue::process_waiting_queue()
	: base_waiting_queue(true)
{
}

process::thread_waiting_queue::thread_waiting_queue()
	: base_waiting_queue(false)
{
}
