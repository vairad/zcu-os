#include "VFS.h"

namespace kiv_os_vfs {

	uint8_t _fs_driver_count_max;
	uint8_t _fs_driver_count;
	FsDriver **_fs_drivers;

	uint8_t _fs_mount_count_max;
	uint8_t _fs_mount_count;
	Superblock **_superblocks;


	FileDescriptor files[fdCount];
	

	int init(uint8_t driverCount, uint8_t fsMountCapacity) {
		_fs_drivers = new __nothrow FsDriver *[driverCount];
		_superblocks = new __nothrow Superblock *[fsMountCapacity];

		if (_fs_drivers == nullptr || _superblocks == nullptr) {
			return 2;
		}
		_fs_driver_count_max = driverCount;
		_fs_driver_count = 0;

		_fs_mount_count_max = fsMountCapacity;
		_fs_mount_count = 0;

		return 0;
	}

	int destroy() {
		// TODO: shutdown functionality for loaded drivers and mounted drives?
		delete[] _fs_drivers;
		delete[] _superblocks;

		_fs_drivers = nullptr;
		_superblocks = nullptr;

		_fs_driver_count_max = _fs_driver_count = 0;
		_fs_mount_count_max = _fs_mount_count = 0;


		return 0;
	}

	int registerDriver(FsDriver *p_driver, filesys_id *result) {
		if (_fs_driver_count >= _fs_driver_count_max) {
			return driverReg_err;
		}
		_fs_drivers[_fs_driver_count] = p_driver;
		*result = _fs_driver_count;

		return 0;
	}

	int read(kiv_os::THandle fd, uint8_t *dest, uint64_t length) {
		FileDescriptor *fDesc = files + fd;

		Superblock *superblock = _superblocks[fDesc->superblockId];
		FsDriver *driver = _fs_drivers[superblock->filesys_id];
		
		if (driver == nullptr) {
			return driverErr_notLoaded;
		}
		
		return driver->read(fDesc, dest, length);
	}

	int write(kiv_os::THandle fd, uint8_t *src, uint64_t length) {
		FileDescriptor *fDesc = files + fd;

		Superblock *superblock = _superblocks[fDesc->superblockId];
		FsDriver *driver = _fs_drivers[superblock->filesys_id];

		if (driver == nullptr) {
			return driverErr_notLoaded;
		}

		
		return driver->write(fDesc, src, length);
	}

	kiv_os::THandle openFile(char *path, uint8_t flags, uint8_t attrs) {
		return kiv_os::erInvalid_Handle;
	}

	int delFile(char *path) {
		return 1;
	}
	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType) {
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}
		if (files[fd].status == 0) {
			return 2;
		}

		size_t newPosition;


		switch (posType) {
		case kiv_os::fsBeginning:
			newPosition = position; break;
		case kiv_os::fsCurrent:
			newPosition = files[fd].position + position; break;
		case kiv_os::fsEnd:
			newPosition = files[fd].size - position; break;
		default: return 3; break;
		}

		// todo: possibly validate position change for each case individually
		if (newPosition > files[fd].size || newPosition < 0) {
			return 4;
		}

		files[fd].position = newPosition;
		return 0;
	}
	int getPos(kiv_os::THandle fd, size_t *position, uint8_t posType) {
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}
		if (files[fd].status == 0) {
			return 2;
		}

		switch (posType) {
		case kiv_os::fsBeginning:
			*position = files[fd].position; break;
		case kiv_os::fsCurrent:
			*position = 0; break;
		case kiv_os::fsEnd:
			*position = files[fd].size - files[fd].position; break;
		default: return 3; break;
		}
		return 0;
	}
	int close(kiv_os::THandle fd) {
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}
		if (files[fd].openCounter > 1) {
			files[fd].openCounter--;
		}
		if (files[fd].status != 0) {
			return 2;
		}
		// todo: possibly use better closed signalization
		files[fd].status = 0;

		return 0;
	}
}