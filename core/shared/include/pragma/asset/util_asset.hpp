/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

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
