#pragma once

#include "VFS.h"

#undef stdin
#undef stderr
#undef stdout

namespace fs_stdio {
	int registerAndMount();
}
