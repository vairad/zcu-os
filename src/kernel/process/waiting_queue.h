#pragma once

#include <mutex>
#include <list>

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"

namespace process
{
	
	class base_waiting_queue
	{
	protected:
		std::mutex queue_lock;
		std::list<kiv_os::THandle> waiting_handles;
		bool is_process;
		bool is_locked;

	public:
		base_waiting_queue(const bool isProcess);

	//work with queue
		/**
		 * \brief add my handle to waiting queue for this handle
		 * \param handle id of handle
		 * \return if thread is still running and caller should be blocked 
		 */
		bool wait(kiv_os::THandle handle);

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

		/**
		 * \brief Method close queue. Next calls of wait will not sleep calling thread.
		 */
		void close();
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
