#pragma once

#include <Windows.h>

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

kiv_os::THandle Convert_Native_Handle(const HANDLE hnd);
HANDLE Resolve_kiv_os_Handle(const kiv_os::THandle hnd);
bool Remove_Handle(const kiv_os::THandle hnd);
