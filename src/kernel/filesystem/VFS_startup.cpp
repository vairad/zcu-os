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
		size_t read;

		kiv_os_vfs::openFile("var/././../etc/./karel.txt", 0, 0);

		createFolder("C:/comics");
		createFolder("C:/comics/marvel");
		createFolder("C:/del-me/");


		auto fd = kiv_os_vfs::openFile("C:/comics/marvel/spdrmn.txt", 0, 0);

		kiv_os_vfs::write(fd, "2017-11-i23 Block A\n", 20);
		kiv_os_vfs::write(fd, "2017-11-i24 Block B\n", 20);
		kiv_os_vfs::write(fd, "2017-11-i27 Not available\n", 26);
		kiv_os_vfs::write(fd, "2017-11-i28 Block A\n", 20);

		fd = kiv_os_vfs::openFile("C:/comics/marvel/dedpl.txt", 0, 0);
		kiv_os_vfs::write(fd, "B****\nPanc*kes\nHi Francis!", 26);
		kiv_os_vfs::close(fd);

		fd = kiv_os_vfs::openFile("C:/comics/marvel/test1.txt", 0, 0);
		kiv_os_vfs::close(fd);

		kiv_os::TDir_Entry dentry[17];
		fd = kiv_os_vfs::openFile("C:/comics/marvel/", kiv_os::fmOpen_Always, kiv_os::faDirectory);
		read = kiv_os_vfs::read(fd, (void *)&dentry, sizeof(kiv_os::TDir_Entry) * 17);
		


		fd = kiv_os_vfs::openFile("C:/del-me/ahoj.txt", 0, 0);
		kiv_os_vfs::write(fd, "aibohphobia", 11);
		kiv_os_vfs::close(fd);

		fd = kiv_os_vfs::openFile("C:/del-me/ahoj.txt", 0, 0);
		kiv_os_vfs::write(fd, "Jelenovi pivo nelej", 19);
		kiv_os_vfs::close(fd);

		fd = kiv_os_vfs::openFile("C:/del-me/ahoj.txt", kiv_os::fmOpen_Always, 0);
		kiv_os_vfs::setPos(fd, 0, kiv_os::fsEnd, 0);

		kiv_os_vfs::write(fd, ". Ups, jeste ta tecka.", 22);
		kiv_os_vfs::close(fd);

		char bfr[255] = {0};
		fd = kiv_os_vfs::openFile("C:/del-me/ahoj.txt", kiv_os::fmOpen_Always, 0);
		read = kiv_os_vfs::read(fd, bfr, 255);

		kiv_os_vfs::setPos(fd, 0, kiv_os::fsBeginning, 0);
		read += kiv_os_vfs::read(fd, bfr + read, 255 - read);
		kiv_os_vfs::close(fd);

		// this should be invalid handle
		fd = kiv_os_vfs::openFile("C:/del-me/nada.txt", kiv_os::fmOpen_Always, 0);

		kiv_os_vfs::delFile("C:/del-me");

		// now this should be invalid handle too
		fd = kiv_os_vfs::openFile("C:/del-me/ahoj.txt", kiv_os::fmOpen_Always, 0);


	}
}