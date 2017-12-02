#include "VFS_startup.h"

#include "fs_stdio.h"
#include "fs_pipe.h"
#include "proc_fs.h"
#include "../memtree/fs_mem_tree.h"

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
		fs_mem_tree::mountDrive("C", 256, 1024, 2048);

		return true;
	}


	// todo: remove this testing code, yah?

	void createFolder(char *absPath) {
		uint8_t folderAttrs = kiv_os::faDirectory;
		uint64_t openFlags = 0;

		kiv_os::THandle fd = kiv_os_vfs::openFile(absPath, openFlags, folderAttrs);
		kiv_os_vfs::close(fd);
	}

	void prefillDriveC() {
		createFolder("C:/comics");
		createFolder("C:/comics/marvel");

		auto fd = kiv_os_vfs::openFile("C:/comics/marvel/spdrmn.txt", 0, 0);

		kiv_os_vfs::write(fd, "2017-11-i23 Block A\n", 20);
		kiv_os_vfs::write(fd, "2017-11-i24 Block B\n", 20);
		kiv_os_vfs::write(fd, "2017-11-i27 Not available\n", 26);
		kiv_os_vfs::write(fd, "2017-11-i28 Block A\n", 20);

		kiv_os_vfs::close(fd);

		fd = kiv_os_vfs::openFile("C:/comics/marvel/dedpl.txt", 0, 0);
		kiv_os_vfs::write(fd, "B****\nPanc*kes\nHi Francis!", 26);
		kiv_os_vfs::close(fd);
	}
}