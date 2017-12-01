#pragma once
#include "VFS.h"
namespace fs_pipe {
	int registerAndMount();

	int createPipe(kiv_os_vfs::FileDescriptor *fd_in, kiv_os_vfs::FileDescriptor *fd_out);
}