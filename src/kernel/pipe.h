#pragma once

#include "semaphore.h"

#undef stdin
#undef stderr
#undef stdout

class pipe
{
	typedef uint8_t PipeStatus;

	static const size_t PIPE_SIZE = 1024;

	std::mutex read_lock, write_lock;
	semaphore::semaphore empty;
	semaphore::semaphore full;

	uint8_t buffer[PIPE_SIZE];
	size_t read_index;
	size_t write_index;
	size_t fields_count;

	PipeStatus status = status_idle;

	size_t getReadIndex();
	size_t getWriteIndex();

public:
	// Pipe is about to be cleaned up
	static const PipeStatus status_idle = 0;
	// A reference to this pipe still exists
	static const PipeStatus status_both_closed = 1;
	// Pipes read end is open
	static const PipeStatus status_open_read = 2;
	// Pipes write end is open
	static const PipeStatus status_open_write = 4;
	// Pipe is fully open
	static const PipeStatus status_open = status_both_closed | status_open_read | status_open_write;

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


	bool readMakeNoSense();
	bool writteMakeNoSense();

	bool statusContains(PipeStatus ps);
	bool close(PipeStatus ps);

};
