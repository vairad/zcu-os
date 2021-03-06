#pragma once

#include "../filesystem/VFS.h"

#undef stdin
#undef stderr
#undef stdout

namespace fs_mem_tree {

	int registerDriver();
	
	int mountDrive(char *label, node_t inodes, block_t blocks, size_t blockSize);

	int cleanUp();
}
