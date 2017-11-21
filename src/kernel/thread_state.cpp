#include "thread_state.h"

process::thread_state::thread_state(): waked_by(kiv_os::erInvalid_Handle)
{
	state = runable;
}

process::TState process::thread_state::get_state() const
{
	return state;
}

kiv_os::THandle process::thread_state::get_wake_by()
{
	const auto handle = waked_by;
	waked_by = kiv_os::erInvalid_Handle;
	return handle;
}

void process::thread_state::sleep()
{
	std::unique_lock<std::mutex> lck(cond_lock);
	state = TState::sleep;
	cond_var.wait(lck);
}

void process::thread_state::wake_up(const kiv_os::THandle handle)
{
	std::unique_lock<std::mutex> lck(cond_lock);
	state = TState::runable;
	waked_by = handle;
	cond_var.notify_all();
}
