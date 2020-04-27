/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::Lightmap::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCLightMap = luabind::class_<CLightMapHandle,BaseEntityComponentHandle>("LightMapComponent");
	defCLightMap.def("GetLightmapTexture",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		auto lightMap = hLightMapC->GetLightMap();
		if(lightMap == nullptr)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,lightMap);
		}));
	defCLightMap.def("ConvertLightmapToBSPLuxelData",static_cast<void(*)(lua_State*,CLightMapHandle&)>([](lua_State *l,CLightMapHandle &hLightMapC) {
		pragma::Lua::check_component(l,hLightMapC);
		hLightMapC->ConvertLightmapToBSPLuxelData();
		}));
	entsMod[defCLightMap];

	auto defCLightMapReceiver = luabind::class_<CLightMapReceiverHandle,BaseEntityComponentHandle>("LightMapReceiverComponent");
	entsMod[defCLightMapReceiver];
}
