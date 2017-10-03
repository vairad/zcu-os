#include <Windows.h>
#include <iostream>

using TRun_VM = void(__stdcall *)();

int __cdecl main() {
	HMODULE kernel = LoadLibrary(L"kernel.dll");
	if (!kernel) {
		std::cout << "Nelze nacist kernel.dll!" << std::endl;
		return 1;
	}
	TRun_VM vm = reinterpret_cast<TRun_VM>(GetProcAddress(kernel, "Run_VM"));
	if (vm) vm();
	  else {
		std::cout << "Nelze ziskat adresu Run_VM v kernel.dll!" << std::endl;
		return 2;
	
	}
	FreeLibrary(kernel);
	return 0;
}