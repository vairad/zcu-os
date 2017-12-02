#include <cstring>

#include "MemtreeMount.h"



MemtreeMount::MemtreeMount(kiv_os_vfs::Superblock *superblock)
{
	this->sb = superblock;
	this->data = new uint8_t[superblock->blockCount * superblock->blockSize];
	this->inodes = new kiv_os_vfs::Inode[superblock->inodeCount];



	this->maxFilesize = this->sb->blockSize * kiv_os_vfs::inode_directLinks;
	this->dirEntrySize = sizeof(MemtreeDirEntry);
}


MemtreeMount::~MemtreeMount()
{
	delete[] this->data;
	delete[] this->inodes;
}

kiv_os_vfs::Inode *MemtreeMount::getNode(node_t n) {
	if (n == kiv_os_vfs::invalidNode || n >= this->sb->inodeCount) {
		return nullptr;
	}

	kiv_os_vfs::Inode *node = this->inodes + n;
	if (node->refCount == 0) {
		return nullptr;
	}

	return node;
}


size_t MemtreeMount::readNode(kiv_os_vfs::Inode *node, uint8_t *dst, size_t from, size_t to) {
	if (from > node->size) {
		return 0;
	}
	if (to > node->size) {
		to = node->size;
	}
	if (from > to) {
		return 0;
	}

	size_t blockN, realBlock, blockOffset;
	for (size_t i = from; i < to; i++) {
		blockN = i / this->sb->blockSize;
		blockOffset = i % this->sb->blockSize;

		realBlock = node->directBlocks[blockN];
		dst[i - from] = this->data[realBlock * this->sb->blockSize + blockOffset];
	}

	return to - from;
}
size_t MemtreeMount::read(node_t n, uint8_t *dst, size_t from, size_t to) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return 0;
	}

	return readNode(node, dst, from, to);
}

size_t MemtreeMount::writeNode(kiv_os_vfs::Inode *node, uint8_t *src, size_t from, size_t to) {
	if (from > node->size) {
		return 0;
	}
	if (to > this->maxFilesize) {
		to = this->maxFilesize;
	}
	if (from > to) {
		return 0;
	}

	size_t blockN, realBlock, blockOffset;
	for (size_t i = from; i < to; i++) {
		blockN = i / this->sb->blockSize;
		blockOffset = i % this->sb->blockSize;

		realBlock = node->directBlocks[blockN];
		this->data[realBlock * this->sb->blockSize + blockOffset] = src[i - from];
	}

	if (to > node->size) {
		node->size = to;
	}

	return to - from;
}
size_t MemtreeMount::write(node_t n, uint8_t *src, size_t from, size_t to) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return 0;
	}

	return writeNode(node, src, from, to);
}


bool MemtreeMount::setSize(node_t n, size_t size) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return false;
	}

	if (size > this->maxFilesize) {
		return false;
	}

	node->size = size;
	return true;
}
size_t MemtreeMount::getSize(node_t n) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return 0;
	}
	return node->size;
}

bool isDirectoryNode(kiv_os_vfs::Inode *test) {
	if (test == nullptr) {
		return false;
	}

	return ((test->mode & kiv_os::faDirectory) != 0);
}

bool MemtreeMount::isDirectory(node_t n) {
	return isDirectoryNode(this->getNode(n));
}

node_t MemtreeMount::findInDirectory(node_t dir, const char *member) {
	kiv_os_vfs::Inode *currentDir = getNode(dir);
	if (!isDirectoryNode(currentDir)) {
		return kiv_os_vfs::invalidNode;
	}

	MemtreeDirEntry dirEntry;
	size_t readEntries = 0, readBytes;
	while (readEntries * this->dirEntrySize < currentDir->size)
	{
		readBytes = this->readNode(currentDir, (uint8_t *)(&dirEntry), readEntries * this->dirEntrySize, (readEntries + 1) * this->dirEntrySize);
		if (readBytes != this->dirEntrySize) {
			return kiv_os_vfs::invalidNode;
		}
		if (std::strcmp(member, dirEntry.apiEntry.file_name) == 0) {
			return dirEntry.node;
		}
		readEntries++;
	}
	
	return kiv_os_vfs::invalidNode;
}