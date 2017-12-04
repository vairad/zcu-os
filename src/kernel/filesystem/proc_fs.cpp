#include "VFS.h"
#include "proc_fs.h"

#include <string>
#include <string.h>
#include <map>

#undef stdin
#undef stderr
#undef stdout

namespace fs_process {

	struct proces_block{
		kiv_os::THandle pid;
		std::string name;
		size_t referenced;
	};

	std::map<size_t, proces_block> registered_processes;

	const int processCapacity = 1024;

	int _registered = 0;


	int readFolder(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length)
	{
		kiv_os::TDir_Entry dir;
		size_t index = 0;
		bool success = false;
		for (auto iterator = registered_processes.begin(); iterator != registered_processes.end(); ++iterator)
		{
			if(index == fd->position)
			{
				proces_block process = iterator->second;
				strcpy_s(dir.file_name, sizeof(dir.file_name) , std::to_string(process.pid).c_str());
				dir.file_attributes = kiv_os::faSystem_File;
				success = true;
				break;
			}
			index++;
		}
		if(success)
		{
			*static_cast<kiv_os::TDir_Entry *>(buffer) = dir;
			fd->position++;
			return 1;
		}
		return -1;
	}

	int readFile(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length)
	{
		proces_block proces;
		try
		{
			proces = registered_processes.at(fd->inode);
		}
		catch (std::out_of_range)
		{
			return -1; // file not found
		}
		if(fd->position == proces.name.size())
		{
			return -1; // all readed
		}
		std::string to_read = proces.name.substr(fd->position, length - 1); // -1 for size

		strcpy_s((char *)buffer, length, to_read.c_str());
		size_t read = strnlen_s((char *)buffer, length);

		fd->position = fd->position + read;
		return int(read);
	}

	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		proces_block proces;
		if ( strlen(path) == 0 && (attrs & kiv_os::faDirectory) ) // open root folder
		{
			fd->position = 0;
			fd->openCounter = 1;
			fd->inode = 0;
			fd->attributes = kiv_os::faDirectory;
			return 0;
		}

		proces.pid = atoi(path);
		try
		{
			if(registered_processes.count(proces.pid) > 0 )
			{
				fd->position = 0;
				fd->openCounter = 1;
				fd->inode = proces.pid;

				registered_processes[proces.pid].referenced++;
				return 0;
			}
			registered_processes[kiv_os::erInvalid_Handle] = proces;
			registered_processes.erase(kiv_os::erInvalid_Handle);
		}
		catch (...)
		{
			return 1;
		}

		fd->position = 0;
		fd->openCounter = 1;
		fd->inode = proces.pid;

		registered_processes[proces.pid] = proces;
		return 0;
	}

	int deleteFile(char *path) {
		// todo: implement
		return 1;
	}

	int readBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		if(fd->attributes & kiv_os::faDirectory)
		{
			return readFolder(fd, buffer, length);
		}else
		{
			return readFile(fd, buffer, length);
		}
	}

	int writeBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		proces_block proces;
		try
		{
			proces = registered_processes.at(fd->inode);
		}
		catch (std::out_of_range)
		{
			return -1; // file not found
		}

		std::string to_write = static_cast<char *>(buffer);

		std::string content = proces.name.substr(0, fd->position);
		content += to_write;

		registered_processes[fd->inode].name = content;

		const int written = int(to_write.size());
		fd->position += written;
		return int(written);
	}

	int closeDescriptor(kiv_os_vfs::FileDescriptor *fd) {
		if (fd->openCounter > 1) {
			return 1;
		}

		try
		{
			registered_processes.at(fd->inode);
		}
		catch (std::out_of_range)
		{
			return 0; // file not found -> so its closed
		}
		
		if (registered_processes[fd->inode].referenced > 0) {
			registered_processes[fd->inode].referenced--;
			return 0;
		}

//		registered_processes.erase(fd->inode);
		return 0;
	}

	int mountStdio(filesys_t fs_id) {
		kiv_os_vfs::Superblock *sb = new kiv_os_vfs::Superblock();

		sb->filesys_id = fs_id;
		sb->connections = 0;

		sb->inodeCount = sb->emptyInodes = processCapacity;

		int result = kiv_os_vfs::mountDrive("procfs", sb);
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

		kiv_os_vfs::FsDriver driver;

		driver.openFile = openFile;
		driver.read = readBytes;
		driver.write = writeBytes;
		driver.cleanupDescriptor = closeDescriptor;
		driver.deleteFile = deleteFile;

		filesys_t fs_id;

		int result = kiv_os_vfs::registerDriver(driver, &fs_id);
		if (result != 0) {
			return 1;
		}

		return mountStdio(fs_id);
	}

	//TODO delete records

}
