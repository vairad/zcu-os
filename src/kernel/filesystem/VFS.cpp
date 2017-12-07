#include <cstring>
#include <condition_variable>

#include "VFS.h"
#include "VFS_paths.h"

#undef stdin
#undef stderr
#undef stdout

namespace kiv_os_vfs {

	const size_t pathBufferSize = 2048;

	std::mutex descriptorsMutex;

	int(*_fs_createPipe)(kiv_os_vfs::FileDescriptor *fd_in, kiv_os_vfs::FileDescriptor *fd_out);


	filesys_t _fs_driver_count_max;
	filesys_t _fs_driver_count;
	FsDriver *_fs_drivers;

	sblock_t _fs_mount_count_max;
	sblock_t _fs_mount_count;
	Superblock **_superblocks;

	const kiv_os::THandle fdCount = 0xFFFF;
	FileDescriptor files[fdCount];

	//		VFS PRIVATE FUNCTIONS

	Superblock *resolveSuperblockByLabel(const char *label, sblock_t *sblockId = nullptr) {
		for (sblock_t i = 0; i < _fs_mount_count; i++) {
			if (_superblocks[i] == nullptr) {
				continue;
			}
			if (!strcmp(label, (_superblocks[i])->label)) {
				if (sblockId != nullptr) {
					*sblockId = i;
				}
				return _superblocks[i];
			}
		}



		return nullptr;
	}

	int resolveSuperblock(char *path, Superblock **sb, char **fsRest, sblock_t *sblockId = nullptr) {
		char pathLabel[mountpointLabelSize] = { 0 };
		char *mountSeparatorPos = strchr(path, mountSeparator);

		if (mountSeparatorPos == nullptr) {
			return 1;
		}

		uint8_t lblLength = (uint8_t)(mountSeparatorPos - path);
		if (lblLength > mountpointLabelSize) {
			return 1;
		}

		strncpy_s(pathLabel, path, lblLength);

		*sb = resolveSuperblockByLabel(pathLabel, sblockId);
		if (sb == nullptr) {
			return 1;
		}

		*fsRest = mountSeparatorPos + 2; // also skip the ":/"
		if (**fsRest == pathSeparator) {
			*fsRest += 1;
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

		*sb = _superblocks[(*fDesc)->superblockId];
		if ((*sb)->connections < 1) {
			return 3;
		}
		*driver = _fs_drivers + (*sb)->filesys_id;

		return 0;
	}

	kiv_os::THandle getFreeDescriptorIndex(kiv_os::THandle searchStart = 0) {
		std::lock_guard<std::mutex> lock(descriptorsMutex);
		
		for (kiv_os::THandle i = searchStart; i < fdCount; i++) {
			if (files[i].status != fdStatus_idle) {
				continue;
			}

			files[i].status = fdStatus_reserved;
			return i;
		}

		return kiv_os::erInvalid_Handle;
	}
	//		VFS STATE FUNCTIONS

	int init(uint8_t driverCount, sblock_t fsMountCapacity, int(*createPipe)(kiv_os_vfs::FileDescriptor *, kiv_os_vfs::FileDescriptor *)) {
		_fs_drivers = new __nothrow FsDriver[driverCount];
		_superblocks = new __nothrow Superblock*[fsMountCapacity];

		_fs_createPipe = createPipe;

		if (_fs_drivers == nullptr || _superblocks == nullptr) {
			return 2;
		}
		_fs_driver_count_max = driverCount;
		_fs_driver_count = 0;

		_fs_mount_count_max = fsMountCapacity;
		_fs_mount_count = 0;

		return 0;
	}

	int shutdown() {
		// TODO: shutdown functionality for loaded drivers and mounted drives?
		delete[] _fs_drivers;
		delete[] _superblocks;

		_fs_drivers = nullptr;
		_superblocks = nullptr;

		_fs_driver_count_max = _fs_driver_count = 0;
		_fs_mount_count_max = _fs_mount_count = 0;


		return 0;
	}


	int registerDriver(FsDriver &p_driver, filesys_t *result) {
		if (_fs_driver_count >= _fs_driver_count_max) {
			return driverReg_err;
		}
		_fs_drivers[_fs_driver_count] = p_driver;
		*result = _fs_driver_count;

		_fs_driver_count++;

		return 0;
	}

	int mountDrive(char *label, Superblock *sb, sblock_t *result) {
		if (strlen(label) > mountpointLabelSize) {
			return mountErr_labelTooLong;
		}
		if (_fs_mount_count >= _fs_mount_count_max) {
			return mountErr_noMoreRoom;
		}

		strcpy_s(sb->label, label);

		_superblocks[_fs_mount_count] = sb;
		if (result != nullptr) {
			*result = _fs_mount_count;
		}

		_fs_mount_count++;

		return 0;
	}

	int increaseFDescOpenCounter(kiv_os::THandle fd) {
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}

		FileDescriptor *fDesc = files + fd;
		if (fDesc->openCounter < 1) {
			return 2;
		}

		fDesc->openCounter++;
		return 0;
	}

	//		SYSCALL FUNCTIONS


	size_t read(kiv_os::THandle fd, void *dest, uint64_t length) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}

		return driver->read(fDesc, dest, length);
	}

	size_t write(kiv_os::THandle fd, void *src, uint64_t length) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}

		return driver->write(fDesc, src, length);
	}

	kiv_os::THandle openFile(const char *path, uint64_t flags, uint8_t attrs) {
		Superblock *sb; sblock_t sblockId;

		char *drivePath;

		char pathBuffer[pathBufferSize] = {0};
		vfs_paths::normalizePath(pathBuffer, path, pathBufferSize);

		if (resolveSuperblock(pathBuffer, &sb, &drivePath, &sblockId)) {
			return kiv_os::erInvalid_Handle;
		}

		kiv_os::THandle fd = getFreeDescriptorIndex();

		if (fd == kiv_os::erInvalid_Handle) {
			return fd;
		}
		int result = (_fs_drivers + sb->filesys_id)->openFile(drivePath, flags, attrs, files + fd);

		// if opening failed, mark this fd as unopened, usable
		if (result != 0) {
			memset(files + fd, 0, sizeof(FileDescriptor));
			return kiv_os::erInvalid_Handle;
		}

		(files + fd)->superblockId = sblockId;

		sb->connections++;

		return fd;
	}

	int delFile(const char *path) {
		Superblock *sb;
		char *drivePath;

		char pathBuffer[pathBufferSize] = { 0 };
		vfs_paths::normalizePath(pathBuffer, path, pathBufferSize);

		if (resolveSuperblock(pathBuffer, &sb, &drivePath)) {
			return 1;
		}

		FsDriver *driver = _fs_drivers + sb->filesys_id;
		if (driver->deleteFile == nullptr) {
			return -1;
		}

		return driver->deleteFile(drivePath);
	}

	bool fileExists(const char *path) {
		kiv_os::THandle fd = openFile(path, kiv_os::fmOpen_Always, kiv_os::faRead_Only);

		bool result = fd != kiv_os::erInvalid_Handle;

		if (result) {
			close(fd);
		}

		return result;
	}

	bool directoryExists(const char *path) {
		kiv_os::THandle fd = openFile(path, kiv_os::fmOpen_Always, kiv_os::faDirectory | kiv_os::faRead_Only);

		bool result = fd != kiv_os::erInvalid_Handle;

		if (result) {
			close(fd);
		}

		return result;
	}

	int setPos(kiv_os::THandle fd, size_t position, uint8_t posType, uint8_t setType) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}
		if (driver->setPos == nullptr) {
			return -1;
		}

		return driver->setPos(fDesc, position, posType, setType);
	}
	int getPos(kiv_os::THandle fd, size_t *position, uint8_t posType) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return -1;
		}
		if (driver->setPos == nullptr) {
			return -1;
		}

		return driver->getPos(fDesc, position, posType);
	}


	int close(kiv_os::THandle fd) {
		FileDescriptor *fDesc;
		Superblock *superblock;
		FsDriver *driver;

		if (resolveOpenedFd(fd, &fDesc, &superblock, &driver)) {
			return 1;
		}

		fDesc->openCounter--;
		if (fDesc->openCounter > 0) {
			// descriptor had multiple references, do nothing
			return 0;
		}

		superblock->connections--;

		if (driver->cleanupDescriptor != nullptr) {
			int error = driver->cleanupDescriptor(fDesc);
			if (error) {
				return 2;
			}
		}

		memset(fDesc, 0, sizeof(FileDescriptor));

		return 0;
	}

	int openPipe(kiv_os::THandle *fd_in, kiv_os::THandle *fd_out) {
		kiv_os::THandle i_in = getFreeDescriptorIndex();
		kiv_os::THandle i_out = getFreeDescriptorIndex(i_in + 1);

		if (i_in == kiv_os::erInvalid_Handle || i_out == kiv_os::erInvalid_Handle) {
			return 1;
		}

		int error = _fs_createPipe(files + i_in, files + i_out);
		if (error) {
			return 2;
		}

		Superblock *sb = resolveSuperblockByLabel("pipe");
		if (sb == nullptr) {
			return 2;
		}

		sb->connections += 2;
		*fd_in = i_in;
		*fd_out = i_out;

		return 0;
	}

	int getFileAttributes(kiv_os::THandle fd, uint16_t *dest) {
		if (fd == kiv_os::erInvalid_Handle) {
			return 1;
		}
		if (files[fd].openCounter == 0) {
			return 2;
		}
		*dest = files[fd].attributes;

		return 0;
	}
}
