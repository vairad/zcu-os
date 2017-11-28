#pragma once
#include "VFS.h"
namespace fs_pipe {
	int registerDriver();

	int createPipe(kiv_os_vfs::FileDescriptor *fd_in, kiv_os_vfs::FileDescriptor *fd_out);
}