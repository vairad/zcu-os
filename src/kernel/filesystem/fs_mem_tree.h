#pragma once

#include "VFS.h"

#include <cstdint>

namespace fs_mem_tree {

	int registerDriver();

	int readBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length);
	int writeBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length);

	int mountDrive(char *label, size_t inodes, size_t blocks, size_t blockSize);
}
