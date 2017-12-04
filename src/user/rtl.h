#pragma once


#undef stdin
#undef stderr
#undef stdout
#include "..\api\api.h"

namespace kiv_os_rtl {

	size_t Get_Last_Error();

	
	/**
	 * \brief Depend on parameters try to create an OPEN file.
	 * \param file_name path to file 
	 * \param flags file atribute
	 * \param handle field for returned handle of opened file
	 * \return success flag
	 */
	bool Create_File(const char* file_name, size_t flags, uint16_t attrs, kiv_os::THandle& handle);


	bool Write_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	bool Read_File(const kiv_os::THandle file_handle, const void *buffer, const size_t buffer_size, size_t &read);
	//cte ze souboru identifikovaneho deskriptor data do buffer o velikosti buffer_size a vrati pocet prectenych dat v read
	//vraci true, kdyz vse OK
	bool Close_File(const kiv_os::THandle file_handle);
	//uzavre soubor identifikovany pomoci deskriptoru
	//vraci true, kdyz vse OK

	bool Create_Process(kiv_os::THandle *returned, const char* program, const char* args,
		const kiv_os::THandle in_stream = kiv_os::erInvalid_Handle,
		const kiv_os::THandle out_stream = kiv_os::erInvalid_Handle,
		const kiv_os::THandle err_stream = kiv_os::erInvalid_Handle);
	bool Create_Thread(kiv_os::THandle *returned, const void *program, const void *data);

	bool Join_One_Handle(kiv_os::THandle wait_for);
	bool Create_Pipe(kiv_os::THandle handles[]);
	bool Get_Working_Dir(const void *wd, const size_t wd_size, size_t &read);
	bool Change_Working_Dir(const void *path);
	bool Delete_File(const void *file);

	bool Get_File_Attributes(kiv_os::THandle handle, uint16_t &attrs);
	bool Shutdown();
}
