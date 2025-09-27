// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

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

export module pragma.shared;

import :assets.asset_conversion;

bool util::port_source2_map(NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(Game &, const std::string &)>(impl::get_module_func(nw, "convert_source2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}

bool util::port_hl2_map(NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(Game &, const std::string &)>(impl::get_module_func(nw, "convert_hl2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}
