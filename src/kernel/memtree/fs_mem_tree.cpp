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

	void createDescriptor(kiv_os_vfs::FileDescriptor *fd, node_t node, uint16_t attrs, size_t size = 0) {
		fd->attributes = attrs;
		fd->openCounter = 1;
		fd->superblockId = _superblock;

		fd->inode = node;
		fd->position = 0;
		fd->size = size;
	}


	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
		char *immediatePart, *rest;
		
		MemtreeMount *mm = mountPoints[0];

		node_t currentFolder = mm->getRootNode();

		vfs_paths::immediatePathPart(path, &immediatePart, &rest);

		while (rest != nullptr) {
			if (!mm->isDirectory(currentFolder)) {
				// there is still some path to be walked but current node is not a directory
				return 1;
			}
			currentFolder = mm->findInDirectory(currentFolder, immediatePart);
			if (currentFolder == kiv_os_vfs::invalidNode) {
				// current immediate part was not found in parent folder
				return 1;
			}

			vfs_paths::immediatePathPart(rest, &immediatePart, &rest);
		}

		node_t desiredFile = mm->findInDirectory(currentFolder, immediatePart);

		if (desiredFile != kiv_os_vfs::invalidNode) {
			// file is found within a folder 
			bool fileTypeMatches = ((attrs & kiv_os::faDirectory) != 0) == mm->isDirectory(desiredFile);
			if (!fileTypeMatches) {
				return 2;
			}
			createDescriptor(fd, desiredFile, attrs, mm->getSize(desiredFile));

			return 0;
		}

		// parent folder was found but the final file was not
		if (flags & kiv_os::fmOpen_Always) {
			return 2;
		}
		desiredFile = mm->createFile(currentFolder, immediatePart, attrs);
		if (desiredFile == kiv_os_vfs::invalidNode) {
			return 3;
		}

		createDescriptor(fd, desiredFile, attrs);

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