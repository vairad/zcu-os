#pragma once
#include <condition_variable>

#undef stdin
#undef stderr
#undef stdout

namespace semaphore
{
	class semaphore
	{
		size_t waiting_count;
		std::condition_variable waiting;
		std::mutex	cond_lock;
		size_t semaphore_value;

	public:
		/**
		 * \brief Semaphore constructor
		 * \param initialValue number of acquires before semaphore blocks
		 */
		explicit semaphore(size_t initialValue);
		
		/**
		 * \brief Semaphore operation P() decrease semaphore value.
		 * If semaphore value is zero next call will sleep thread.
		 */
		void acquire(); 

		/**
		 * \brief Semaphore operation V() increase semaphore value.
		 * If semaphore value was zero wake one from wating queue.
		 */
		void release();
	};
}
