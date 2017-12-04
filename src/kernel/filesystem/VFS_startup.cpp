#include "VFS_startup.h"

#include "fs_stdio.h"
#include "fs_pipe.h"
#include "proc_fs.h"
#include "fs_mem_tree.h"

#undef stdin
#undef stderr
#undef stdout

namespace kiv_os_vfs {

	bool startUp() {
		int initResult = init(42, 255, fs_pipe::createPipe);
		if (initResult != 0) {
			return false;
		}

		// stdio filesystemm needs to be first - if mount label (C:/ or mydrive:/) is not found in path, first superblock is used
		fs_stdio::registerAndMount();

		fs_pipe::registerAndMount();

		fs_process::registerAndMount();

		fs_mem_tree::registerDriver();

		// hardcoded fstab
		fs_mem_tree::mountDrive("C", 256, 1024, 1024);

		return true;
	}
}