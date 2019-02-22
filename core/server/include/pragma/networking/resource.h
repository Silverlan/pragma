#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "pragma/serverdefinitions.h"
#include <string>
#include <memory>

class VFilePtrInternal;
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLSERVER Resource
{
	Resource(std::string name,bool bStream=true);
	~Resource();
	bool Construct();
	std::string name;
	unsigned int offset;
	std::shared_ptr<VFilePtrInternal> file;
	bool stream;
};
#pragma warning(pop)

#endif