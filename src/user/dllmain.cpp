/*

V tomto projektu se nesmi pouzivat hlavickove soubory Windows, protoze
vse nam zajisti nas simulovany OS.

#include <Windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 ) {
	switch (ul_reason_for_call)	{
	case DLL_PROCESS_ATTACH: 
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:break;
	}
	return TRUE;
}


*/

int __stdcall DllMain() {
	return 1;
}