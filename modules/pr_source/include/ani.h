#ifndef __ANI_H__
#define __ANI_H__

#include <string>
#include <vector>
#include <mathutil/uvec.h>
#include <pragma/model/vertex.h>
#include "mdl_animation.h"

#pragma pack(push,1)
namespace import
{
	struct MdlInfo;
	namespace mdl
	{
		bool load_ani(const VFilePtr &f,const MdlInfo &mdlInfo);
	};
};
#pragma pack(pop)

#endif
