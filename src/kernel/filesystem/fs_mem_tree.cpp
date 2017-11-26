#include "VFS.h"
#include "fs_mem_tree.h"

namespace fs_mem_tree {

	kiv_os_vfs::filesys_id _fsid;
	kiv_os_vfs::FsDriver _driver;

	int readBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length) {
		return 1;
	}
	int writeBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length) {
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
		int result;
		_driver.read = readBytes;
		_driver.write = &writeBytes;

		result = kiv_os_vfs::registerDriver(_driver, &_fsid);
		if (result != 0) {
			// clean up after unsuccessfull registration?
		}

		return result;
	}
}