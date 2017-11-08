#pragma once

#include <cstdint>
#include <cstdlib>

#include "../../api/api.h"

typedef uint8_t filesys_id;

namespace ko_vfs {
	const uint8_t driverErr_notLoaded = 1;
	const uint8_t driverReg_err = 1;
	const uint8_t driverReg_noMoreRoom = 2;

	const uint8_t inode_directLinks = 12;

	const uint8_t dentry_fileNameLength = 128;

	const uint16_t fdCount = 0xFFFF;

	struct FsDriver {
		int(*openFile)(char *path, uint8_t flags, uint8_t attrs);
		int(*read)(FileDescriptor *fd, uint8_t *b);
		int(*write)(FileDescriptor *fd, uint8_t b);
	};

	struct Superblock {
		size_t inodeCount;
		size_t blockCount;
		size_t emptyBlocks, emptyInodes;
		size_t blockSize;
		size_t groupBlocks, groupInodes;
		filesys_id fsid;
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

	struct FileDescriptor {
		filesys_id fsid;
		uint8_t status;
		uint64_t position;
		uint64_t size;
		uint16_t openCounter;

		void *data;
	};

	struct Dentry
	{
		char fileName[dentry_fileNameLength];
		size_t inode;
	};



	int init(size_t driverCount);
	int destroy();

	int registerDriver(FsDriver *p_driver, filesys_id *result);

	kiv_os::THandle openFile(char *path, uint8_t flags, uint8_t attrs);
	int read(kiv_os::THandle fd, uint8_t *dest, uint64_t length);
	int write(kiv_os::THandle fd, uint8_t *dest, uint64_t length);
	int delFile(char *path);
	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType);
	int getPos(kiv_os::THandle fd, uint8_t posType);
	int close(kiv_os::THandle fd);

}
