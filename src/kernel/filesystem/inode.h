#pragma once

#include <cstdint>

#undef stdin
#undef stderr
#undef stdout
#include "../../api/api.h"

typedef size_t node_t;
typedef uint32_t block_t;

namespace kiv_os_vfs {
	const uint8_t inode_directLinks = 12;

	struct Inode {
		uint16_t mode;
		uint16_t refCount;
		uint16_t owner, group; // unused
		size_t size;
		uint32_t atime, mtime, ctime; // unused

		block_t directBlocks[inode_directLinks];
		node_t singleIndirect; // not implemented
		node_t doubleIndirect; // not implemented
		node_t tripleIndirect; // not implemented
	};
}


bool isValidNode(kiv_os_vfs::Inode *test);

bool isDirectoryNode(kiv_os_vfs::Inode *test);