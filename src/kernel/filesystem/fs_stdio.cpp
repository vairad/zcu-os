#include "VFS.h"
#include "fs_stdio.h"

#include <Windows.h>

#undef stdin
#undef stderr
#undef stdout

namespace fs_stdio {

	const int inodeCapacity = 128;

	int _registered = 0;

	HANDLE inodeToHandle[inodeCapacity];


	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		int freeInode = -1;
		for (int i = 0; i < inodeCapacity; i++) {
			if (inodeToHandle[i] == INVALID_HANDLE_VALUE) {
				freeInode = i;
				break;
			}
		}
		if (freeInode == -1) {
			return 1;
		}

		fd->position = 0;
		fd->openCounter = 1;
		fd->inode = freeInode;

		// todo:? zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby

		inodeToHandle[freeInode] = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, (DWORD)flags, 0, OPEN_EXISTING, 0, 0);

		return 0;
	}

	int readBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		HANDLE hnd = inodeToHandle[fd->inode];

		DWORD read;
		if (hnd == INVALID_HANDLE_VALUE) {
			return -1;
		}

		bool error = !ReadFile(hnd, buffer, (DWORD)length, &read, NULL);
		if (error)
			return -1;

		return read;
	}

	int writeBytes(kiv_os_vfs::FileDescriptor *fd, void *buffer, size_t length) {
		HANDLE hnd = inodeToHandle[fd->inode];

		DWORD written;
		if (hnd == INVALID_HANDLE_VALUE) {
			return -1;
		}

		bool error = !WriteFile(hnd, buffer, (DWORD)length, &written, NULL);
		if (error)
			return -1;

		return written;
	}

	int closeDescriptor(kiv_os_vfs::FileDescriptor *fd) {
		if (fd->openCounter < 1) {
			return 1;
		}
		
		HANDLE h = inodeToHandle[fd->inode];
		if (!CloseHandle(h)) {
			return 2;
		}

		inodeToHandle[fd->inode] = INVALID_HANDLE_VALUE;
		return 0;
	}

	int mountStdio(kiv_os_vfs::filesys_id fs_id) {
		kiv_os_vfs::Superblock sb;

		sb.filesys_id = fs_id;
		sb.connections = 0;

		sb.inodeCount = sb.emptyInodes = inodeCapacity;

		int result = kiv_os_vfs::mountDrive("stdio", sb);
		if (result) {
			return result;
		}

		return 0;
	}

	int registerDriver() {
		if (_registered) {
			return 1;
		}
		_registered = 1;

		for (int i = 0; i < inodeCapacity; i++) {
			inodeToHandle[i] = INVALID_HANDLE_VALUE;
		}

		kiv_os_vfs::FsDriver driver;

		driver.openFile = openFile;
		driver.read = readBytes;
		driver.write = writeBytes;
		driver.closeDescriptor = closeDescriptor;

		kiv_os_vfs::filesys_id fs_id;

		int result = kiv_os_vfs::registerDriver(driver, &fs_id);
		if (result != 0) {
			return 1;
		}


		return mountStdio(fs_id);
	}

}
