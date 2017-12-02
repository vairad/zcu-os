#include <cstring>

#include "VFS_paths.h"

namespace vfs_paths {
	int immediatePathPart(char *src, char** immediate, char**rest) {
		*immediate = strtok_s(src, &kiv_os_vfs::pathSeparator, rest);
		if (std::strlen(*rest) == 0) {
			*rest = nullptr;
		}
		return 0;
	}

	int separeLastPart(char *src, char**last) {
		char *pos = strrchr(src, kiv_os_vfs::pathSeparator);

		if (pos == nullptr) {
			return 1;
		}

		*pos = '\0';
		*last = pos + 1;

		return 0;
	}
}