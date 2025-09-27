// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lai.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include <luainterface.hpp>

module pragma.client;

import :scripting.lua.libraries.ai;

void Lua::ai::register_library(Lua::Interface &lua)
{
	auto &modAi = lua.RegisterLibrary("ai");
	auto classDefMoveInfo = luabind::class_<pragma::BaseAIComponent::MoveInfo>("MoveInfo");
	classDefMoveInfo.def(luabind::constructor<>());
	classDefMoveInfo.def_readwrite("activity", reinterpret_cast<std::underlying_type_t<decltype(pragma::BaseAIComponent::MoveInfo::activity)> pragma::BaseAIComponent::MoveInfo::*>(&pragma::BaseAIComponent::MoveInfo::activity));
	classDefMoveInfo.def_readwrite("faceTarget", &pragma::BaseAIComponent::MoveInfo::faceTarget);
	classDefMoveInfo.def_readwrite("moveOnPath", &pragma::BaseAIComponent::MoveInfo::moveOnPath);
	classDefMoveInfo.def_readwrite("moveSpeed", &pragma::BaseAIComponent::MoveInfo::moveSpeed);
	classDefMoveInfo.def_readwrite("turnSpeed", &pragma::BaseAIComponent::MoveInfo::turnSpeed);
	classDefMoveInfo.def_readwrite("destinationTolerance", &pragma::BaseAIComponent::MoveInfo::destinationTolerance);
	modAi[classDefMoveInfo];
}
