/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/level/level_info.hpp"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/asset_types/world.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include <pragma/math/intersection.h>
#include <pragma/util/resource_watcher.h>
#include <sharedutils/util_file.h>
#include <unordered_set>

extern DLLNETWORK Engine *engine;

bool util::port_source2_map(NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(Game &, const std::string &)>(impl::get_module_func(nw, "convert_source2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = engine->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}

bool util::port_hl2_map(NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(Game &, const std::string &)>(impl::get_module_func(nw, "convert_hl2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = engine->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}
