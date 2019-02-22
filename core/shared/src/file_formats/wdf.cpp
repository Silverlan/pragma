#include "stdafx_shared.h"
#include "pragma/file_formats/wdf.h"

void FWDF::Read(void *ptr,size_t size)
{
	m_file->Read(ptr,size);
}

std::string FWDF::ReadString() {return m_file->ReadString();}