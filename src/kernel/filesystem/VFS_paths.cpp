#include <string>

#include "VFS_paths.h"
#include "VFS.h"

#include "../process/process_api.h"

namespace vfs_paths {
	char labelSeparator[] = { kiv_os_vfs::mountSeparator, kiv_os_vfs::pathSeparator };

	int immediatePathPart(char *src, char** immediate, char**rest) {
		*immediate = strtok_s(src, &kiv_os_vfs::pathSeparator, rest);
		if (*rest != nullptr) {
			if (std::strlen(*rest) == 0) {
				*rest = nullptr;
			}
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

	size_t removeDots(char *path, const uint16_t labelLength, const uint16_t pathLength) {
		size_t srcPos = labelLength + 2;
		size_t dstPos = srcPos;

		uint16_t depth = 0;

		const char *sepPos;
		while ((sepPos = strchr(path + srcPos, kiv_os_vfs::pathSeparator)) != nullptr) {
			size_t sepPosN = sepPos - path;
			size_t pathPartLength = sepPosN - srcPos;

			if (pathPartLength == 1 && path[srcPos] == '.') {
				// same directory - do nothing
				srcPos += 2;
			}
			else if (pathPartLength != 2 || path[srcPos] != '.' || path[srcPos + 1] != '.') {
				// new directory - append it
				memcpy_s(path + dstPos, pathLength - dstPos, path + srcPos, pathPartLength);
				path[dstPos + pathPartLength] = kiv_os_vfs::pathSeparator;

				dstPos += pathPartLength + 1;
				srcPos += pathPartLength + 1;
				depth++;
			}
			else {
				// directory up - go back
				if (depth == 0) {
					return 0;
				}

				srcPos += pathPartLength + 1;
				// revert dstPos to one position after last pathSeparator
				for (dstPos--; dstPos > 1 && path[dstPos - 1] != kiv_os_vfs::pathSeparator; dstPos--);

				depth--;
			}
		}

		strcpy_s(path + dstPos, pathLength, path + srcPos);


		return dstPos + pathLength - srcPos;
	}

	size_t normalizePath(char *dst, const char *src, size_t maxDstLength) {
		uint16_t labelLength;

		if (isAbsolute(src, &labelLength)) {
			strcpy_s(dst, maxDstLength, src);
		}
		else {
			// a relative path - use current work dir as a prefix
			std::string wd = process::getWorkingDir();
			const char *wdp = wd.c_str();
			strcpy_s(dst, maxDstLength, wdp);

			strcpy_s(dst + wd.length(), maxDstLength - wd.length(), src);

			// assumes workdir always has system label
			labelLength = (uint16_t)(strstr(dst, labelSeparator) - dst);
		}

		uint16_t pathLength = (uint16_t)strnlen_s(dst, maxDstLength);

		for (uint16_t i = 0; i < pathLength; i++) {
			if (dst[i] == '\\') {
				dst[i] = kiv_os_vfs::pathSeparator;
			}
		}


		return removeDots(dst, labelLength, pathLength);
	}

	bool isAbsolute(const char *path, uint16_t *labelLength) {
		const char *mountSeparatorPos = strstr(path, labelSeparator);
		if (mountSeparatorPos == nullptr) {
			return false;
		}
		if (labelLength != nullptr) {
			*labelLength = (uint16_t)(mountSeparatorPos - path);
		}

		return  mountSeparatorPos - path < kiv_os_vfs::mountpointLabelSize;
	}
}