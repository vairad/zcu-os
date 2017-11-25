#include "VFS.h"
#include "fs_mem_tree.h"

namespace fs_mem_tree {

	kiv_os_vfs::filesys_id _fsid;
	kiv_os_vfs::FsDriver _driver;

	int readBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length) {
		return 1;
	}
	int writeBytes(kiv_os_vfs::FileDescriptor *fd, uint8_t *byte, size_t length) {
		return 1;
	}

	int registerDriver() {
		_driver.read = readBytes;
		_driver.write = &writeBytes;
		kiv_os_vfs::registerDriver(&_driver, &_fsid);
		return 0;
	}
}