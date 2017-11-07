#include "VFS.h"

namespace ko_vfs {

	uint8_t _max_fs_driver_count;
	uint8_t _fs_driver_count;
	FsDriver **_fs_drivers;

	int init(uint8_t driverCount) {
		_fs_drivers = new __nothrow FsDriver *[driverCount];
		if (_fs_drivers == nullptr) {
			return 2;
		}
		_max_fs_driver_count = driverCount;
		_fs_driver_count = 0;

		return 0;
	}

	int destroy() {
		// TODO: shutdown functionality for loaded drivers?
		delete[] _fs_drivers;
		_fs_drivers = nullptr;
		_max_fs_driver_count = 0;
		_fs_driver_count = 0;
	}

	int registerDriver(FsDriver *p_driver, filesys_id *result) {
		if (_fs_driver_count >= _max_fs_driver_count) {
			return VFS_DRIVER_REG_ERR_NO_MORE_ROOM;
		}
		_fs_drivers[_fs_driver_count] = p_driver;
		*result = _fs_driver_count;

		return VFS_DRIVER_REG_OK;
	}

	int read(FileDescriptor *fd, uint8_t *dest, uint64_t length) {
		FsDriver *driver = _fs_drivers[fd->fsid];
		if (driver == nullptr) {
			return VFS_DRIVER_ERR_NOT_LOADED;
		}

		int status = 0;
		uint64_t i;
		
		for (i = 0; i < length; i++) {
			status += (driver->read(fd, dest + i));
			// todo: check for error every read call?
		}

		return status;
	}

	int write(FileDescriptor *fd, uint8_t *src, uint64_t length) {
		FsDriver *driver = _fs_drivers[fd->fsid];
		if (driver == nullptr) {
			return VFS_DRIVER_ERR_NOT_LOADED;
		}

		int status = 0;
		uint64_t i;

		for (i = 0; i < length; i++) {
			status += (driver->write(fd, src[i]));
			// todo: check for error every write call?
		}

		return status;
	}
}