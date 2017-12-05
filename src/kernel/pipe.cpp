#include "pipe.h"



size_t pipe::getReadIndex()
{
	const auto actual_read = read_index;
	read_index = ++read_index % PIPE_SIZE;
	return actual_read;
}

size_t pipe::getWriteIndex()
{
	const auto actual_write = write_index;
	write_index = ++write_index % PIPE_SIZE;
	return actual_write;
}

pipe::pipe()
	: empty(PIPE_SIZE)
	, full(0)
	, read_index(0)
	, write_index(0)
    , fields_count(0)
{
	status = pipe::status_open;
	memset(buffer, 0, PIPE_SIZE);
}

/*size_t pipe::read_in(uint8_t* buf, const size_t nbytes) const
{
	return 0;
}*/

size_t pipe::write_in(const uint8_t* src, const size_t nbytes)
{
	size_t written = 0;
	if (writteMakeNoSense()) {
		return written;
	}
	for(size_t iter = 0; iter < nbytes; iter++)
	{
		if (writteMakeNoSense()) {
			return written;
		}
		empty.acquire();
		if (writteMakeNoSense()) {
			empty.release();
			return written;
		}
		{
			std::lock_guard<std::mutex> guard(write_lock);
			buffer[getWriteIndex()] = src[iter];
			written++;
			fields_count++;
		}
		full.release();
	}
	
	return written;
}

size_t pipe::read_out(uint8_t* buf, const size_t nbytes)
{
	size_t read = 0;
	for (size_t iter = 0; iter < nbytes; iter++)
	{
		if (readMakeNoSense()) {
			buf[iter] = EOF;
			return read;
		}
		full.acquire();
		if (readMakeNoSense()) {
			buf[iter] = EOF;
			empty.release();
			return read;
		}
		{
			std::lock_guard<std::mutex> guard(write_lock);
			buf[iter] = buffer[getReadIndex()];
			read++;
			fields_count--;
		}
		empty.release();
	}
	return read;
}

/*size_t pipe::pipe::write_out(const uint8_t* buf, const size_t nbytes) const
{
	return 0;
}*/

bool pipe::close(PipeStatus s) {
	bool currentStatus = status & s;

	if (!currentStatus) {
		return 0;
	}
	if (s == pipe::status_open_read) {
		empty.release();
	}
	else if (s == pipe::status_open_write) {
		full.release();
	}

	status = status & ~s;

	return 0;
}

bool pipe::statusContains(PipeStatus s) {
	std::lock_guard<std::mutex> guard(write_lock);
	return (status & s);
}

bool pipe::isOpenWrite() {
	std::lock_guard<std::mutex> guard(write_lock);
	 bool ret = status & pipe::status_open_write;
	 return ret;
}

bool pipe::isOpenRead() {
	std::lock_guard<std::mutex> guard(write_lock);
	bool ret = status & pipe::status_open_read;
	return ret;
}

bool pipe::isEmpty() {
	std::lock_guard<std::mutex> guard(write_lock);
	bool ret = fields_count;
	return !ret;
}

bool pipe::readMakeNoSense()
{
	bool sense = isOpenRead() && !isEmpty();
	return !sense;
}

bool pipe::writteMakeNoSense()
{
	bool nonsense = !isOpenWrite() || !isOpenRead();
	return nonsense;
}


