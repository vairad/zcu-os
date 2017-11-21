#pragma once

#include "..\api\api.h"

#include <mutex>
#include <list>

namespace process
{
	
	class waiting_queue
	{
		std::mutex queue_lock;
		std::list<kiv_os::THandle> waiting_handles;
	public:
		waiting_queue();
		
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
	};
}
