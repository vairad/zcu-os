#include "VFS_startup.h"

#include "fs_stdio.h"
#include "fs_mem_tree.h"

#undef stdin
#undef stderr
#undef stdout

namespace kiv_os_vfs {

	int startUp() {
		int initResult = init(42, 255);
		if (initResult != 0) {
			return initResult;
		}

		fs_stdio::registerDriver();

		fs_mem_tree::registerDriver();

		// hardcoded fstab
		fs_mem_tree::mountDrive("C", 256, 1024, 1024);

		return 0;
	}
}