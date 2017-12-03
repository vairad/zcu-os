#include "fs_mem_tree.h"
#include "MemtreeMount.h"

#include "../filesystem/VFS.h"
#include "../filesystem/VFS_paths.h"



#undef stdin
#undef stderr
#undef stdout

namespace fs_mem_tree {

	const int fileResolved = 0;
	const int fileMissingPathResolved = 1;
	const int fileMissingPathMissing = 2;


	filesys_t _fsid;

	MemtreeMount *mountPoints[1];
	sblock_t _superblock;

	void createDescriptor(kiv_os_vfs::FileDescriptor *fd, node_t node, uint16_t attrs) {
		fd->attributes = attrs;
		fd->openCounter = 1;
		fd->superblockId = _superblock;

		fd->inode = node;
		fd->position = 0;
	}

	int resolveFile(char *path, node_t *file, node_t *parent, bool createMissingDirs) {
		MemtreeMount *mm = mountPoints[0];
		char *rest;
		char *immediatePart;

		*parent = mm->getRootNode();

		vfs_paths::immediatePathPart(path, &immediatePart, &rest);

		while (rest != nullptr) {
			if (!mm->isDirectory(*parent)) {
				// there is still some path to be walked but current node is not a directory
				return fileMissingPathMissing;
			}
			*file = mm->findInDirectory(*parent, immediatePart);
			if (*file == kiv_os_vfs::invalidNode) {
				// current immediate part was not found in parent folder
				return fileMissingPathMissing;
			}

			vfs_paths::immediatePathPart(rest, &immediatePart, &rest);
		}

		*file = mm->findInDirectory(*parent, immediatePart);

		return fileResolved;
	}

	int openFile(char *path, uint64_t flags, uint8_t attrs, kiv_os_vfs::FileDescriptor *fd) {
	
		MemtreeMount *mm = mountPoints[0];

		node_t parentFolder;
		node_t desiredFile;

		int resolveResult = resolveFile(path, &desiredFile, &parentFolder, false);
		
		if (resolveResult == fileMissingPathMissing) {
			return 1;
		}

		if (resolveResult == fileResolved) {
			// file is found within a folder 
			bool fileTypeMatches = ((attrs & kiv_os::faDirectory) != 0) == mm->isDirectory(desiredFile);
			if (!fileTypeMatches) {
				return 2;
			}
			createDescriptor(fd, desiredFile, attrs);

			return 0;
		}

		// parent folder was found but the final file was not
		if (flags & kiv_os::fmOpen_Always) {
			return 2;
		}
		desiredFile = mm->createFile(parentFolder, immediatePart, attrs);
		if (desiredFile == kiv_os_vfs::invalidNode) {
			return 3;
		}

		createDescriptor(fd, desiredFile, attrs);

		return 0;
	}

	int deleteFile(char *path) {
		node_t parentFolder;
		node_t desiredFile;

		int resolveResult = resolveFile(path, &desiredFile, &parentFolder, false);

		// todo: implement

		return 1;
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

	int setPos(kiv_os_vfs::FileDescriptor *fd, size_t position, uint8_t posType, uint8_t setType) {
		size_t newPosition;
		size_t currentSize;

		MemtreeMount *mm = mountPoints[0];

		currentSize = mm->getSize(fd->inode);
		
		switch (posType) {
		case kiv_os::fsBeginning:
			newPosition = position; break;
		case kiv_os::fsCurrent:
			newPosition = fd->position + position; break;
		case kiv_os::fsEnd:
			newPosition = currentSize - position; break;
		default: return 3; break;
		}

		// todo: possibly validate position change for each case individually
		if (newPosition > currentSize) {
			return 4;
		}

		fd->position = newPosition;
		if (setType & kiv_os::fsSet_Size) {
			mm->setSize(fd->inode, newPosition);
		}

		return 0;
	}
	int getPos(kiv_os_vfs::FileDescriptor *fd, size_t *position, uint8_t posType) {
		MemtreeMount *mm = mountPoints[0];

		switch (posType) {
		case kiv_os::fsBeginning:
			*position = fd->position; break;
		case kiv_os::fsCurrent:
			*position = 0; break;
		case kiv_os::fsEnd:
			*position = mm->getSize(fd->inode) - fd->position; break;
		default: return 3; break;
		}

		return 0;
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
		driver.deleteFile = deleteFile;

		driver.read = readBytes;
		driver.write = writeBytes;

		driver.setPos = setPos;
		driver.getPos = getPos;

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