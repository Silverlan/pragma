#ifndef __SMD_H__
#define __SMD_H__

#include <string>
#include <smdmodel.h>

class NetworkState;
class Model;
namespace import
{
	bool load_smd(NetworkState *nw,const std::string &name,Model &mdl,SMDModel &smd,bool bCollision,std::vector<std::string> &textures);
};

#endif
