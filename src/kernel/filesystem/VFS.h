#pragma once

#define VFS_DRIVER_MEM_TREE 0

#define VFS_DRIVER_ERR_NOT_LOADED 1

#define VFS_DRIVER_REG_OK 0
#define VFS_DRIVER_REG_ERR 1
#define VFS_DRIVER_REG_ERR_NO_MORE_ROOM 2

#include <cstdint>
#include <cstdlib>

#include "../../api/api.h"

typedef uint8_t filesys_id;

struct FileDescriptor {
	void *p;
	uint64_t position;
	filesys_id fsid;
};

struct FsDriver {
	int(*openFile)(char *path, uint8_t flags, uint8_t attrs);
	int(*read)(FileDescriptor *fd, uint8_t *b);
	int(*write)(FileDescriptor *fd, uint8_t b);
};

namespace ko_vfs {

	int init(uint8_t driverCount);
	int destroy();
	
	int registerDriver(FsDriver *p_driver, filesys_id *result);

	int openFile(char *path, uint8_t flags, uint8_t attrs);
	int read(kiv_os::THandle fd, char *dest, uint64_t length);
	int write(kiv_os::THandle fd, char *dest, uint64_t length);
	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType);
	int getPos(kiv_os::THandle fd, uint8_t posType);
	int close(kiv_os::THandle fd);

}
