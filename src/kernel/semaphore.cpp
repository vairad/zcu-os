#include "semaphore.h"

#undef stdin
#undef stderr
#undef stdout

semaphore::semaphore::semaphore(const size_t initialValue)
		: waiting_count(0)
		, semaphore_value(initialValue)
{
}

void semaphore::semaphore::acquire()
{
	std::unique_lock<std::mutex> lck(cond_lock);
	if(semaphore_value <= 0)
	{
		waiting_count++;
		waiting.wait(lck);
		return;
	}
	semaphore_value--;
}

void semaphore::semaphore::release()
{
	std::unique_lock<std::mutex> lck(cond_lock);
	if(waiting_count > 0)
	{
		waiting_count--;
		waiting.notify_one();
		return;
	}
	semaphore_value++;
}
