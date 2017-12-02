#pragma once

#include "../../api/api.h"
#include "../filesystem/VFS.h"

struct MemtreeDirEntry {
	kiv_os::TDir_Entry apiEntry;

	node_t node;
};

class MemtreeMount
{
	kiv_os_vfs::Superblock *sb;

	kiv_os_vfs::Inode *inodes;
	uint8_t *data;


	size_t maxFilesize;
	size_t dirEntrySize;

	kiv_os_vfs::Inode *getNode(node_t n);

	size_t readNode(kiv_os_vfs::Inode *node, uint8_t *dst, size_t from, size_t to);
	size_t writeNode(kiv_os_vfs::Inode *node, uint8_t *src, size_t from, size_t to);

public:
	MemtreeMount(kiv_os_vfs::Superblock *superblock);
	~MemtreeMount();

	size_t read(node_t node, uint8_t *dst, size_t from, size_t to);
	size_t write(node_t node, uint8_t *src, size_t from, size_t to);

	bool setSize(node_t node, size_t size);
	size_t getSize(node_t node);

	bool isDirectory(node_t node);
	node_t findInDirectory(node_t folder, const char* member);
};

