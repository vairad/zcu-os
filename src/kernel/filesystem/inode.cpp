#include "inode.h"

bool isValidNode(kiv_os_vfs::Inode *test) {
	if (test == nullptr) {
		return false;
	}
	if (test->refCount == 0) {
		return false;
	}

	return true;
}

bool isDirectoryNode(kiv_os_vfs::Inode *test) {
	if (!isValidNode(test)) {
		return false;
	}

	return ((test->mode & kiv_os::faDirectory) != 0);
}