#include "waiting_queue.h"

process::waiting_queue::waiting_queue()
{
	waiting_count = 0;
}

void process::waiting_queue::notifyAll()
{
	std::unique_lock<std::mutex> lck(condition_lock);
	condition_variable.notify_all();
	waiting_count = 0;
}

void process::waiting_queue::notifyOne()
{
	std::unique_lock<std::mutex> lck(condition_lock);
	waiting_count--;
	condition_variable.notify_one();
}

void process::waiting_queue::wait()
{
	std::unique_lock<std::mutex> lck(condition_lock);
	waiting_count++;
	condition_variable.wait(lck);
}
