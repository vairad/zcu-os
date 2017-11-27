#include "VFS.h"

#include <cstring>

namespace kiv_os_vfs {

	const char *mountSeparator = ":";
	const char *pathSeparator = "/";

	const size_t pathBufferSize = 1024;


	uint8_t _fs_driver_count_max;
	uint8_t _fs_driver_count;
	FsDriver *_fs_drivers;

	uint8_t _fs_mount_count_max;
	uint8_t _fs_mount_count;
	Superblock *_superblocks;

	const kiv_os::THandle fdCount = 0xFFFF;
	FileDescriptor files[fdCount];

	Superblock *resolveSuperblock(char *path) {
		char *label = nullptr, *rest = nullptr;
		label = strtok_s(path, mountSeparator, &rest);

		for (int i = 0; i < _fs_mount_count; i++) {
			if (!strcmp(label, (_superblocks + i)->label)) {
				return _superblocks + i;
			}
		}

		return nullptr;
	}

	int resolveFolder(char **path, Superblock **sb) {
		char *mountSeparatorPos = strstr(*path, mountSeparator);

		if (mountSeparatorPos == nullptr) {
			*sb = _superblocks;
			return 0;
		}

		*sb = resolveSuperblock(*path);
		if (sb == nullptr) {
			return 1;
		}

		*path += strnlen((*sb)->label, mountpointLabelSize);
		if (**path == '/') {
			*path += 1;
		}

		return 0;
	}

	int resolveOpenedFd(kiv_os::THandle fd, FileDescriptor **fDesc, Superblock **sb, FsDriver **driver) {
		// todo: tweak error state checking
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}

		*fDesc = files + fd;
		if ((*fDesc)->openCounter < 1) {
			return 2;
		}

		*sb = _superblocks + (*fDesc)->superblockId;
		if ((*sb)->connections < 1) {
			return 3;
		}
		*driver = _fs_drivers + (*sb)->filesys_id;

		return 0;
	}

	int init(uint8_t driverCount, uint8_t fsMountCapacity) {
		_fs_drivers = new __nothrow FsDriver[driverCount];
		_superblocks = new __nothrow Superblock[fsMountCapacity];

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


	int registerDriver(FsDriver &p_driver, filesys_id *result) {
		if (_fs_driver_count >= _fs_driver_count_max) {
			return driverReg_err;
		}
		_fs_drivers[_fs_driver_count] = p_driver;
		*result = _fs_driver_count;

		_fs_driver_count++;

		return 0;
	}

	int mountDrive(char *label, Superblock &sb) {
		if (strlen(label) > mountpointLabelSize) {
			return mountErr_labelTooLong;
		}
		if (_fs_mount_count >= _fs_mount_count_max) {
			return mountErr_noMoreRoom;
		}

		strcpy_s(sb.label, label);

		_superblocks[_fs_mount_count] = sb;
		_fs_mount_count++;

		return 0;
	}


	int read(kiv_os::THandle fd, void *dest, uint64_t length) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}

		return driver->read(fDesc, dest, length);
	}

	int write(kiv_os::THandle fd, void *src, uint64_t length) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}

		return driver->write(fDesc, src, length);
	}

	kiv_os::THandle openFile(char *path, uint8_t flags, uint8_t attrs) {
		// copy path to prevent modification of original

		char pathCpy[pathBufferSize];
		strcpy_s(pathCpy, pathBufferSize, path);

		char *pathCpyP = (char *)pathCpy;
		char **remainingPath = &pathCpyP;

		Superblock *sb;
		if (resolveFolder(remainingPath, &sb)) {
			return kiv_os::erInvalid_Handle;
		}

		kiv_os::THandle fd = kiv_os::erInvalid_Handle;
		for (kiv_os::THandle i = 0; i < fdCount; i++) {
			if ((files + i)->openCounter < 1) {
				fd = i;
				break;
			}
		}

		if (fd == kiv_os::erInvalid_Handle) {
			return fd;
		}
		int result = (_fs_drivers + sb->filesys_id)->openFile(*remainingPath, flags, attrs, files + fd);

		// if opening failed, mark this fd as unopened, usable
		if (result != 0) {
			(files + fd)->openCounter = 0;
			return kiv_os::erInvalid_Handle;
		}
		sb->connections++;

		return fd;
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
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return 1;
		}

		int error = driver->closeDescriptor(fDesc);
		if (error) {
			return 2;
		}

		if (fDesc->openCounter < 1) {
			// cleanup after fDescriptor
		}

		return 0;
	}
}
