#pragma once

#include <mutex>
#include <condition_variable>

#undef stdin
#undef stderr
#undef stdout

namespace process
{
	class retval
	{
		std::mutex access_lock;
		std::condition_variable readers;
		size_t return_value = -1; //ZERO mean good run
		size_t waiters = 0;
		bool some_blocked = false;
		bool assigned = false;

	public:
	
		/**
		 * \brief set up return value and size of waiters queue
		 * \param returned value returned by program
		 * \param waiters_count number of processes waiting for me
		 * \return success flag
		 */
		void make_done(size_t returned, size_t waiters_count);
		
		
		/**
		 *
		 * \brief read return value or block on readers condition
		 * \param returned reference to field where will be passed return value 
		 * \return succes flag
		 */
		bool red_ret_val(size_t& returned);
	};
}
