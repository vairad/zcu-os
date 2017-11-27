#include "VFS.h"
#include "fs_mem_tree.h"

#undef stdin
#undef stderr
#undef stdout

namespace fs_mem_tree {

	kiv_os_vfs::filesys_id _fsid;

	int openFile(char *path, uint8_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		// todo: locate file according to path
		// todo: cache pointer to the found file

		if (true) {
			return 1;
		}

		fd->openCounter = 1;
		fd->position = 0;

		return 0;
	}

	int readBytes(kiv_os_vfs::FileDescriptor *fd, void *byte, size_t length) {
		return 1;
	}
	int writeBytes(kiv_os_vfs::FileDescriptor *fd, void *byte, size_t length) {
		return 1;
	}

	int mountDrive(char *label, size_t inodes, size_t blocks, size_t blockSize) {
		kiv_os_vfs::Superblock sb;

		sb.filesys_id = _fsid;
		sb.blockSize = blockSize;
		sb.blockCount = sb.emptyBlocks = blocks;
		sb.inodeCount = sb.emptyInodes = inodes;
		sb.connections = 0;

		return kiv_os_vfs::mountDrive(label, sb);
	}

	int registerDriver() {
		kiv_os_vfs::FsDriver driver;

		driver.openFile = openFile;
		driver.read = readBytes;
		driver.write = writeBytes;

		int result = kiv_os_vfs::registerDriver(driver, &_fsid);
		if (result != 0) {
			// clean up after unsuccessfull registration?
		}

		return result;
	}
}