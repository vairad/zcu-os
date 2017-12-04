#pragma once

#include <cstdint>
#include <cstdlib>

#undef stdin
#undef stderr
#undef stdout

#include "../../api/api.h"

#include "inode.h"



typedef uint8_t filesys_t;
typedef uint8_t sblock_t;


namespace kiv_os_vfs {

	const node_t invalidNode = -1;

	const uint8_t mountpointLabelSize = 8;

	const uint8_t driverErr_notLoaded = 1;
	const uint8_t driverReg_err = 1;
	const uint8_t driverReg_noMoreRoom = 2;

	const int mountErr_labelTooLong = 1;
	const int mountErr_noMoreRoom = 2;

	const uint8_t dentry_fileNameLength = 128;

	const char mountSeparator = ':';
	const char pathSeparator = '/';


	struct FileDescriptor {
		sblock_t superblockId;
		node_t inode;

		uint16_t attributes;
		uint8_t status;
		size_t position;
		uint16_t openCounter;
	};


	struct FsDriver {
		int(*openFile)(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd);
		int(*deleteFile)(char *path);

		int(*read)(FileDescriptor *fd, void *dest, size_t length);
		int(*write)(FileDescriptor *fd, void *src, size_t length);

		int(*setPos)(FileDescriptor *fd, size_t position, uint8_t posType, uint8_t setType);
		int(*getPos)(FileDescriptor *fd, size_t *position, uint8_t posType);

		int(*cleanupDescriptor)(FileDescriptor *fd);
	};

	struct Superblock {
		char label[mountpointLabelSize];

		node_t inodeCount, emptyInodes;
		block_t blockCount, emptyBlocks;

		size_t blockSize;
		size_t groupBlocks, groupInodes; // unused
		filesys_t filesys_id;
		size_t connections;
	};


	/*
	Allocates resources for required number of drivers and mounting points

	Returns error value
	*/
	int init(uint8_t driverCount, sblock_t fsMountCapacity, int(*_fs_createPipe)(kiv_os_vfs::FileDescriptor *, kiv_os_vfs::FileDescriptor *));

	/*
	Performs shutdown tasks

	Returns error value
	*/
	int shutdown();

	/*
	Adds given filesystem driver

	Returns error value
	*/
	int registerDriver(FsDriver &p_driver, filesys_t *result);

	/*
	Registers given filesystem under desired label if that label is not taken yet

	Returns error value
	*/
	int mountDrive(char *label, Superblock *superblock, sblock_t *result = nullptr);

	/*
	Looks up coresponding file descriptor and increases its  open counter

	Returns error value
	*/
	int increaseFDescOpenCounter(kiv_os::THandle fd);

	kiv_os::THandle openFile(const char *path, uint64_t flags, uint8_t attrs);
	int delFile(const char *path);
	bool fileExists(const char *path);


	int read(kiv_os::THandle fd, void *dest, uint64_t length);
	int write(kiv_os::THandle fd, void *src, uint64_t length);

	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType);
	int getPos(kiv_os::THandle fd, size_t *position, uint8_t posType);
	int close(kiv_os::THandle fd);

	int openPipe(kiv_os::THandle *fd_in, kiv_os::THandle *fd_out);

	/*
	Looks for file or folder on given path.
	*/


	int getFileAttributes(kiv_os::THandle fd, uint16_t *dest);

}
