#pragma once
#include "VFS.h"

#undef stdin
#undef stderr
#undef stdout

namespace fs_pipe {
	int registerAndMount();

	int createPipe(kiv_os_vfs::FileDescriptor *fd_in, kiv_os_vfs::FileDescriptor *fd_out);
}