#ifndef __FBX_H__
#define __FBX_H__

#include <string>

namespace import
{
	bool load_fbx(NetworkState *nw,Model &mdl,VFilePtr &f,std::vector<std::string> &textures);
};

#endif
