/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <prosper_command_buffer.hpp>

void Lua::Flex::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCFlex = pragma::lua::create_entity_component_class<pragma::CFlexComponent, pragma::BaseFlexComponent>("FlexComponent");
	defCFlex.def("SetFlexController",
	  static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float, float, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId, float value, float duration, bool clampToLimits) { hEnt.SetFlexController(flexId, value, duration, clampToLimits); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId, float value, float duration) { hEnt.SetFlexController(flexId, value, duration); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId, float value) { hEnt.SetFlexController(flexId, value); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, float, float, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &flexName, float value, float duration, bool clampToLimits) {
		hEnt.SetFlexController(flexName, value, duration, clampToLimits);
	}));
	defCFlex.def("SetFlexController",
	  static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, float, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &flexName, float value, float duration) { hEnt.SetFlexController(flexName, value, duration); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &flexName, float value) { hEnt.SetFlexController(flexName, value); }));
	defCFlex.def("GetFlexController", static_cast<std::optional<float> (*)(pragma::CFlexComponent &, uint32_t)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetFlexController", static_cast<std::optional<float> (*)(pragma::CFlexComponent &, const std::string &)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetScaledFlexController", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexControllerId) {
		float value;
		if(hEnt.GetScaledFlexController(flexControllerId, value) == false)
			return;
		Lua::PushNumber(l, value);
	}));
	defCFlex.def("SetFlexControllerScale", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, float scale) { hEnt.SetFlexControllerScale(scale); }));
	defCFlex.def("GetFlexControllerScale", static_cast<void (*)(lua_State *, pragma::CFlexComponent &)>([](lua_State *l, pragma::CFlexComponent &hEnt) { Lua::PushNumber(l, hEnt.GetFlexControllerScale()); }));
	defCFlex.def("CalcFlexValue", &Lua::Flex::CalcFlexValue);
	defCFlex.def("GetFlexWeights", static_cast<void (*)(lua_State *, pragma::CFlexComponent &)>([](lua_State *l, pragma::CFlexComponent &hEnt) {
		auto &flexWeights = hEnt.GetFlexWeights();
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(flexWeights.size()) {0u}; i < flexWeights.size(); ++i) {
			Lua::PushInt(l, i + 1);
			Lua::PushNumber(l, flexWeights.at(i));
			Lua::SetTableValue(l, t);
		}
	}));
	defCFlex.def("GetFlexWeight", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId) {
		float weight;
		if(hEnt.GetFlexWeight(flexId, weight) == false)
			return;
		Lua::PushNumber(l, weight);
	}));
	defCFlex.def("SetFlexWeight", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId, float weight) { hEnt.SetFlexWeight(flexId, weight); }));
	defCFlex.def("ClearFlexWeightOverride", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId) { hEnt.ClearFlexWeightOverride(flexId); }));
	defCFlex.def("HasFlexWeightOverride", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId) { Lua::PushBool(l, hEnt.HasFlexWeightOverride(flexId)); }));
	defCFlex.def("SetFlexWeightOverride", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t flexId, float weight) { hEnt.SetFlexWeightOverride(flexId, weight); }));
	defCFlex.def("GetFlexAnimations", static_cast<luabind::object (*)(lua_State *, pragma::CFlexComponent &)>([](lua_State *l, pragma::CFlexComponent &hEnt) -> luabind::object {
		auto &flexAnims = hEnt.GetFlexAnimations();
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &flexAnim : flexAnims)
			t[idx++] = flexAnim.flexAnimationId;
		return t;
	}));
	defCFlex.def("GetFlexAnimationCount", static_cast<uint32_t (*)(lua_State *, pragma::CFlexComponent &)>([](lua_State *l, pragma::CFlexComponent &hEnt) -> uint32_t { return hEnt.GetFlexAnimations().size(); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, bool, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id, bool loop, bool reset) { hEnt.PlayFlexAnimation(id, loop, reset); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id, bool loop) { hEnt.PlayFlexAnimation(id, loop); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id) { hEnt.PlayFlexAnimation(id); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, bool, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id, bool loop, bool reset) { hEnt.PlayFlexAnimation(id, loop, reset); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, bool)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id, bool loop) { hEnt.PlayFlexAnimation(id, loop); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id) { hEnt.PlayFlexAnimation(id); }));
	defCFlex.def("StopFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id) { hEnt.StopFlexAnimation(id); }));
	defCFlex.def("StopFlexAnimation", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id) { hEnt.StopFlexAnimation(id); }));
	defCFlex.def("GetFlexAnimationCycle", static_cast<float (*)(lua_State *, pragma::CFlexComponent &, uint32_t)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id) -> float { return hEnt.GetFlexAnimationCycle(id); }));
	defCFlex.def("GetFlexAnimationCycle", static_cast<float (*)(lua_State *, pragma::CFlexComponent &, const std::string &)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id) -> float { return hEnt.GetFlexAnimationCycle(id); }));
	defCFlex.def("SetFlexAnimationCycle", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id, float cycle) { hEnt.SetFlexAnimationCycle(id, cycle); }));
	defCFlex.def("SetFlexAnimationCycle", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id, float cycle) { hEnt.SetFlexAnimationCycle(id, cycle); }));
	defCFlex.def("SetFlexAnimationPlaybackRate", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, uint32_t, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, uint32_t id, float playbackRate) { hEnt.SetFlexAnimationPlaybackRate(id, playbackRate); }));
	defCFlex.def("SetFlexAnimationPlaybackRate", static_cast<void (*)(lua_State *, pragma::CFlexComponent &, const std::string &, float)>([](lua_State *l, pragma::CFlexComponent &hEnt, const std::string &id, float playbackRate) { hEnt.SetFlexAnimationPlaybackRate(id, playbackRate); }));
	defCFlex.add_static_constant("EVENT_ON_FLEX_CONTROLLERS_UPDATED", pragma::CFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED);
	entsMod[defCFlex];
}
