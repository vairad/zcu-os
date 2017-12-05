#include <cstring>

#include "MemtreeMount.h"


MemtreeMount::MemtreeMount(kiv_os_vfs::Superblock *superblock)
{
	this->sb = superblock;

	this->maxFilesize = this->sb->blockSize * kiv_os_vfs::inode_directLinks;
	this->dirEntrySize = sizeof(MemtreeDirEntry);

	this->data = new uint8_t[superblock->blockCount * superblock->blockSize];
	this->inodes = new kiv_os_vfs::Inode[superblock->inodeCount];
	this->blockBitmap = new Bitmap(sb->blockCount);

	std::memset(this->data, 0, superblock->blockCount * superblock->blockSize);
	std::memset(this->inodes, 0, superblock->inodeCount * sizeof(kiv_os_vfs::Inode));

	this->rootNode = this->reserveFreeNode(kiv_os::faDirectory | kiv_os::faSystem_File);
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

node_t MemtreeMount::reserveFreeNode(uint16_t attrs) {
	for (node_t i = 0; i < this->sb->inodeCount; i++) {
		kiv_os_vfs::Inode *node = this->inodes + i;
		if (node->refCount != 0) {
			continue;
		}

		node->refCount = 1;
		node->mode = attrs;

		for (block_t b = 0; b < kiv_os_vfs::inode_directLinks; b++) {
			node->directBlocks[b] = invalidBlock;
		}

		return i;
	}

	return kiv_os_vfs::invalidNode;
}

void MemtreeMount::releaseNode(node_t n) {
	kiv_os_vfs::Inode *node = this->getNode(n);
	if (!isValidNode(node)) {
		return;
	}
	for (block_t b = 0; b < kiv_os_vfs::inode_directLinks; b++) {
		this->releaseBlock(node, b);
	}

	memset(node, 0, sizeof(kiv_os_vfs::Inode));
}

block_t MemtreeMount::reserveBlock(kiv_os_vfs::Inode *n, block_t nodeBlock) {
	if (nodeBlock >= kiv_os_vfs::inode_directLinks) {
		return invalidBlock;
	}
	block_t freeBlock = invalidBlock;
	for (block_t b = 0; b < this->sb->blockCount; b++) {
		if (!this->blockBitmap->isSet(b)) {
			freeBlock = b;
			break;
		}
	}
	if (freeBlock != invalidBlock) {
		this->blockBitmap->set(freeBlock);
	}

	return n->directBlocks[nodeBlock] = freeBlock;
}
void MemtreeMount::releaseBlock(kiv_os_vfs::Inode *node, block_t nodeBlock) {
	if (node->directBlocks[nodeBlock] != invalidBlock) {
		this->blockBitmap->clear(node->directBlocks[nodeBlock]);
		node->directBlocks[nodeBlock] = invalidBlock;
	}
}


bool MemtreeMount::deleteDirectory(kiv_os_vfs::Inode *dir) {
	uint16_t readEntries;
	MemtreeDirEntry dirEntry;

	for (readEntries = 0; readEntries * this->dirEntrySize < dir->size; readEntries++) {
		size_t read = this->readNode(dir, (uint8_t *)&dirEntry, readEntries * this->dirEntrySize, (readEntries + 1) * this->dirEntrySize);
		if (read != this->dirEntrySize) {
			break;
		}
		this->deleteFile((node_t)(dir - this->inodes), dirEntry.node);

	}

	return true;
}

//		PUBLIC

node_t MemtreeMount::getRootNode() {
	return this->rootNode;
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
	if (to > node->size) {
		this->setNodeSize(node, to);
	}
	if (from > to) {
		return 0;
	}

	block_t blockN, realBlock, blockOffset;
	for (size_t i = from; i < to; i++) {
		blockN = static_cast<block_t>(i / this->sb->blockSize);
		blockOffset = static_cast<block_t>(i % this->sb->blockSize);

		// possible optimalization: use memcpy
		realBlock = node->directBlocks[blockN];
		if (realBlock == invalidBlock) {
			to = i;
			break;
		}

		this->data[realBlock * this->sb->blockSize + blockOffset] = src[i - from];
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

size_t MemtreeMount::readDirNode(kiv_os_vfs::Inode *node, kiv_os::TDir_Entry *dst, uint16_t nFrom, uint16_t nTo) {

	MemtreeDirEntry dentry;
	size_t totalRead = 0;
	for (uint16_t i = nFrom; i < nTo; i++) {
		size_t read = this->readNode(node, (uint8_t *)&dentry, i * this->dirEntrySize, (i + 1) * this->dirEntrySize);
		if (read != this->dirEntrySize) {
			return totalRead;
		}
		memcpy(dst + (i - nFrom), &dentry.apiEntry, sizeof(kiv_os::TDir_Entry));
		totalRead += sizeof(kiv_os::TDir_Entry);
	}

	return totalRead;
}

size_t MemtreeMount::readDir(node_t n, kiv_os::TDir_Entry *dst, uint16_t nFrom, uint16_t nTo) {
	kiv_os_vfs::Inode *node = this->getNode(n);
	if (!isDirectoryNode(node)) {
		return 0;
	}

	return this->readDirNode(node, dst, nFrom, nTo);
}

bool MemtreeMount::setNodeSize(kiv_os_vfs::Inode *node, size_t size) {
	if (size > this->maxFilesize) {
		return false;
	}

	node->size = size;

	uint16_t requiredBlocks = (uint16_t)(size / this->sb->blockSize);
	if (size % this->sb->blockSize) {
		requiredBlocks++;
	}

	for (uint16_t i = 0; i < kiv_os_vfs::inode_directLinks; i++) {
		// example: 2 blocks are required: ensure 0, 1 are assigned AND release 2...directBlocks
		if (i < requiredBlocks) {
			if (node->directBlocks[i] == invalidBlock) {
				if (this->reserveBlock(node, i) == invalidBlock) {
					return false;
				}
			}
		}
		else {
			this->releaseBlock(node, i);
		}

	}

	return true;
}

bool MemtreeMount::setSize(node_t n, size_t size) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return false;
	}
	return this->setNodeSize(node, size);
}
size_t MemtreeMount::getSize(node_t n) {
	kiv_os_vfs::Inode *node = getNode(n);
	if (node == nullptr) {
		return 0;
	}
	return node->size;
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

node_t MemtreeMount::createFile(node_t directory, const char* name, uint16_t attrs) {
	kiv_os_vfs::Inode *parentNode = getNode(directory);
	if (!isDirectoryNode(parentNode)) {
		return kiv_os_vfs::invalidNode;
	}
	if (parentNode->size + this->dirEntrySize >= this->maxFilesize) {
		// folder is full
		return kiv_os_vfs::invalidNode;
	}
	if (findInDirectory(directory, name) != kiv_os_vfs::invalidNode) {
		// file with this name already exists
		return kiv_os_vfs::invalidNode;
	}


	node_t newFile = reserveFreeNode(attrs);
	if (newFile == kiv_os_vfs::invalidNode) {
		return kiv_os_vfs::invalidNode;
	}

	MemtreeDirEntry newFileDentry;
	strcpy_s(newFileDentry.apiEntry.file_name, 12, name); // todo: 8 + 1 + 3 in api.h TDir_Entry
	newFileDentry.apiEntry.file_attributes = attrs;
	newFileDentry.node = newFile;

	writeNode(parentNode, (uint8_t *)(&newFileDentry), parentNode->size, parentNode->size + this->dirEntrySize);

	return newFile;
}

bool MemtreeMount::deleteFile(node_t parentDirectory, node_t f) {
	kiv_os_vfs::Inode *dir = this->getNode(parentDirectory);
	kiv_os_vfs::Inode *file = this->getNode(f);

	if (!isValidNode(file)) {
		return false;
	}

	if (isDirectoryNode(file)) {
		if (!this->deleteDirectory(file)) {
			return false;
		}
	}
	this->releaseNode(f);

	MemtreeDirEntry dirEntry;
	size_t readEntries = 0, readBytes;
	bool deleted = false;

	while (readEntries * this->dirEntrySize < dir->size)
	{
		readBytes = this->readNode(dir, (uint8_t *)(&dirEntry), readEntries * this->dirEntrySize, (readEntries + 1) * this->dirEntrySize);
		if (readBytes != this->dirEntrySize) {
			break;
		}

		if (deleted) {
			this->writeNode(dir, (uint8_t *)(&dirEntry), (readEntries - 1) * this->dirEntrySize, readEntries * this->dirEntrySize);
		}
		else if (dirEntry.node == f) {
			deleted = true;
		}

		readEntries++;
	}
	if (deleted) {
		this->setSize(parentDirectory, (readEntries - 1) * this->dirEntrySize);
	}

	return true;
}