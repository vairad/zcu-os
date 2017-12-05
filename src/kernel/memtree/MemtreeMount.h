#pragma once

#include "../filesystem/VFS.h"
#include "../Bitmap.h"

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"

struct MemtreeDirEntry {
	kiv_os::TDir_Entry apiEntry;

	node_t node;
};

class MemtreeMount
{
	static const block_t invalidBlock = -1;

	kiv_os_vfs::Superblock *sb;

	kiv_os_vfs::Inode *inodes;
	uint8_t *data;
	Bitmap *blockBitmap;

	node_t rootNode;


	size_t maxFilesize;
	size_t dirEntrySize;

	kiv_os_vfs::Inode *getNode(node_t n);

	uint16_t findNodeInDirectory(kiv_os_vfs::Inode *dir, node_t n);

	size_t readNode(kiv_os_vfs::Inode *node, uint8_t *dst, size_t from, size_t to);
	size_t writeNode(kiv_os_vfs::Inode *node, uint8_t *src, size_t from, size_t to);
	size_t readDirNode(kiv_os_vfs::Inode *node, kiv_os::TDir_Entry *dst, uint16_t nFrom, uint16_t nTo);

	bool setNodeSize(kiv_os_vfs::Inode *node, size_t size);

	node_t reserveFreeNode(uint16_t attrs);
	void releaseNode(node_t n);

	block_t reserveBlock(kiv_os_vfs::Inode *node, block_t nodeBlock);
	void releaseBlock(kiv_os_vfs::Inode *node, block_t nodeBlock);

	bool deleteDirectory(kiv_os_vfs::Inode *dir);

public:
	MemtreeMount(kiv_os_vfs::Superblock *superblock);
	~MemtreeMount();

	node_t getRootNode();

	size_t read(node_t node, uint8_t *dst, size_t from, size_t to);
	size_t write(node_t node, uint8_t *src, size_t from, size_t to);
	size_t readDir(node_t node, kiv_os::TDir_Entry *dst, uint16_t nFrom, uint16_t nTo);
	
	bool setSize(node_t node, size_t size);
	size_t getSize(node_t node);

	bool isDirectory(node_t node);
	node_t findInDirectory(node_t folder, const char* member);

	node_t createFile(node_t directory, const char* name, uint16_t attrs);

	bool deleteFile(node_t directory, node_t file);
};

