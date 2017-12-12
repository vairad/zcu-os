#pragma once

#include "semaphore.h"
#include "filesystem\VFS.h"

#undef stdin
#undef stderr
#undef stdout

class pipe
{
	typedef FdStatus PipeStatus;

	static const size_t PIPE_SIZE = 1024;

	std::mutex attr_lock, write_lock;
	semaphore::semaphore empty;
	semaphore::semaphore full;

	uint8_t buffer[PIPE_SIZE];
	size_t read_index;
	size_t write_index;
	size_t fields_count;

	FdStatus status = kiv_os_vfs::fdStatus_idle;

	size_t getReadIndex();
	size_t getWriteIndex();

public:
	
	pipe();
	

	/**
	* \brief Dummy function for read from in of pipe
	* \return zero readed bytes
	*/
	//size_t read_in(uint8_t* buf, const size_t nbytes) const;

	/**
	* \brief Function for write in to the pipe
	* \return written bytes count
	*/
	size_t write_in(const uint8_t* buf, const size_t nbytes);


	/**
	* \brief Function for read from in of pipe
	* \return zero readed bytes
	*/
	size_t read_out(uint8_t* buf, const size_t nbytes);
	/**
	* \brief Dummy function for read from in of pipe
	* \return zero readed bytes
	*/
	//size_t write_out(const uint8_t* buf, const size_t nbytes) const;

	bool isOpenWrite();
	bool isOpenRead();
	bool isEmpty();


	bool readMakeSense();
	bool writteMakeSense();

	bool statusContains(PipeStatus ps);
	bool close(PipeStatus ps);

};
