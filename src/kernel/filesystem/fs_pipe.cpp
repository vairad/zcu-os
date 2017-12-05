#include "fs_pipe.h"
#include "../pipe.h"

namespace fs_pipe {

	const size_t pipeCapacity = 1024;

	int _registered = 0;

	sblock_t superblock;

	pipe *pipes[pipeCapacity];

	void createDescriptor(kiv_os_vfs::FileDescriptor *fd, node_t node, uint16_t attrs) {
		fd->superblockId = superblock;
		fd->position = 0;
		fd->openCounter = 1;
		fd->inode = node;
		fd->attributes = attrs;
	}

	int createPipe(kiv_os_vfs::FileDescriptor *fd_in, kiv_os_vfs::FileDescriptor *fd_out) {
		node_t freePipe = -1;
		for (node_t i = 0; i < pipeCapacity; i++) {
			if (pipes[i] == nullptr) {
				freePipe = i;
				break;
			}
		}
		if (freePipe == -1) {
			return 1;
		}

		pipes[freePipe] = new pipe();

		createDescriptor(fd_in, freePipe, pipe::status_open_write);
		fd_in->status = pipe::status_open_write;

		createDescriptor(fd_out, freePipe, pipe::status_open_read);
		fd_out->status = pipe::status_open_read;

		return 0;
	}

	size_t readBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		pipe *pipe = pipes[fd->inode];

		if (!pipe->statusContains(pipe::status_open_read)) {
			return -1; // file descriptor is not open for reading
		}
		if (!pipe->isOpenRead()) {
			return -1; 
		}

		size_t read = pipe->read_out((uint8_t *)buffer, length);
		// todo: verify read amount / errors
		return read;
	}

	size_t writeBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		pipe *pipe = pipes[fd->inode];

		if (!pipe->statusContains(pipe::status_open_write)) {
			return -1; // file descriptor is not open for reading
		}
		if (!pipe->isOpenWrite()) {
			return -1;
		}

		size_t written = pipe->write_in((uint8_t *)buffer, length);
		// todo: verify write amount / errors
		return written;
	}

	int closeDescriptor(kiv_os_vfs::FileDescriptor *fd) {
		pipe *pipe = pipes[fd->inode];

		if (!pipe->statusContains(fd->status)) {
			return -1; // pipe is already closed from this end
		}

		pipe->close(fd->status);

		if (!pipe->isOpenRead() && !pipe->isOpenWrite()) {
			delete pipes[fd->inode];
			pipes[fd->inode] = nullptr;
		}

		return 0;
	}

	int mountPipe(filesys_t fs_id) {
		kiv_os_vfs::Superblock *sb = new kiv_os_vfs::Superblock;

		sb->filesys_id = fs_id;
		sb->connections = 0;

		sb->inodeCount = sb->emptyInodes = pipeCapacity;

		int result = kiv_os_vfs::mountDrive("pipe", sb, &superblock);
		if (result) {
			return result;
		}

		return 0;
	}

	int registerAndMount() {
		kiv_os_vfs::FsDriver driver;

		driver.openFile = nullptr;
		driver.read = readBytes;
		driver.write = writeBytes;
		driver.cleanupDescriptor = closeDescriptor;

		filesys_t fs_id;

		int result = kiv_os_vfs::registerDriver(driver, &fs_id);
		if (result != 0) {
			return 1;
		}

		return mountPipe(fs_id);
	}

}
