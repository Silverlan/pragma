#ifndef __WV_SOURCE_HPP__
#define __WV_SOURCE_HPP__

#include <inttypes.h>
#include <string>

class NetworkState;
class Model;
struct TextureGroup;
namespace import
{
	namespace util
	{
		uint32_t add_texture(NetworkState &nw,Model &mdl,const std::string &name,TextureGroup *optTexGroup=nullptr,bool forceAddToTexGroup=false);
		void port_model_texture_assets(NetworkState &nw,Model &mdl);
	};
};

#endif
