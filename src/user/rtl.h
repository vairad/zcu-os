#pragma once

#include "..\api\api.h"

namespace kiv_os_rtl {

	size_t Get_Last_Error();

	kiv_os::THandle Create_File(const char* file_name, size_t flags);
	//podle flags otevre, vytvori soubor a vrati jeho deskriptor
	//vraci nenulovy handle, kdyz vse OK
	bool Write_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	bool Close_File(const kiv_os::THandle file_handle);
	//uzavre soubor identifikovany pomoci deskriptoru
	//vraci true, kdyz vse OK

}