#pragma once

#include <mutex>
#include <condition_variable>

namespace process
{
	
	class waiting_queue
	{
		size_t waiting_count;
		std::mutex condition_lock;
		std::condition_variable condition_variable;

	public:
		waiting_queue();
		void wait();
		void notifyAll();
		void notifyOne();
	};
}