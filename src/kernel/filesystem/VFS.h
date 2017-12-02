#pragma once

#include <cstdint>
#include <cstdlib>

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"

namespace kiv_os_vfs {

	typedef uint8_t filesys_id;
	
	typedef uint8_t sblock;

	const uint8_t mountpointLabelSize = 8;

	const uint8_t driverErr_notLoaded = 1;
	const uint8_t driverReg_err = 1;
	const uint8_t driverReg_noMoreRoom = 2;

	const int mountErr_labelTooLong = 1;
	const int mountErr_noMoreRoom = 2;

	const uint8_t inode_directLinks = 12;
	const uint8_t dentry_fileNameLength = 128;


	struct FileDescriptor {
		sblock superblockId;
		size_t inode;
		size_t size;

		uint8_t attributes;
		uint8_t status;
		uint64_t position;
		uint16_t openCounter;
	};

	struct Dentry
	{
		char fileName[dentry_fileNameLength];
		size_t inode;
	};


	struct FsDriver {
		int(*openFile)(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd);
		int(*read)(FileDescriptor *fd, void *b, size_t length);
		int(*write)(FileDescriptor *fd, void *b, size_t length);
		int(*cleanupDescriptor)(FileDescriptor *fd);
	};

	struct Superblock {
		char label[mountpointLabelSize];

		size_t inodeCount;
		size_t blockCount;
		size_t emptyBlocks, emptyInodes;
		size_t blockSize;
		size_t groupBlocks, groupInodes; // unused
		filesys_id filesys_id;
		size_t connections;
	};

	struct Inode {
		uint8_t mode;
		uint16_t refCount;
		uint16_t owner, group; // unused
		size_t size;
		uint32_t atime, mtime, ctime; // unused

		size_t directBlocks[inode_directLinks]; // not implemented
		size_t singleIndirect; // not implemented
		size_t doubleIndirect; // not implemented
		size_t tripleIndirect; // not implemented
		void *data; // todo: remove

	};

	/*
		Allocates resources for required number of drivers and mounting points

		Returns error value
	*/
	int init(uint8_t driverCount, uint8_t fsMountCapacity, int(*_fs_createPipe)(kiv_os_vfs::FileDescriptor *, kiv_os_vfs::FileDescriptor *));
	
	/*
		Performs shutdown tasks

		Returns error value
	*/
	int shutdown();

	/*
		Adds given filesystem driver

		Returns error value
	*/
	int registerDriver(FsDriver &p_driver, filesys_id *result);

	/*
		Registers given filesystem under desired label if that label is not taken yet

		Returns error value
	*/
	int mountDrive(char *label, Superblock &superblock, sblock *mountpoint = nullptr);

	/*
		Looks up coresponding file descriptor and increases its  open counter

		Returns error value
	*/
	int increaseFDescOpenCounter(kiv_os::THandle fd);

	kiv_os::THandle openFile(char *path, uint64_t flags, uint8_t attrs);
	int read(kiv_os::THandle fd, void *dest, uint64_t length);
	int write(kiv_os::THandle fd, void *dest, uint64_t length);
	int delFile(char *path);
	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType);
	int getPos(kiv_os::THandle fd, size_t *position, uint8_t posType);
	int close(kiv_os::THandle fd);

	int openPipe(kiv_os::THandle *fd_in, kiv_os::THandle *fd_out);

	int getFileAttributes(kiv_os::THandle fd, uint8_t *dest);

}
