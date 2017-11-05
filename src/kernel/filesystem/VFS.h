#pragma once

#define VFS_DRIVER_MEM_TREE 0

#define VFS_DRIVER_ERR_NOT_LOADED 1

#define VFS_DRIVER_REG_OK 0
#define VFS_DRIVER_REG_ERR 1
#define VFS_DRIVER_REG_ERR_NO_MORE_ROOM 2

#include <cstdint>
#include <cstdlib>

typedef uint8_t filesys_id;

struct FileDescriptor {
	void *p;
	uint64_t position;
	filesys_id fsid;
};

struct FsDriver {
	(int)(read)(FileDescriptor *fd, uint8_t *b);
	(int)(write)(FileDescriptor *fd, uint8_t b);
};

namespace ko_vfs {

	uint8_t _max_fs_driver_count;
	uint8_t _fs_driver_count;
	FsDriver **_fs_drivers;

	int init(uint8_t driverCount);
	int destroy();

	filesys_id registerDriver(FsDriver *p_driver);

	int read(FileDescriptor *fd, char *dest, uint64_t length);
	int write(FileDescriptor *fd, char *dest, uint64_t length);

}
