#include "fs_mem_tree.h"
#include "MemtreeMount.h"

#include "../filesystem/VFS.h"
#include "../filesystem/VFS_paths.h"



#undef stdin
#undef stderr
#undef stdout

namespace fs_mem_tree {

	filesys_t _fsid;

	MemtreeMount *mountPoints[1];
	sblock_t _superblock;

	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		char *immediatePart, *rest;
		
		MemtreeMount *mm = mountPoints[0];
		node_t currentFolder = 0, parent = 0;

		vfs_paths::immediatePathPart(path, &immediatePart, &rest);

		while (rest != nullptr) {
			node_t immediate = mm->findInDirectory(currentFolder, immediatePart);
			vfs_paths::immediatePathPart(rest, &immediatePart, &rest);
		}

		node_t desiredFile = mm->findInDirectory(currentFolder, immediatePart);

		if (desiredFile != kiv_os_vfs::invalidNode) {

		}
		else {

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

	int readBytes(kiv_os_vfs::FileDescriptor *fd, void *dst, size_t length) {
		MemtreeMount *mm = mountPoints[0];

		size_t read = mm->read(fd->inode, (uint8_t *)dst, fd->position, fd->position + length);
		fd->position += read;

		return (int)read;
	}
	int writeBytes(kiv_os_vfs::FileDescriptor *fd, void *src, size_t length) {
		MemtreeMount *mm = mountPoints[0];

		size_t written = mm->write(fd->inode, (uint8_t *)src, fd->position, fd->position + length);
		fd->position += written;

		return (int)written;
	}

	int mountDrive(char *label, node_t inodes, block_t blocks, size_t blockSize) {
		if (mountPoints[0] != nullptr) {
			return 1;
		}

		kiv_os_vfs::Superblock * sb = new kiv_os_vfs::Superblock;

		sb->filesys_id = _fsid;
		sb->blockSize = blockSize;
		sb->blockCount = sb->emptyBlocks = blocks;
		sb->inodeCount = sb->emptyInodes = inodes;
		sb->connections = 0;

		int mountResult = kiv_os_vfs::mountDrive(label, sb, &_superblock);
		if (mountResult != 0) {
			return mountResult;
		}

		mountPoints[0] = new MemtreeMount(sb);


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