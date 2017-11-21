#pragma once
#include "../api/api.h"

#include <condition_variable>
#include <mutex>

namespace process
{
	enum TState
	{
		sleep,
		runable
	};

	class thread_state
	{
		std::condition_variable cond_var;
		std::mutex cond_lock;

		kiv_os::THandle waked_by;

		TState state;

	public:
		thread_state();
		/**
		 * \brief 
		 * \return state of this thread (sleeping or not)
		 */
		TState get_state() const;


		/**
		 * \brief First call after wake up return value of handle which wake this thread 
		 * another call returns kiv_os::erInvalidHandler
		 * \return handle which wake me up
		 */
		kiv_os::THandle get_wake_by();

		/**
		 * \brief Sleep this thread on its cond var
		 * hint use from this (owner) thread
		 */
		void sleep();

		/**
		 * \brief wake up this thread ond its cond var
		 * hint: use from another thread (not owner)
		 */
		void wake_up(const kiv_os::THandle handle);
	};
}
