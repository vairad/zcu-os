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

	size_t ensureAbsoluteForm(char *dst, const char *src, size_t maxDstLength, uint16_t *labelLength) {
		if (isAbsolute(src, labelLength)) {
			strcpy_s(dst, maxDstLength, src);
		}
		else {
			// a relative path - use current work dir as a prefix
			std::string wd = process::getWorkingDir();
			const char *wdp = wd.c_str();
			size_t wdLength = wd.length();

			strcpy_s(dst, maxDstLength, wdp);
			if (dst[wdLength - 1] != kiv_os_vfs::pathSeparator) {
				dst[wdLength] = kiv_os_vfs::pathSeparator;
				wdLength++;
			}
			// assumes workdir always has system label
			*labelLength = (uint16_t)(strstr(dst, labelSeparator) - dst);
			
			if (strnlen_s(src, maxDstLength) == 1) {
				if (src[0] == kiv_os_vfs::pathSeparator || src[0] == '\\') {
					// we want to go to root
					dst[*labelLength + 2] = '\0';
					return *labelLength + 2;
				}
			}

			strcpy_s(dst + wdLength, maxDstLength - wdLength, src);
		}

		return (uint16_t)strnlen_s(dst, maxDstLength);
	}

	size_t removeDots(char *path, const uint16_t labelLength, const uint16_t pathLength) {
		size_t srcPos = labelLength + 2;
		size_t dstPos = srcPos;

		uint16_t depth = 0;

		const char *sepPos;
		while ((sepPos = strchr(path + srcPos, kiv_os_vfs::pathSeparator)) != nullptr) {
			size_t sepPosN = sepPos - path;
			size_t pathPartLength = sepPosN - srcPos;

			if (pathPartLength == 0 || pathPartLength == 1 && path[srcPos] == '.') {
				// duplicated path separator OR
				// same directory - do nothing
			}
			else if (pathPartLength != 2 || path[srcPos] != '.' || path[srcPos + 1] != '.') {
				// new directory - append it
				memcpy_s(path + dstPos, pathLength - dstPos, path + srcPos, pathPartLength);
				path[dstPos + pathPartLength] = kiv_os_vfs::pathSeparator;

				dstPos += pathPartLength + 1;
				
				depth++;
			}
			else {
				// directory up - go back
				
				if (depth != 0) {
					// revert dstPos to one position after last pathSeparator
					for (dstPos--; dstPos > 1 && path[dstPos - 1] != kiv_os_vfs::pathSeparator; dstPos--);
					depth--;
				}
			}

			// move src cursor after next separator
			srcPos += pathPartLength + 1;
		}

		size_t lastPartLength = pathLength - srcPos;
		if (lastPartLength == 1 && path[srcPos] == '.') {
			srcPos += 1; // skip dot
		}
		else if (lastPartLength == 2 && path[srcPos] == '.' && path[srcPos + 1] == '.') {
			if (depth != 0) {
				// revert dstPos to one position after last pathSeparator
				for (dstPos--; dstPos > 1 && path[dstPos - 1] != kiv_os_vfs::pathSeparator; dstPos--);
			}
			srcPos += 2; // skip
		}

		strcpy_s(path + dstPos, pathLength, path + srcPos);
		dstPos += pathLength - srcPos;

		return dstPos;
	}

	size_t normalizePath(char *dst, const char *src, size_t maxDstLength) {
		uint16_t labelLength;

		uint16_t pathLength = ensureAbsoluteForm(dst, src, maxDstLength, &labelLength);

		// unify possibly flipped path separators
		for (uint16_t i = 0; i < pathLength; i++) {
			if (dst[i] == '\\') {
				dst[i] = kiv_os_vfs::pathSeparator;
			}
		}

		pathLength = removeDots(dst, labelLength, pathLength);

		// remove trailing slash if it's not right after label
		if (pathLength > labelLength + 2 && dst[pathLength - 1] == kiv_os_vfs::pathSeparator) {
			dst[pathLength - 1] = '\0';
			pathLength--;
		}

		return pathLength;
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