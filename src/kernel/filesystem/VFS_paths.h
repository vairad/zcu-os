#pragma once

namespace vfs_paths {
	int immediatePathPart(char *src, char** immediate, char**rest);

	int separeLastPart(char *src, char**last);

	size_t normalizePath(char *dst, const char *src, size_t maxDstLength);

	bool isAbsolute(const char *path, uint16_t *labelLength);
}
