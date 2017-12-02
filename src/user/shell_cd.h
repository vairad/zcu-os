#pragma once

#include "shell_executor.h"
#include <string>

#undef stdin
#undef stderr
#undef stdout

namespace shell_cd {

	bool cd(shell_executor::CommandExecute command, std::string args);

}