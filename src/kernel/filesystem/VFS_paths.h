#pragma once

#include "VFS.h"

namespace vfs_paths {
	int immediatePathPart(char *src, char** immediate, char**rest);

	int separeLastPart(char *src, char**last);
}
