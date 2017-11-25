#pragma once

#include "VFS.h"

#include <cstdint>

namespace fs_mem_tree {
	
	int registerDriver();

	int read(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length);
	int write(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length);
}
