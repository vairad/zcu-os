#include "pipe.h"

size_t pipe::pipe::getReadIndex()
{
	const auto actual_read = read_index;
	read_index = ++read_index % PIPE_SIZE;
	return actual_read;
}

size_t pipe::pipe::getWriteIndex()
{
	const auto actual_write = write_index;
	write_index = ++write_index % PIPE_SIZE;
	return actual_write;
}

pipe::pipe::pipe()
	: empty(PIPE_SIZE)
	, full(0)
{
}

size_t pipe::pipe::read_in(uint8_t* buf, const size_t nbytes) const
{
	return 0;
}

size_t pipe::pipe::write_in(const uint8_t* buf, const size_t nbytes)
{
	size_t written = 0;
	for(size_t iter = 0; iter < nbytes; iter++)
	{
		empty.acquire();
		buffer[getWriteIndex()] = buf[iter];
		written++;
		full.release();
	}
	return written;
}

size_t pipe::pipe::read_out(uint8_t* buf, const size_t nbytes)
{
	size_t read = 0;
	for (size_t iter = 0; iter < nbytes; iter++)
	{
		full.acquire();
		buf[iter] = buffer[getReadIndex()];
		read++;
		empty.release();
	}
	return read;
}

size_t pipe::pipe::write_out(const uint8_t* buf, const size_t nbytes) const
{
	return 0;
}


