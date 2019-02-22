#ifndef __WDF_H__
#define __WDF_H__
#include "pragma/networkdefinitions.h"
#include <string>
#include <fsys/filesystem.h>
class DLLNETWORK FWDF
{
private:
protected:
	template<class T>
		T Read();
	void Read(void *ptr,size_t size);
	std::string ReadString();
	VFilePtr m_file;
public:
};

template<class T>
	inline T FWDF::Read()
{
	return m_file->Read<T>();
}

#endif