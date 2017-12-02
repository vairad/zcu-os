#include "fs_mem_tree.h"
#include "MemtreeMount.h"

#include "../filesystem/VFS.h"
#include "../filesystem/VFS_paths.h"



#undef stdin
#undef stderr
#undef stdout

namespace fs_mem_tree {

	kiv_os_vfs::filesys_t _fsid;

	MemtreeMount *mountPoints[1];

	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		char *immediatePart, *rest;

		vfs_paths::immediatePathPart(path, &immediatePart, &rest);

		while (rest != nullptr) {
			vfs_paths::immediatePathPart(rest, &immediatePart, &rest);
		}

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

	int mountDrive(char *label, node_t inodes, size_t blocks, size_t blockSize) {
		if (mountPoints[0] != nullptr) {
			return 1;
		}

		kiv_os_vfs::Superblock sb;

		sb.filesys_id = _fsid;
		sb.blockSize = blockSize;
		sb.blockCount = sb.emptyBlocks = blocks;
		sb.inodeCount = sb.emptyInodes = inodes;
		sb.connections = 0;

		kiv_os_vfs::Superblock *psb;
		int mountResult = kiv_os_vfs::mountDrive(label, sb, &psb);
		if (mountResult != 0) {
			return mountResult;
		}

		mountPoints[0] = new MemtreeMount(psb);


		return 0;
	}

	int registerDriver() {
		kiv_os_vfs::FsDriver driver;

		driver.openFile = openFile;
		driver.read = readBytes;
		driver.write = writeBytes;
		driver.cleanupDescriptor = nullptr;

		int result = kiv_os_vfs::registerDriver(driver, &_fsid);
		if (result != 0) {
			// clean up after unsuccessfull registration?
		}

		return result;
	}

	int cleanUp() {
		delete mountPoints[0];
		mountPoints[0] = nullptr;

		return 0;
	}
}