#ifndef __UTIL_H__
#define __UTIL_H__

#include <inttypes.h>

class NetworkState;
class Model;
namespace import
{
	namespace util
	{
		uint32_t add_texture(Model &mdl,const std::string &name);
	};
};

#endif
