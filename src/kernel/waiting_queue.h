#pragma once

#include "..\api\api.h"

#include <mutex>
#include <list>

namespace process
{
	
	class base_waiting_queue
	{
	protected:
		std::mutex queue_lock;
		std::list<kiv_os::THandle> waiting_handles;
		bool is_process;
	public:
		base_waiting_queue(const bool isProcess);

	//work with queue
		/**
		 * \brief add my handle to waiting queue for this handle
		 * \param handle id of handle 
		 */
		void wait(kiv_os::THandle handle);

		/**
		 * \brief wake up all handles from my queue
		 */
		void notifyAll();

		/**
		 * \brief wake up one handle from my queue
		 */
		void notifyOne();

		/**
		 * \brief return size of waiting queue 
		 * \return count of registered waiters
		 */
		size_t size();
	};

//============================================================================================

	class process_waiting_queue : public base_waiting_queue
	{
	public:
		process_waiting_queue();
	};

	class thread_waiting_queue : public base_waiting_queue
	{
	public:
		thread_waiting_queue();
	};
}
