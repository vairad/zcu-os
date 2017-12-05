#pragma once

#include <string>

#include "shell_executor.h"

#undef stdin
#undef stderr
#undef stdout

namespace shell_cd {

	bool cd(shell_executor::CommandExecute command, std::string args);

}