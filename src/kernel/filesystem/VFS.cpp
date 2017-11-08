#include "VFS.h"

namespace ko_vfs {

	uint8_t _max_fs_driver_count;
	uint8_t _fs_driver_count;
	FsDriver **_fs_drivers;

	FileDescriptor files[fdCount];

	int init(size_t driverCount) {
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
			return driverReg_err;
		}
		_fs_drivers[_fs_driver_count] = p_driver;
		*result = _fs_driver_count;

		return 0;
	}

	int read(kiv_os::THandle fd, uint8_t *dest, uint64_t length) {
		FileDescriptor *fDesc = files + fd;
		FsDriver *driver = _fs_drivers[fDesc->fsid];
		if (driver == nullptr) {
			return driverErr_notLoaded;
		}

		int status = 0;
		uint64_t i;

		for (i = 0; i < length; i++) {
			status += (driver->read(fDesc, dest + i));
			// todo: check for error every read call?
		}

		return status;
	}

	int write(kiv_os::THandle fd, uint8_t *src, uint64_t length) {
		FileDescriptor *fDesc = files + fd;
		FsDriver *driver = _fs_drivers[fDesc->fsid];
		if (driver == nullptr) {
			return driverErr_notLoaded;
		}

		int status = 0;
		uint64_t i;

		for (i = 0; i < length; i++) {
			status += (driver->write(fDesc, src[i]));
			// todo: check for error every write call?
		}

		return status;
	}

	kiv_os::THandle openFile(char *path, uint8_t flags, uint8_t attrs) {
		return kiv_os::erInvalid_Handle;
	}

	int write(kiv_os::THandle fd, uint8_t *dest, uint64_t length) {
		return 1;
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
		if (newPosition > files[fd].size) {
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