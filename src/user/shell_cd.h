#pragma once

#include "shell_executor.h"
#include <string>

#undef stdin
#undef stderr
#undef stdout

namespace kiv_os {

	bool cd(kiv_os::CommandExecute command, std::string args);

}