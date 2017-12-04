#pragma once


#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

extern "C" size_t __stdcall shell(const kiv_os::TRegisters &regs);


//nasledujici funkce si dejte do vlastnich souboru
extern "C" size_t __stdcall dir(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall type(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall md(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall rd(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall echo(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall wc(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall sort(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall rgen(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall freq(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall ps(const kiv_os::TRegisters &regs);
extern "C" size_t __stdcall shutdown(const kiv_os::TRegisters &regs);
