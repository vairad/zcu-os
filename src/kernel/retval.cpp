#include "retval.h"

void process::retval::make_done(const size_t returned, const size_t waiters_count)
{
	std::unique_lock<std::mutex> lock(access_lock);
	return_value = returned;
	waiters = waiters_count;
	assigned = true;
	if(some_blocked)
	{
		readers.notify_all();
	}
}

bool process::retval::red_ret_val(size_t &returned)
{
	std::unique_lock<std::mutex> lock(access_lock);
	if(!assigned)
	{
		some_blocked = true;
		readers.wait(lock);
	}
	returned = return_value;
	waiters--;

	if(waiters == 0)
	{
		return true;
	}
	return false;
}
