#pragma once

#include "VFS.h"

#include <cstdint>

namespace fs_mem_tree {

	int registerDriver();
	
	int mountDrive(char *label, size_t inodes, size_t blocks, size_t blockSize);
}
