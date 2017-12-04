#include "VFS.h"
#include "fs_stdio.h"

#include <string>
#include <iostream>
#include <string.h>

#undef stdin
#undef stderr
#undef stdout

namespace fs_stdio {

	enum stream_type
	{
		in, out, none
	};

	size_t in_open_count = 0;
	size_t out_open_count = 0;

	const int inodeCapacity = 3;

	int _registered = 0;

	stream_type inodeToStream[inodeCapacity];

	bool checkCin(const uint16_t openCount)
	{
		if(std::cin.fail() && openCount > 1)
		{
			std::cin.clear();
			return true;
		}
		return false;
	}

	bool checkCout(const uint16_t openCount)
	{
		if (std::cout.fail() && openCount > 1)
		{
			std::cout.clear();
			return true;
		}
		return false;
	}



	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		int freeInode = -1;
		for (int i = 0; i < inodeCapacity; i++) {
			if (inodeToStream[i] == none) {
				freeInode = i;
				break;
			}
		}
		if (freeInode == -1) {
			return 1;
		}

		fd->position = 0;
		fd->openCounter = 1;
		fd->inode = freeInode;

		// todo:? zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby

		if(strcmp(path, "CONIN$") == 0)
		{
			inodeToStream[freeInode] = in;
		}
		else
		{
			inodeToStream[freeInode] = out;
		}
		return 0;
	}

	int readBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		stream_type type = inodeToStream[fd->inode];
		std::string readed = "";
		
		switch (type) 
		{
		case in:
			
			if(!std::cin.good())
			{
				checkCin(fd->openCounter);
				return -1;
			}
			getline(std::cin, readed);
			break;
		default: //none and out is fault
			return -1;
		}
		
		strcpy_s((char *)buffer, length, readed.c_str());
		size_t read = strnlen_s((char *)buffer, length);

		bool error = false;
		if (error)
			return -1;

		return int(read);
	}

	int writeBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		stream_type type = inodeToStream[fd->inode];
		std::string to_write = (char *)buffer;
		const size_t written = to_write.size();

		switch (type)
		{
		case out:
			
			if (!std::cout.good())
			{
				checkCout(fd->openCounter);
				return -1;
			}
			std::cout << to_write;
			break;
		default: //none and in is fault
			return -1;
		}

		return int(written);
	}

	int closeDescriptor(kiv_os_vfs::FileDescriptor *fd) {
		if (fd->openCounter > 1) {
			return 1;
		}

		inodeToStream[fd->inode] = none;
		return 0;
	}

	int mountStdio(filesys_t fs_id) {
		kiv_os_vfs::Superblock *sb = new kiv_os_vfs::Superblock();

		sb->filesys_id = fs_id;
		sb->connections = 0;

		sb->inodeCount = sb->emptyInodes = inodeCapacity;

		int result = kiv_os_vfs::mountDrive("stdio", sb);
		if (result) {
			return result;
		}

		return 0;
	}

	int registerAndMount() {
		if (_registered) {
			return 1;
		}
		_registered = 1;

		for (int i = 0; i < inodeCapacity; i++) {
			inodeToStream[i] = none;
		}

		kiv_os_vfs::FsDriver driver;

		driver.openFile = openFile;
		driver.read = readBytes;
		driver.write = writeBytes;
		driver.cleanupDescriptor = closeDescriptor;

		filesys_t fs_id;

		int result = kiv_os_vfs::registerDriver(driver, &fs_id);
		if (result != 0) {
			return 1;
		}

		return mountStdio(fs_id);
	}

}
