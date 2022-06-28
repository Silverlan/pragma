/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/liquid/c_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_simulation_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_static_bvh_cache_component.hpp"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
// --template-include-location

void RegisterLuaEntityComponents2(lua_State *l,luabind::module_ &entsMod)
{
	auto defLiquidControl = pragma::lua::create_entity_component_class<pragma::CLiquidControlComponent,pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::lua::create_entity_component_class<pragma::CLiquidSurfaceSimulationComponent,pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	auto defBvhHitInfo = luabind::class_<pragma::BvhHitInfo>("HitInfo");
	defBvhHitInfo.def_readonly("mesh",&pragma::BvhHitInfo::mesh);
	defBvhHitInfo.def_readonly("entity",&pragma::BvhHitInfo::entity);
	defBvhHitInfo.def_readonly("primitiveIndex",&pragma::BvhHitInfo::primitiveIndex);
	defBvhHitInfo.def_readonly("distance",&pragma::BvhHitInfo::distance);
	defBvhHitInfo.def_readonly("t",&pragma::BvhHitInfo::t);
	defBvhHitInfo.def_readonly("u",&pragma::BvhHitInfo::u);
	defBvhHitInfo.def_readonly("v",&pragma::BvhHitInfo::v);

	auto defBvh = pragma::lua::create_entity_component_class<pragma::CBvhComponent,pragma::BaseEntityComponent>("BvhComponent");
	defBvh.def("IntersectionTest",static_cast<std::optional<pragma::BvhHitInfo>(pragma::CBvhComponent::*)(const Vector3&,const Vector3&,float,float) const>(&pragma::CBvhComponent::IntersectionTest));
	defBvh.scope[defBvhHitInfo];
	entsMod[defBvh];

	auto defStaticBvh = pragma::lua::create_entity_component_class<pragma::CStaticBvhCacheComponent,pragma::BaseEntityComponent>("StaticBvhCacheComponent");
	defStaticBvh.def("IntersectionTest",static_cast<std::optional<pragma::BvhHitInfo>(pragma::CStaticBvhCacheComponent::*)(const Vector3&,const Vector3&,float,float) const>(&pragma::CStaticBvhCacheComponent::IntersectionTest));
	defStaticBvh.def("TestRebuildBvh",&pragma::CStaticBvhCacheComponent::TestRebuildBvh);
	defStaticBvh.def("TestPopulate",&pragma::CStaticBvhCacheComponent::TestPopulate);

	defStaticBvh.def("SetEntityDirty",&pragma::CStaticBvhCacheComponent::SetEntityDirty);
	defStaticBvh.def("AddEntity",&pragma::CStaticBvhCacheComponent::AddEntity);
	defStaticBvh.def("RemoveEntity",+[](pragma::CStaticBvhCacheComponent &component,BaseEntity &ent) {component.RemoveEntity(ent);});
	entsMod[defStaticBvh];

	auto defStaticBvhUser = pragma::lua::create_entity_component_class<pragma::CStaticBvhUserComponent,pragma::BaseEntityComponent>("StaticBvhUserComponent");
	entsMod[defStaticBvhUser];
	// --template-component-register-location
}
