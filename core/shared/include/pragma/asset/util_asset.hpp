#ifndef __PRAGMA_UTIL_ASSET_HPP__
#define __PRAGMA_UTIL_ASSET_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma::asset
{
	enum class Type : uint8_t
	{
		Model = 0,
		Material,
		Texture,
		Sound
	};
	DLLNETWORK bool exists(NetworkState &nw,const std::string &name,Type type);
	DLLNETWORK std::optional<std::string> find_file(NetworkState &nw,const std::string &name,Type type);
	DLLNETWORK bool is_loaded(NetworkState &nw,const std::string &name,Type type);
};

#endif
