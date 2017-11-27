#pragma once

#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

void HandleIO(kiv_os::TRegisters &regs);