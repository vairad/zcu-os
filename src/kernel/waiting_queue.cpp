#include "waiting_queue.h"
#include "process_api.h"

process::waiting_queue::waiting_queue()
{
}

void process::waiting_queue::notifyAll()
{
	while(!waiting_handles.empty())
	{
		this->notifyOne();
	}
}

void process::waiting_queue::notifyOne()
{
	std::unique_lock<std::mutex> lck(queue_lock);
	if(!waiting_handles.empty())
	{
		const auto handle = waiting_handles.front();
		waiting_handles.pop_front();
		process::wakeUpHandle(handle);
	}
}

size_t process::waiting_queue::size()
{
	std::unique_lock<std::mutex> lck(queue_lock);
	return waiting_handles.size();
}

void process::waiting_queue::wait(const kiv_os::THandle handle)
{
	std::unique_lock<std::mutex> lck(queue_lock);
	waiting_handles.push_back(handle);
}
