/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/lua/classes/lvector.h"
#include "pragma/lua/classes/langle.h"
#include "pragma/lua/libraries/lmatrix.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/classes/lconvar.h"
#include "pragma/lua/classes/lquaternion.h"
#include "pragma/lua/libraries/ltimer.h"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/types/nil_type.hpp"
#include <pragma/util/matrices.h>
#include "pragma/physics/physobj.h"
#include "pragma/lua/classes/lphysobj.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/classes/ldamageinfo.h"
#include "pragma/lua/lua_util_class.hpp"
#include "pragma/game/damageinfo.h"
#include "pragma/lua/classes/lplane.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/model/model.h"
#include "luasystem.h"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/lua/libraries/lnoise.h"
#include "pragma/lua/classes/lsurfacematerial.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/lua/libraries/lregex.h"
#include "pragma/lua/classes/ldata.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/libraries/lstring.hpp"
#include "pragma/util/giblet_create_info.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/math/util_pid_controller.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/lua/lua_entity_iterator.hpp"
#include "pragma/lua/libraries/lents.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/string_view_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/util/util_splash_damage_info.hpp"
#include "pragma/util/functional_parallel_worker.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/classes/lentity.h"
#include "pragma/entities/entity_property.hpp"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/classes/lproperty_generic.hpp"
#include "pragma/lua/classes/lproperty_entity.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include "pragma/lua/lua_entity_handles.hpp"
#include "pragma/lua/policies/handle_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/generic_policy.hpp"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/alias_converter_t.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include "pragma/lua/converters/thread_pool_converter_t.hpp"
#include "pragma/lua/classes/thread_pool.hpp"
#include "pragma/lua/classes/parallel_job.hpp"
#include <pragma/util/transform.h>
#include <sharedutils/datastream.h>
#include <sharedutils/util_path.hpp>
#include <util_image_buffer.hpp>
#include <mathutil/umath_lighting.hpp>
#include <luainterface.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <pragma/lua/policies/optional_policy.hpp>
#include <mathutil/inverse_kinematics/ik.hpp>
#include <mathutil/inverse_kinematics/constraints.hpp>
#include <sharedutils/magic_enum.hpp>
#include <fsys/directory_watcher.h>
#include <glm/gtx/matrix_decompose.hpp>

extern DLLNETWORK Engine *engine;

std::ostream &operator<<(std::ostream &out, const ALSound &snd)
{
	auto state = snd.GetState();
	out << "ALSound[" << snd.GetIndex() << "][";
	switch(state) {
	case ALState::Initial:
		out << "Initial";
		break;
	case ALState::Playing:
		out << "Playing";
		break;
	case ALState::Paused:
		out << "Paused";
		break;
	case ALState::Stopped:
		out << "Stopped";
		break;
	}
	out << "][";
	auto type = snd.GetType();
	auto values = umath::get_power_of_2_values(static_cast<uint64_t>(type));
	auto bStart = true;
	for(auto v : values) {
		if(bStart == false)
			out << " | ";
		else
			bStart = true;
		if(v == static_cast<uint64_t>(ALSoundType::Effect))
			out << "Effect";
		else if(v == static_cast<uint64_t>(ALSoundType::Music))
			out << "Music";
		else if(v == static_cast<uint64_t>(ALSoundType::Voice))
			out << "Voice";
		else if(v == static_cast<uint64_t>(ALSoundType::Weapon))
			out << "Weapon";
		else if(v == static_cast<uint64_t>(ALSoundType::NPC))
			out << "NPC";
		else if(v == static_cast<uint64_t>(ALSoundType::Player))
			out << "Player";
		else if(v == static_cast<uint64_t>(ALSoundType::Vehicle))
			out << "Vehicle";
		else if(v == static_cast<uint64_t>(ALSoundType::Physics))
			out << "Physics";
		else if(v == static_cast<uint64_t>(ALSoundType::Environment))
			out << "Environment";
	}
	out << "]";
	return out;
}

static void RegisterLuaMatrices(Lua::Interface &lua);
static void RegisterIk(Lua::Interface &lua);

static void create_directory_change_listener(lua_State *l, const std::string &path, luabind::object callback, DirectoryWatcherCallback::WatchFlags flags)
{
	Lua::CheckFunction(l, 2);
	try {
		auto listener = std::make_shared<DirectoryWatcherCallback>(
		  path, [callback](const std::string &fileName) mutable { callback(fileName); }, flags);
		Lua::Push(l, listener);
	}
	catch(const std::runtime_error &err) {
		Lua::PushBool(l, false);
		Lua::PushString(l, err.what());
		return;
	}
	Lua::PushBool(l, false);
	Lua::PushString(l, "Unknown error!");
}

static void register_directory_watcher(lua_State *l, luabind::module_ &modUtil)
{
	auto defListener = pragma::lua::register_class<"DirectoryChangeListener", DirectoryWatcherCallback>(l);
	defListener->add_static_constant("LISTENER_FLAG_NONE", umath::to_integral(DirectoryWatcherCallback::WatchFlags::None));
	defListener->add_static_constant("LISTENER_FLAG_BIT_WATCH_SUB_DIRECTORIES", umath::to_integral(DirectoryWatcherCallback::WatchFlags::WatchSubDirectories));
	defListener->add_static_constant("LISTENER_FLAG_ABSOLUTE_PATH", umath::to_integral(DirectoryWatcherCallback::WatchFlags::AbsolutePath));
	defListener->add_static_constant("LISTENER_FLAG_START_DISABLED", umath::to_integral(DirectoryWatcherCallback::WatchFlags::StartDisabled));
	defListener->add_static_constant("LISTENER_FLAG_WATCH_DIRECTORY_CHANGES", umath::to_integral(DirectoryWatcherCallback::WatchFlags::WatchDirectoryChanges));
	static_assert(magic_enum::flags::enum_count<DirectoryWatcherCallback::WatchFlags>() == 4);
	defListener
	  ->scope[luabind::def("create", static_cast<void (*)(lua_State *, const std::string &, luabind::object)>([](lua_State *l, const std::string &path, luabind::object callback) { create_directory_change_listener(l, path, callback, DirectoryWatcherCallback::WatchFlags::None); }))];
	defListener->scope[luabind::def("create", static_cast<void (*)(lua_State *, const std::string &, luabind::object, DirectoryWatcherCallback::WatchFlags)>([](lua_State *l, const std::string &path, luabind::object callback, DirectoryWatcherCallback::WatchFlags flags) {
		create_directory_change_listener(l, path, callback, flags);
	}))];
	defListener->def("Poll", static_cast<uint32_t (*)(lua_State *, DirectoryWatcherCallback &)>([](lua_State *l, DirectoryWatcherCallback &listener) { return listener.Poll(); }));
	defListener->def("SetEnabled", static_cast<void (*)(lua_State *, DirectoryWatcherCallback &, bool)>([](lua_State *l, DirectoryWatcherCallback &listener, bool enabled) { listener.SetEnabled(enabled); }));
	defListener->def("IsEnabled", static_cast<bool (*)(lua_State *, DirectoryWatcherCallback &)>([](lua_State *l, DirectoryWatcherCallback &listener) { return listener.IsEnabled(); }));
	modUtil[*defListener];
}

std::ostream &operator<<(std::ostream &out, const umath::Transform &t)
{
	auto &origin = t.GetOrigin();
	auto &rot = t.GetRotation();
	auto ang = EulerAngles {rot};
	out << "Transform[" << origin.x << "," << origin.y << "," << origin.z << "][" << ang.p << "," << ang.y << "," << ang.r << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const umath::ScaledTransform &t)
{
	auto &origin = t.GetOrigin();
	auto &rot = t.GetRotation();
	auto ang = EulerAngles {rot};
	auto &scale = t.GetScale();
	out << "ScaledTransform[" << origin.x << "," << origin.y << "," << origin.z << "][" << ang.p << "," << ang.y << "," << ang.r << "][" << scale.x << "," << scale.y << "," << scale.z << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const uimg::ImageLayerSet &layerSet)
{
	out << "ImageLayerSet[" << layerSet.images.size() << "]";
	return out;
}
template<typename T>
static void register_string_to_vector_type_constructor(lua_State *l)
{
	pragma::lua::define_custom_constructor<T,
	  [](const std::string &str) -> T {
		  T r;
		  using ValueType = typename decltype(r)::value_type;
		  ustring::string_to_array<ValueType, Double>(str, reinterpret_cast<ValueType *>(&r[0]), atof, decltype(r)::length());
		  return r;
	  },
	  const std::string &>(l);
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(util, BaseParallelJob);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(util, Path);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(util, Version);

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(uimg, ImageBuffer);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(uimg, ImageLayerSet);

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, Transform);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, ScaledTransform);

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector3i);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector4i);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector2);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector3);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector4);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Quat);

/* namespace panima
{
std::ostream &operator<<(std::ostream &out,const pragma::animation::Bone &o)
{
    return ::operator<<(out,o);
}
}; */

#undef DEFINE_OSTEAM_OPERATOR_NAMESPACE_ALIAS

namespace glm {
	std::ostream &operator<<(std::ostream &out, Vector3 &o) { return ::operator<<(out, o); }
	std::ostream &operator<<(std::ostream &out, Vector4 &o) { return ::operator<<(out, o); }

#define DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(type)                                                                                                                                                                                                                                            \
	std::ostream &operator<<(std::ostream &out, const Mat##type &o) { return ::operator<<(out, o); }

	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x2)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x3)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x4)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x2)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x3)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x4)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x2)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x3)
	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x4)

};
#endif

std::string pragma::lua::detail::tostring(const luabind::object &o)
{
	auto oToString = o["__tostring"];
	auto t = luabind::type(oToString);
	if(t != LUA_TFUNCTION)
		return "ERROR: No __tostring method!";
	auto oStr = luabind::call_member<luabind::object>(o, "__tostring");
	return luabind::object_cast_nothrow<std::string>(oStr, std::string {"ERROR: Failed to cast to string!"});
}

void pragma::lua::detail::register_lua_debug_tostring(lua_State *l, const std::type_info &typeInfo)
{
	// This is required to display __tostring with lua-debug
	// https://github.com/actboy168/lua-debug/issues/237#issuecomment-1475533297
	auto *registry = luabind::detail::class_registry::get_registry(l);
	auto *crep = registry->find_class(typeInfo);
	assert(crep);
	lua_rawgeti(l, LUA_REGISTRYINDEX, crep->metatable_ref());
	auto o = luabind::object {luabind::from_stack(l, -1)};
	o["__debugger_tostring"] = luabind::make_function(
	  l, +[](const luabind::object &o) -> std::string { return tostring(o); });
	o["__name"] = crep->name();
	lua_pop(l, 1);
}

static void add_task(lua_State *l, pragma::lua::LuaWorker &worker, const luabind::object &o0, const luabind::object &o1, float taskProgress)
{
	using TNonLuaTask = std::shared_ptr<util::ParallelJob<luabind::object>>;
	auto *nonLuaTask = luabind::object_cast_nothrow<TNonLuaTask *>(o0, static_cast<TNonLuaTask *>(nullptr));
	if(nonLuaTask) {
		auto &onCompleteTask = o1;
		if(luabind::type(onCompleteTask) != LUA_TFUNCTION)
			Lua::Error(l, "onCompleteTask is not a function!");
		worker.AddLuaTask(*nonLuaTask, onCompleteTask, taskProgress);
		return;
	}
	if(luabind::type(o0) == LUA_TFUNCTION) {
		auto &task = o0;
		auto &cancel = o1;
		if(luabind::type(cancel) != LUA_TFUNCTION)
			Lua::Error(l, "cancel is not a function!");
		worker.AddLuaTask(task, cancel, taskProgress);
		return;
	}
	auto &subJob = o0;
	auto &onCompleteTask = o1;
	worker.AddTask(subJob, onCompleteTask, taskProgress);
}

static void add_task(pragma::lua::LuaWorker &worker, const std::shared_ptr<util::ParallelJob<luabind::object>> &subJob, float taskProgress) { worker.AddLuaTask(subJob, taskProgress); }

void NetworkState::RegisterSharedLuaClasses(Lua::Interface &lua)
{
	auto modString = luabind::module_(lua.GetState(), "string");
	modString[luabind::def("snake_case_to_camel_case", Lua::string::snake_case_to_camel_case), luabind::def("camel_case_to_snake_case", Lua::string::camel_case_to_snake_case), luabind::def("calc_levenshtein_distance", Lua::string::calc_levenshtein_distance),
	  luabind::def("calc_levenshtein_similarity", Lua::string::calc_levenshtein_similarity),
	  luabind::def("find_longest_common_substring", Lua::string::find_longest_common_substring, luabind::meta::join<luabind::pure_out_value<3>, luabind::pure_out_value<4>, luabind::pure_out_value<5>>::type {}),
	  luabind::def("find_similar_elements", Lua::string::find_similar_elements, luabind::meta::join<luabind::pure_out_value<5>, luabind::pure_out_value<6>>::type {}), luabind::def("is_integer", ustring::is_integer), luabind::def("is_number", ustring::is_number),
	  luabind::def("split", Lua::string::split), luabind::def("join", static_cast<std::string (*)(lua_State *, luabind::table<>, const std::string &)>(Lua::string::join)),
	  luabind::def("join", static_cast<std::string (*)(lua_State *, luabind::table<>)>([](lua_State *l, luabind::table<> values) { return Lua::string::join(l, values); })), luabind::def("remove_whitespace", Lua::string::remove_whitespace),
	  luabind::def("remove_quotes", Lua::string::remove_quotes), luabind::def("replace", static_cast<std::string (*)(const std::string &, const std::string &, const std::string &)>([](const std::string &subject, const std::string &from, const std::string &to) -> std::string {
		  auto tmp = subject;
		  ustring::replace(tmp, from, to);
		  return tmp;
	  })),
	  luabind::def("fill_zeroes", ustring::fill_zeroes), luabind::def("compare", static_cast<bool (*)(const char *, const char *, bool, size_t)>(ustring::compare)), luabind::def("compare", static_cast<bool (*)(const std::string &, const std::string &, bool)>(ustring::compare)),
	  luabind::def("compare", static_cast<bool (*)(const std::string &, const std::string &)>([](const std::string &a, const std::string &b) -> bool { return ustring::compare(a, b, true); })),
	  luabind::def("hash", static_cast<std::string (*)(const std::string &)>([](const std::string &str) -> std::string { return std::to_string(std::hash<std::string> {}(str)); }))];

	auto modLight = luabind::module_(lua.GetState(), "light");
	modLight[luabind::def("get_color_temperature", static_cast<void (*)(ulighting::NaturalLightType, Kelvin &, Kelvin &)>([](ulighting::NaturalLightType type, Kelvin &outMin, Kelvin &outMax) {
		auto colTemp = ulighting::get_color_temperature(type);
		outMin = colTemp.first;
		outMax = colTemp.second;
	}),
	           luabind::meta::join<luabind::pure_out_value<2>, luabind::pure_out_value<3>>::type {}),
	  luabind::def("get_average_color_temperature", ulighting::get_average_color_temperature), luabind::def("color_temperature_to_color", ulighting::color_temperature_to_color), luabind::def("wavelength_to_color", ulighting::wavelength_to_color),
	  luabind::def("get_luminous_efficacy", ulighting::get_luminous_efficacy), luabind::def("lumens_to_watts", static_cast<Watt (*)(Lumen, LuminousEfficacy)>(ulighting::lumens_to_watts)),
	  luabind::def("lumens_to_watts", static_cast<Watt (*)(Lumen)>([](Lumen lumen) -> Watt { return ulighting::lumens_to_watts(lumen); })), luabind::def("watts_to_lumens", static_cast<Watt (*)(Lumen, LuminousEfficacy)>(ulighting::watts_to_lumens)),
	  luabind::def("watts_to_lumens", static_cast<Watt (*)(Lumen)>([](Watt watt) -> Lumen { return ulighting::watts_to_lumens(watt); })), luabind::def("irradiance_to_lux", ulighting::irradiance_to_lux), luabind::def("lux_to_irradiance", ulighting::lux_to_irradiance),
	  luabind::def(
	    "convert_light_intensity",
	    +[](float intensity, pragma::BaseEnvLightComponent::LightIntensityType srcType, pragma::BaseEnvLightComponent::LightIntensityType dstType, float coneAngle) -> float {
		    auto result = 0.f;
		    switch(dstType) {
		    case pragma::BaseEnvLightComponent::LightIntensityType::Candela:
			    result = pragma::BaseEnvLightComponent::GetLightIntensityCandela(intensity, srcType, coneAngle);
			    break;
		    case pragma::BaseEnvLightComponent::LightIntensityType::Lumen:
			    result = pragma::BaseEnvLightComponent::GetLightIntensityLumen(intensity, srcType, coneAngle);
			    break;
		    default:
			    break;
		    }
		    return result;
	    }),
	  luabind::def(
	    "convert_light_intensity", +[](float intensity, pragma::BaseEnvLightComponent::LightIntensityType srcType, pragma::BaseEnvLightComponent::LightIntensityType dstType) -> float {
		    auto result = 0.f;
		    switch(dstType) {
		    case pragma::BaseEnvLightComponent::LightIntensityType::Candela:
			    result = pragma::BaseEnvLightComponent::GetLightIntensityCandela(intensity, srcType);
			    break;
		    case pragma::BaseEnvLightComponent::LightIntensityType::Lumen:
			    result = pragma::BaseEnvLightComponent::GetLightIntensityLumen(intensity, srcType);
			    break;
		    default:
			    break;
		    }
		    return result;
	    })];

	Lua::RegisterLibraryEnums(lua.GetState(), "light",
	  {{"NATURAL_LIGHT_TYPE_MATCH_FLAME", umath::to_integral(ulighting::NaturalLightType::MatchFlame)}, {"NATURAL_LIGHT_TYPE_CANDLE", umath::to_integral(ulighting::NaturalLightType::Candle)}, {"NATURAL_LIGHT_TYPE_FLAME", umath::to_integral(ulighting::NaturalLightType::Flame)},
	    {"NATURAL_LIGHT_TYPE_SUNSET", umath::to_integral(ulighting::NaturalLightType::Sunset)}, {"NATURAL_LIGHT_TYPE_SUNRISE", umath::to_integral(ulighting::NaturalLightType::Sunrise)},
	    {"NATURAL_LIGHT_TYPE_HOUSEHOLD_TUNGSTEN_BULB", umath::to_integral(ulighting::NaturalLightType::HouseholdTungstenBulb)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_500W_TO_1K", umath::to_integral(ulighting::NaturalLightType::TungstenLamp500WTo1K)},
	    {"NATURAL_LIGHT_TYPE_INCANDESCENT_LAMP", umath::to_integral(ulighting::NaturalLightType::IncandescentLamp)}, {"NATURAL_LIGHT_TYPE_WARM_FLUORESCENT_LAMP", umath::to_integral(ulighting::NaturalLightType::WarmFluorescentLamp)},
	    {"NATURAL_LIGHT_TYPE_LED_LAMP", umath::to_integral(ulighting::NaturalLightType::LEDLamp)}, {"NATURAL_LIGHT_TYPE_QUARTZ_LIGHT", umath::to_integral(ulighting::NaturalLightType::QuartzLight)},
	    {"NATURAL_LIGHT_TYPE_STUDIO_LAMP", umath::to_integral(ulighting::NaturalLightType::StudioLamp)}, {"NATURAL_LIGHT_TYPE_FLOODLIGHT", umath::to_integral(ulighting::NaturalLightType::Floodlight)},
	    {"NATURAL_LIGHT_TYPE_FLUORESCENT_LIGHT", umath::to_integral(ulighting::NaturalLightType::FluorescentLight)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_2K", umath::to_integral(ulighting::NaturalLightType::TungstenLamp2K)},
	    {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_5K", umath::to_integral(ulighting::NaturalLightType::TungstenLamp5K)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_10K", umath::to_integral(ulighting::NaturalLightType::TungstenLamp10K)},
	    {"NATURAL_LIGHT_TYPE_MOONLIGHT", umath::to_integral(ulighting::NaturalLightType::Moonlight)}, {"NATURAL_LIGHT_TYPE_HORIZON_DAYLIGHT", umath::to_integral(ulighting::NaturalLightType::HorizonDaylight)},
	    {"NATURAL_LIGHT_TYPE_TUBULAR_FLUORESCENT_LAMP", umath::to_integral(ulighting::NaturalLightType::TubularFluorescentLamp)}, {"NATURAL_LIGHT_TYPE_VERTICAL_DAYLIGHT", umath::to_integral(ulighting::NaturalLightType::VerticalDaylight)},
	    {"NATURAL_LIGHT_TYPE_SUN_AT_NOON", umath::to_integral(ulighting::NaturalLightType::SunAtNoon)}, {"NATURAL_LIGHT_TYPE_DAYLIGHT", umath::to_integral(ulighting::NaturalLightType::Daylight)},
	    {"NATURAL_LIGHT_TYPE_SUN_THROUGH_CLOUDS", umath::to_integral(ulighting::NaturalLightType::SunThroughClouds)}, {"NATURAL_LIGHT_TYPE_OVERCAST", umath::to_integral(ulighting::NaturalLightType::Overcast)},
	    {"NATURAL_LIGHT_TYPE_RGB_MONITOR_WHITE_POINT", umath::to_integral(ulighting::NaturalLightType::RGBMonitorWhitePoint)}, {"NATURAL_LIGHT_TYPE_OUTDOOR_SHADE", umath::to_integral(ulighting::NaturalLightType::OutdoorShade)},
	    {"NATURAL_LIGHT_TYPE_PARTLY_CLOUDY", umath::to_integral(ulighting::NaturalLightType::PartlyCloudy)}, {"NATURAL_LIGHT_TYPE_CLEAR_BLUESKY", umath::to_integral(ulighting::NaturalLightType::ClearBlueSky)},
	    {"NATURAL_LIGHT_TYPE_CLEAR_COUNT", umath::to_integral(ulighting::NaturalLightType::Count)},

	    {"LIGHT_SOURCE_TYPE_TUNGSTEN_INCANDESCENT_LIGHT_BULB", umath::to_integral(ulighting::LightSourceType::TungstenIncandescentLightBulb)}, {"LIGHT_SOURCE_TYPE_HALOGEN_LAMP", umath::to_integral(ulighting::LightSourceType::HalogenLamp)},
	    {"LIGHT_SOURCE_TYPE_FLUORESCENT_LAMP", umath::to_integral(ulighting::LightSourceType::FluorescentLamp)}, {"LIGHT_SOURCE_TYPE_LED_LAMP", umath::to_integral(ulighting::LightSourceType::LEDLamp)},
	    {"LIGHT_SOURCE_TYPE_METAL_HALIDE_LAMP", umath::to_integral(ulighting::LightSourceType::MetalHalideLamp)}, {"LIGHT_SOURCE_TYPE_HIGH_PRESSURE_SODIUM_VAPOR_LAMP", umath::to_integral(ulighting::LightSourceType::HighPressureSodiumVaporLamp)},
	    {"LIGHT_SOURCE_TYPE_LOW_PRESSURE_SODIUM_VAPOR_LAMP", umath::to_integral(ulighting::LightSourceType::LowPressureSodiumVaporLamp)}, {"LIGHT_SOURCE_TYPE_MERCURY_VAPOR_LAMP", umath::to_integral(ulighting::LightSourceType::MercuryVaporLamp)},
	    {"LIGHT_SOURCE_TYPE_D65_STANDARD_ILLUMINANT", umath::to_integral(ulighting::LightSourceType::D65StandardIlluminant)}});

	auto &modUtil = lua.RegisterLibrary("util");
	register_directory_watcher(lua.GetState(), modUtil);

	auto defParallelJob = pragma::lua::register_class<util::BaseParallelJob>(lua.GetState(), "ParallelJob");
	defParallelJob->add_static_constant("JOB_STATUS_FAILED", umath::to_integral(util::JobStatus::Failed));
	defParallelJob->add_static_constant("JOB_STATUS_SUCCESSFUL", umath::to_integral(util::JobStatus::Successful));
	defParallelJob->add_static_constant("JOB_STATUS_INITIAL", umath::to_integral(util::JobStatus::Initial));
	defParallelJob->add_static_constant("JOB_STATUS_CANCELLED", umath::to_integral(util::JobStatus::Cancelled));
	defParallelJob->add_static_constant("JOB_STATUS_PENDING", umath::to_integral(util::JobStatus::Pending));
	defParallelJob->add_static_constant("JOB_STATUS_INVALID", umath::to_integral(util::JobStatus::Invalid));
	defParallelJob->def("Cancel", &util::BaseParallelJob::Cancel);
	defParallelJob->def("Wait", &util::BaseParallelJob::Wait);
	defParallelJob->def("Start", &util::BaseParallelJob::Start);
	defParallelJob->def("IsComplete", &util::BaseParallelJob::IsComplete);
	defParallelJob->def("IsPending", &util::BaseParallelJob::IsPending);
	defParallelJob->def("IsCancelled", &util::BaseParallelJob::IsCancelled);
	defParallelJob->def("IsSuccessful", &util::BaseParallelJob::IsSuccessful);
	defParallelJob->def("IsThreadActive", &util::BaseParallelJob::IsThreadActive);
	defParallelJob->def("GetProgress", &util::BaseParallelJob::GetProgress);
	defParallelJob->def("GetStatus", &util::BaseParallelJob::GetStatus);
	defParallelJob->def("GetResultMessage", &util::BaseParallelJob::GetResultMessage);
	defParallelJob->def("GetResultCode", &util::BaseParallelJob::GetResultCode);
	defParallelJob->def("IsValid", &util::BaseParallelJob::IsValid);
	defParallelJob->def("Poll", &util::BaseParallelJob::Poll);
	modUtil[*defParallelJob];

	auto defImageBuffer = pragma::lua::register_class<uimg::ImageBuffer>(lua.GetState(), "ImageBuffer");
	defImageBuffer->add_static_constant("FORMAT_NONE", umath::to_integral(uimg::Format::None));
	defImageBuffer->add_static_constant("FORMAT_RGB8", umath::to_integral(uimg::Format::RGB8));
	defImageBuffer->add_static_constant("FORMAT_RGBA8", umath::to_integral(uimg::Format::RGBA8));
	defImageBuffer->add_static_constant("FORMAT_RGB16", umath::to_integral(uimg::Format::RGB16));
	defImageBuffer->add_static_constant("FORMAT_RGBA16", umath::to_integral(uimg::Format::RGBA16));
	defImageBuffer->add_static_constant("FORMAT_RGB32", umath::to_integral(uimg::Format::RGB32));
	defImageBuffer->add_static_constant("FORMAT_RGBA32", umath::to_integral(uimg::Format::RGBA32));
	defImageBuffer->add_static_constant("FORMAT_COUNT", umath::to_integral(uimg::Format::Count));

	defImageBuffer->add_static_constant("FORMAT_RGB_LDR", umath::to_integral(uimg::Format::RGB_LDR));
	defImageBuffer->add_static_constant("FORMAT_RGBA_LDR", umath::to_integral(uimg::Format::RGBA_LDR));
	defImageBuffer->add_static_constant("FORMAT_RGB_HDR", umath::to_integral(uimg::Format::RGB_HDR));
	defImageBuffer->add_static_constant("FORMAT_RGBA_HDR", umath::to_integral(uimg::Format::RGBA_HDR));
	defImageBuffer->add_static_constant("FORMAT_RGB_FLOAT", umath::to_integral(uimg::Format::RGB_FLOAT));
	defImageBuffer->add_static_constant("FORMAT_RGBA_FLOAT", umath::to_integral(uimg::Format::RGBA_FLOAT));

	defImageBuffer->add_static_constant("CHANNEL_RED", umath::to_integral(uimg::Channel::Red));
	defImageBuffer->add_static_constant("CHANNEL_GREEN", umath::to_integral(uimg::Channel::Green));
	defImageBuffer->add_static_constant("CHANNEL_BLUE", umath::to_integral(uimg::Channel::Blue));
	defImageBuffer->add_static_constant("CHANNEL_ALPHA", umath::to_integral(uimg::Channel::Alpha));
	defImageBuffer->add_static_constant("CHANNEL_R", umath::to_integral(uimg::Channel::R));
	defImageBuffer->add_static_constant("CHANNEL_G", umath::to_integral(uimg::Channel::G));
	defImageBuffer->add_static_constant("CHANNEL_B", umath::to_integral(uimg::Channel::B));
	defImageBuffer->add_static_constant("CHANNEL_A", umath::to_integral(uimg::Channel::A));

	defImageBuffer->add_static_constant("TONE_MAPPING_GAMMA_CORRECTION", umath::to_integral(uimg::ToneMapping::GammaCorrection));
	defImageBuffer->add_static_constant("TONE_MAPPING_REINHARD", umath::to_integral(uimg::ToneMapping::Reinhard));
	defImageBuffer->add_static_constant("TONE_MAPPING_HEJIL_RICHARD", umath::to_integral(uimg::ToneMapping::HejilRichard));
	defImageBuffer->add_static_constant("TONE_MAPPING_UNCHARTED", umath::to_integral(uimg::ToneMapping::Uncharted));
	defImageBuffer->add_static_constant("TONE_MAPPING_ACES", umath::to_integral(uimg::ToneMapping::Aces));
	defImageBuffer->add_static_constant("TONE_MAPPING_GRAN_TURISMO", umath::to_integral(uimg::ToneMapping::GranTurismo));

	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua_State *, uint32_t, uint32_t, uint32_t, DataStream &)>([](lua_State *l, uint32_t width, uint32_t height, uint32_t format, DataStream &ds) {
		auto imgBuffer = uimg::ImageBuffer::Create(ds->GetData(), width, height, static_cast<uimg::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua_State *, uint32_t, uint32_t, uint32_t)>([](lua_State *l, uint32_t width, uint32_t height, uint32_t format) {
		auto imgBuffer = uimg::ImageBuffer::Create(width, height, static_cast<uimg::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
		auto imgBuffer = uimg::ImageBuffer::Create(parent, x, y, w, h);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("CreateCubemap", static_cast<void (*)(lua_State *, luabind::object)>([](lua_State *l, luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l, t);
		std::array<std::shared_ptr<uimg::ImageBuffer>, 6> cubemapSides {};
		for(uint8_t i = 0; i < 6; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, t);
			auto &img = Lua::Check<uimg::ImageBuffer>(l, -1);
			cubemapSides.at(i) = img.shared_from_this();
			Lua::Pop(l, 1);
		}
		auto imgBuffer = uimg::ImageBuffer::CreateCubemap(cubemapSides);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->def("GetData", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) {
		auto *data = imgBuffer.GetData();
		auto dataSize = imgBuffer.GetSize();
		DataStream ds {data, static_cast<uint32_t>(dataSize)};
		ds->SetOffset(0);
		Lua::Push(l, ds);
	}));
	defImageBuffer->def("GetFormat", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, umath::to_integral(imgBuffer.GetFormat())); }));
	defImageBuffer->def("GetWidth", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetWidth()); }));
	defImageBuffer->def("GetHeight", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetHeight()); }));
	defImageBuffer->def("GetChannelCount", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetChannelCount()); }));
	defImageBuffer->def("GetChannelSize", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetChannelSize()); }));
	defImageBuffer->def("GetPixelSize", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetPixelSize()); }));
	defImageBuffer->def("GetPixelCount", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetPixelCount()); }));
	defImageBuffer->def("HasAlphaChannel", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.HasAlphaChannel()); }));
	defImageBuffer->def("IsLDRFormat", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsLDRFormat()); }));
	defImageBuffer->def("IsHDRFormat", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsHDRFormat()); }));
	defImageBuffer->def("IsFloatFormat", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsFloatFormat()); }));
	defImageBuffer->def("Insert", static_cast<void (uimg::ImageBuffer::*)(const uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(&uimg::ImageBuffer::Insert));
	defImageBuffer->def("Insert", static_cast<void (uimg::ImageBuffer::*)(const uimg::ImageBuffer &, uint32_t, uint32_t)>(&uimg::ImageBuffer::Insert));
	defImageBuffer->def("Copy", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::Push(l, imgBuffer.Copy()); }));
	defImageBuffer->def("Copy", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t format) { Lua::Push(l, imgBuffer.Copy(static_cast<uimg::Format>(format))); }));
	defImageBuffer->def("Copy",
	  static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, uimg::ImageBuffer &imgBuffer, uimg::ImageBuffer &dst, uint32_t xSrc, uint32_t ySrc, uint32_t xDst, uint32_t yDst, uint32_t w, uint32_t h) { imgBuffer.Copy(dst, xSrc, ySrc, xDst, yDst, w, h); }));
	defImageBuffer->def("Convert", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t format) { imgBuffer.Convert(static_cast<uimg::Format>(format)); }));
	defImageBuffer->def(
	  "ToLDR", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, const pragma::lua::LuaThreadWrapper &tw) {
		  auto pImgBuffer = imgBuffer.shared_from_this();
		  auto task = [pImgBuffer]() -> pragma::lua::LuaThreadPool::ResultHandler {
			  pImgBuffer->ToLDR();
			  return {};
		  };
		  if(tw.IsTask())
			  tw.GetTask()->AddSubTask(task);
		  else
			  tw.GetPool().AddTask(task);
	  });
	defImageBuffer->def("ToLDR", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.ToLDR(); }));
	defImageBuffer->def("ToHDR", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.ToHDR(); }));
	defImageBuffer->def("ToFloat", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.ToFloat(); }));
	defImageBuffer->def("GetSize", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetSize()); }));
	defImageBuffer->def("Clear", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, const Color &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, const Color &color) { imgBuffer.Clear(color); }));
	defImageBuffer->def("Clear", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, const Vector4 &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, const Vector4 &color) { imgBuffer.Clear(color); }));
	defImageBuffer->def("ClearAlpha", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, float)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, float a) {
		a = umath::clamp(a, 0.f, 1.f);
		imgBuffer.ClearAlpha(a * std::numeric_limits<uint8_t>::max());
	}));
	defImageBuffer->def("GetPixelIndex", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelIndex(x, y)); }));
	defImageBuffer->def("GetPixelOffset", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelOffset(x, y)); }));
	defImageBuffer->def("Resize", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) { imgBuffer.Resize(w, h); }));
	defImageBuffer->def("FlipHorizontally", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.FlipHorizontally(); }));
	defImageBuffer->def("FlipVertically", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.FlipVertically(); }));
	defImageBuffer->def("Flip", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, bool, bool)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, bool flipH, bool flipV) { imgBuffer.Flip(flipH, flipV); }));
	defImageBuffer->def("SwapChannels", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uimg::Channel, uimg::Channel)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uimg::Channel channel0, uimg::Channel channel1) { imgBuffer.SwapChannels(channel0, channel1); }));
	defImageBuffer->def(
	  "SwapChannels", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, uimg::Channel channel0, uimg::Channel channel1, const pragma::lua::LuaThreadWrapper &tw) {
		  auto pImgBuffer = imgBuffer.shared_from_this();
		  auto task = [pImgBuffer, channel0, channel1]() -> pragma::lua::LuaThreadPool::ResultHandler {
			  pImgBuffer->SwapChannels(channel0, channel1);
			  return {};
		  };
		  if(tw.IsTask())
			  tw.GetTask()->AddSubTask(task);
		  else
			  tw.GetPool().AddTask(task);
	  });
	defImageBuffer->def("ApplyToneMapping", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t toneMapping) {
		auto tonemappedImg = imgBuffer.ApplyToneMapping(static_cast<uimg::ToneMapping>(toneMapping));
		if(tonemappedImg == nullptr)
			return;
		Lua::Push(l, tonemappedImg);
	}));
	defImageBuffer->def("ApplyGammaCorrection", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) { imgBuffer.ApplyGammaCorrection(); }));
	defImageBuffer->def("ApplyGammaCorrection", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, float)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, float gamma) { imgBuffer.ApplyGammaCorrection(gamma); }));
	defImageBuffer->def("ApplyExposure", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, float)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, float exposure) { imgBuffer.ApplyExposure(exposure); }));

	defImageBuffer->def("GetPixelOffset", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelOffset(x, y)); }));
	defImageBuffer->def("GetPixelIndex", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelIndex(x, y)); }));
	defImageBuffer->def("GetPixelValue", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel) {
		Lua::PushNumber(l, imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).GetFloatValue(static_cast<uimg::Channel>(channel)));
	}));
	defImageBuffer->def("SetPixelValue", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, float)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, float value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<uimg::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelValueLDR", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint8_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, uint8_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<uimg::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelValueHDR", static_cast<void (*)(lua_State *, uimg::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint16_t)>([](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, uint16_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<uimg::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelColor", static_cast<void (uimg::ImageBuffer::*)(uint32_t, uint32_t, const Vector4 &)>(&uimg::ImageBuffer::SetPixelColor));
	defImageBuffer->def("SetPixelColor", static_cast<void (uimg::ImageBuffer::*)(uimg::ImageBuffer::PixelIndex, const Vector4 &)>(&uimg::ImageBuffer::SetPixelColor));
	defImageBuffer->def(
	  "SetPixelColor", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, const Color &color) { imgBuffer.SetPixelColor(x, y, color.ToVector4()); });
	defImageBuffer->def(
	  "SetPixelColor", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, uimg::ImageBuffer::PixelIndex pixelIdx, const Color &color) { imgBuffer.SetPixelColor(pixelIdx, color.ToVector4()); });
	defImageBuffer->def("CalcLuminance", static_cast<void (*)(lua_State *, uimg::ImageBuffer &)>([](lua_State *l, uimg::ImageBuffer &imgBuffer) {
		float avgLuminance, minLuminance, maxLuminance, logAvgLuminance;
		Vector3 avgIntensity;
		imgBuffer.CalcLuminance(avgLuminance, minLuminance, maxLuminance, avgIntensity, &logAvgLuminance);
		Lua::PushNumber(l, avgLuminance);
		Lua::PushNumber(l, minLuminance);
		Lua::PushNumber(l, maxLuminance);
		Lua::Push<Vector3>(l, avgIntensity);
		Lua::PushNumber(l, logAvgLuminance);
	}));
	modUtil[*defImageBuffer];

	auto defImageLayerSet = pragma::lua::register_class<uimg::ImageLayerSet>(lua.GetState(), "ImageLayerSet");
	defImageLayerSet->def(
	  "GetImage", +[](const uimg::ImageLayerSet &layerSet, const std::string &name) -> std::shared_ptr<uimg::ImageBuffer> {
		  auto it = layerSet.images.find(name);
		  if(it == layerSet.images.end())
			  return nullptr;
		  return it->second;
	  });
	defImageLayerSet->def(
	  "GetImages", +[](const uimg::ImageLayerSet &layerSet) { return layerSet.images; });
	modUtil[*defImageLayerSet];

	auto defWorker = luabind::class_<pragma::lua::LuaWorker>("Worker");
	defWorker.add_static_constant("TASK_STATUS_PENDING", umath::to_integral(pragma::lua::LuaWorker::TaskStatus::Pending));
	defWorker.add_static_constant("TASK_STATUS_COMPLETE", umath::to_integral(pragma::lua::LuaWorker::TaskStatus::Complete));
	defWorker.def("GetResult", &pragma::lua::LuaWorker::GetResult);
	defWorker.def("SetResult", &pragma::lua::LuaWorker::SetResult);
	defWorker.def("SetStatus", &pragma::lua::LuaWorker::SetStatus);
	defWorker.def(
	  "SetStatus", +[](pragma::lua::LuaWorker &worker, util::JobStatus jobStatus, const std::optional<std::string> &resultMsg) { worker.SetStatus(jobStatus, resultMsg); });
	defWorker.def(
	  "SetStatus", +[](pragma::lua::LuaWorker &worker, util::JobStatus jobStatus) { worker.SetStatus(jobStatus); });
	defWorker.def("UpdateProgress", &pragma::lua::LuaWorker::UpdateProgress);
	defWorker.def("AddTask", static_cast<void (*)(pragma::lua::LuaWorker &, const std::shared_ptr<util::ParallelJob<luabind::object>> &, float)>(&add_task));
	defWorker.def("AddTask", static_cast<void (*)(lua_State *, pragma::lua::LuaWorker &, const luabind::object &, const luabind::object &, float)>(&add_task));
	defWorker.def("SetProgressCallback", &pragma::lua::LuaWorker::SetProgressCallback);
	defWorker.def(
	  "Cancel", +[](pragma::lua::LuaWorker &worker) { worker.Cancel(); });
	defWorker.def(
	  "IsComplete", +[](pragma::lua::LuaWorker &worker) { return worker.IsComplete(); });
	defWorker.def(
	  "IsPending", +[](pragma::lua::LuaWorker &worker) { return worker.IsPending(); });
	defWorker.def(
	  "IsCancelled", +[](pragma::lua::LuaWorker &worker) { return worker.IsCancelled(); });
	defWorker.def(
	  "IsSuccessful", +[](pragma::lua::LuaWorker &worker) { return worker.IsSuccessful(); });
	defWorker.def(
	  "IsThreadActive", +[](pragma::lua::LuaWorker &worker) { return worker.IsThreadActive(); });
	defWorker.def(
	  "GetProgress", +[](pragma::lua::LuaWorker &worker) { return worker.GetProgress(); });
	defWorker.def(
	  "GetStatus", +[](pragma::lua::LuaWorker &worker) { return worker.GetStatus(); });
	defWorker.def(
	  "GetResultMessage", +[](pragma::lua::LuaWorker &worker) { return worker.GetResultMessage(); });
	defWorker.def(
	  "GetResultCode", +[](pragma::lua::LuaWorker &worker) { return worker.GetResultCode(); });
	defWorker.def(
	  "IsValid", +[](pragma::lua::LuaWorker &worker) { return worker.IsValid(); });
	modUtil[defWorker];

	auto defLuaParallelJob = luabind::class_<util::ParallelJob<luabind::object>, util::BaseParallelJob>("ParallelJob");
	defLuaParallelJob.def("GetResult", static_cast<luabind::object (*)(lua_State *, util::ParallelJob<luabind::object> &)>([](lua_State *l, util::ParallelJob<luabind::object> &job) -> luabind::object { return job.GetResult(); }));
	defLuaParallelJob.def(
	  "CallOnComplete", +[](util::ParallelJob<luabind::object> &job, const Lua::func<void> &onComplete) { static_cast<pragma::lua::LuaWorker &>(job.GetWorker()).CallOnComplete(onComplete); });
	defLuaParallelJob.def(
	  "SetProgressCallback", +[](util::ParallelJob<luabind::object> &job, const Lua::func<float> &func) { static_cast<pragma::lua::LuaWorker &>(job.GetWorker()).SetProgressCallback(func); });
	modUtil[defLuaParallelJob];
	modUtil[luabind::def(
	  "create_parallel_job", +[](Game &game, const std::string &name, const Lua::func<void> &func, const Lua::func<void> &cancelFunc) -> std::shared_ptr<util::ParallelJob<luabind::object>> {
		  auto job = std::make_shared<util::ParallelJob<luabind::object>>(util::create_parallel_job<pragma::lua::LuaWorker>(game, name));
		  static_cast<pragma::lua::LuaWorker &>(job->GetWorker()).AddLuaTask(func, cancelFunc, 0.f);
		  return job;
	  })];

	auto defGenericParallelJob = luabind::class_<util::ParallelJob<void>, util::BaseParallelJob>("ParallelJobGeneric");
	modUtil[defGenericParallelJob];

	auto defImgParallelJob = luabind::class_<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>, util::BaseParallelJob>("ParallelJobImage");
	defImgParallelJob.def("GetResult", static_cast<void (*)(lua_State *, util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &)>([](lua_State *l, util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defImgParallelJob];

	auto defImgLayerSetParallelJob = luabind::class_<util::ParallelJob<uimg::ImageLayerSet>, util::BaseParallelJob>("ParallelJobImageLayerSet");
	defImgLayerSetParallelJob.def(
	  "GetResult", +[](lua_State *l, util::ParallelJob<uimg::ImageLayerSet> &job) { Lua::Push(l, job.GetResult()); });
	defImgLayerSetParallelJob.def(
	  "GetImage", +[](lua_State *l, util::ParallelJob<uimg::ImageLayerSet> &job) -> std::shared_ptr<uimg::ImageBuffer> {
		  if(job.GetResult().images.empty())
			  return nullptr;
		  return job.GetResult().images.begin()->second;
	  });
	modUtil[defImgLayerSetParallelJob];

	auto defStringParallelJob = luabind::class_<util::ParallelJob<const std::string &>, util::BaseParallelJob>("ParallelJobString");
	defStringParallelJob.def("GetResult", static_cast<void (*)(lua_State *, util::ParallelJob<const std::string &> &)>([](lua_State *l, util::ParallelJob<const std::string &> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defStringParallelJob];

	auto defDataStreamJob = luabind::class_<util::ParallelJob<const DataStream &>, util::BaseParallelJob>("ParallelJobData");
	defDataStreamJob.def("GetResult", static_cast<void (*)(lua_State *, util::ParallelJob<const DataStream &> &)>([](lua_State *l, util::ParallelJob<const DataStream &> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defDataStreamJob];

	auto defDataBlock = luabind::class_<ds::Block>("DataBlock");
	defDataBlock.scope[luabind::def("load", static_cast<void (*)(lua_State *, const std::string &)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("load", static_cast<void (*)(lua_State *, LFile &)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("create", static_cast<void (*)(lua_State *)>(Lua::DataBlock::create))];

	defDataBlock.def("GetInt", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector2", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetVector2));
	defDataBlock.def("GetVector4", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::GetVector4));

	defDataBlock.def("GetInt", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, int32_t)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, float)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, bool)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, const std::string &)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, const Color &)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, const Vector3 &)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector2", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, const ::Vector2 &)>(&Lua::DataBlock::GetVector2));
	defDataBlock.def("GetVector4", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, const Vector4 &)>(&Lua::DataBlock::GetVector4));

	defDataBlock.def("GetData", &Lua::DataBlock::GetData);
	defDataBlock.def("GetChildBlocks", &Lua::DataBlock::GetChildBlocks);
	defDataBlock.def("SetValue", &Lua::DataBlock::SetValue);
	defDataBlock.def("Merge", &Lua::DataBlock::Merge);
	defDataBlock.def("GetValueType", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>([](lua_State *l, ds::Block &dataBlock, const std::string &key) {
		auto val = dataBlock.GetDataValue(key);
		if(val == nullptr)
			return;
		Lua::PushString(l, val->GetTypeString());
	}));
	defDataBlock.def("GetKeys", static_cast<void (*)(lua_State *, ds::Block &)>([](lua_State *l, ds::Block &dataBlock) {
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &pair : *dataBlock.GetData()) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, pair.first);
			Lua::SetTableValue(l, t);
		}
	}));

	defDataBlock.def("RemoveValue", &Lua::DataBlock::RemoveValue);
	defDataBlock.def("IsEmpty", &Lua::DataBlock::IsEmpty);
	defDataBlock.def("HasValue", &Lua::DataBlock::HasValue);
	defDataBlock.def("AddBlock", &Lua::DataBlock::AddBlock);
	defDataBlock.def("IsString", &Lua::DataBlock::IsString);
	defDataBlock.def("IsInt", &Lua::DataBlock::IsInt);
	defDataBlock.def("IsFloat", &Lua::DataBlock::IsFloat);
	defDataBlock.def("IsBool", &Lua::DataBlock::IsBool);
	defDataBlock.def("IsColor", &Lua::DataBlock::IsColor);
	defDataBlock.def("IsVector", &Lua::DataBlock::IsVector);
	defDataBlock.def("IsVector4", &Lua::DataBlock::IsVector4);
	defDataBlock.def("ToString", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, uint8_t)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua_State *, ds::Block &, uint8_t)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua_State *, ds::Block &)>(&Lua::DataBlock::ToString));
	defDataBlock.def("FindBlock", static_cast<void (*)(lua_State *, ds::Block &, const std::string &)>(&Lua::DataBlock::FindBlock));
	defDataBlock.def("FindBlock", static_cast<void (*)(lua_State *, ds::Block &, const std::string &, uint32_t)>(&Lua::DataBlock::FindBlock));
	modUtil[defDataBlock];

	// Version
	auto defVersion = pragma::lua::register_class<util::Version>(lua.GetState(), "Version");
	defVersion->def(luabind::constructor<>());
	defVersion->def(luabind::constructor<uint32_t, uint32_t>());
	defVersion->def(luabind::constructor<uint32_t, uint32_t, uint32_t>());
	defVersion->def(luabind::constructor<const std::string &>());
	defVersion->def(luabind::const_self == luabind::const_self);
	defVersion->def(luabind::const_self < luabind::const_self);
	defVersion->def(luabind::const_self <= luabind::const_self);
	defVersion->def_readwrite("major", &util::Version::major);
	defVersion->def_readwrite("minor", &util::Version::minor);
	defVersion->def_readwrite("revision", &util::Version::revision);
	defVersion->def("Reset", &util::Version::Reset);
	defVersion->def("ToString", &util::Version::ToString);
	modUtil[*defVersion];
	//

	// Path
	auto defPath = pragma::lua::register_class<util::Path>(lua.GetState(), "Path");
	defPath->scope[luabind::def("CreateFilePath", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &path) { Lua::Push<util::Path>(l, util::Path::CreateFile(path)); }))];
	defPath->scope[luabind::def("CreateFilePath", static_cast<void (*)(lua_State *, const util::Path &)>([](lua_State *l, const util::Path &path) { Lua::Push<util::Path>(l, util::Path::CreateFile(path.GetString())); }))];
	defPath->scope[luabind::def("CreatePath", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &path) { Lua::Push<util::Path>(l, util::Path::CreatePath(path)); }))];
	defPath->scope[luabind::def("CreatePath", static_cast<void (*)(lua_State *, const util::Path &)>([](lua_State *l, const util::Path &path) { Lua::Push<util::Path>(l, util::Path::CreatePath(path.GetString())); }))];
	defPath->scope[luabind::def("CreateFromComponents", static_cast<void (*)(lua_State *, luabind::object)>([](lua_State *l, luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l, t);
		std::vector<std::string> components {};
		auto n = Lua::GetObjectLength(l, t);
		components.reserve(n);
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, t);
			components.push_back(Lua::CheckString(l, -1));

			Lua::Pop(l, 1);
		}
		Lua::Push<util::Path>(l, {components});
	}))];
	defPath->def(luabind::constructor<>());
	defPath->def(luabind::constructor<const util::Path &>());
	defPath->def(luabind::constructor<const std::string &>());

	defPath->def(luabind::self + luabind::const_self);
	defPath->def(luabind::self + std::string {});

	defPath->def(luabind::const_self == luabind::const_self);
	defPath->def(luabind::const_self == std::string {});

	defPath->def("Copy", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::Push<util::Path>(l, p); }));
	defPath->def("ToComponents", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) {
		auto components = p.ToComponents();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &c : components) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, c);
			Lua::SetTableValue(l, t);
		}
	}));
	defPath->def("GetString", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushString(l, p.GetString()); }));
	defPath->def("GetPath", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetPath()}); }));
	defPath->def("GetFileName", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetFileName()}); }));
	defPath->def("GetFront", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetFront()}); }));
	defPath->def("GetBack", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetBack()}); }));
	defPath->def("MoveUp", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { p.MoveUp(); }));
	defPath->def("PopFront", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { p.PopFront(); }));
	defPath->def("PopBack", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { p.PopBack(); }));
	defPath->def("Canonicalize", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { p.Canonicalize(); }));
	defPath->def("IsFile", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushBool(l, p.IsFile()); }));
	defPath->def("IsPath", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { Lua::PushBool(l, !p.IsFile()); }));
	defPath->def("GetFileExtension", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) {
		auto ext = p.GetFileExtension();
		if(ext.has_value() == false)
			return;
		Lua::PushString(l, *ext);
	}));
	defPath->def("RemoveFileExtension", static_cast<void (*)(lua_State *, util::Path &)>([](lua_State *l, util::Path &p) { p.RemoveFileExtension(); }));
	defPath->def(
	  "RemoveFileExtension", +[](lua_State *l, util::Path &p, const std::vector<std::string> &extensions) { p.RemoveFileExtension(extensions); });
	defPath->def(
	  "MakeRelative", +[](lua_State *l, util::Path &p, util::Path &pOther) { return p.MakeRelative(pOther); });
	defPath->def(
	  "MakeRelative", +[](lua_State *l, util::Path &p, const std::string &other) { return p.MakeRelative(other); });
	defPath->def("GetComponentCount", &util::Path::GetComponentCount);
	defPath->def(
	  "GetComponent", +[](util::Path &p, size_t offset) -> std::optional<std::pair<std::string_view, size_t>> {
		  if(offset >= p.GetString().size())
			  return {};
		  size_t nextOffset;
		  auto sv = p.GetComponent(offset, &nextOffset);
		  return std::pair<std::string_view, size_t> {sv, nextOffset};
	  });
	defPath->def("IsEmpty", &util::Path::IsEmpty);
	modUtil[*defPath];

	// Properties
	Lua::Property::register_classes(lua);

	auto &modMath = lua.RegisterLibrary("math");

	// Transform
	auto classDefTransform = luabind::class_<umath::Transform>("Transform");
	classDefTransform.def(luabind::constructor<const Mat4 &>());
	classDefTransform.def(luabind::constructor<const Vector3 &, const Quat &>());
	classDefTransform.def(luabind::constructor<const Vector3 &>());
	classDefTransform.def(luabind::constructor<const Quat &>());
	classDefTransform.def(luabind::constructor<const umath::ScaledTransform &>());
	classDefTransform.def(luabind::constructor<>());
	classDefTransform.def(luabind::const_self == luabind::const_self);
	classDefTransform.def(luabind::tostring(luabind::self));
	classDefTransform.def("Copy", static_cast<umath::Transform (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &t) -> umath::Transform { return t; }));
	classDefTransform.property("x", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetOrigin().x; }),
	  static_cast<void (*)(lua_State *, umath::Transform &, float)>([](lua_State *l, umath::Transform &pose, float x) { pose.GetOrigin().x = x; }));
	classDefTransform.property("y", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetOrigin().y; }),
	  static_cast<void (*)(lua_State *, umath::Transform &, float)>([](lua_State *l, umath::Transform &pose, float y) { pose.GetOrigin().y = y; }));
	classDefTransform.property("z", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetOrigin().z; }),
	  static_cast<void (*)(lua_State *, umath::Transform &, float)>([](lua_State *l, umath::Transform &pose, float z) { pose.GetOrigin().z = z; }));
	classDefTransform.property("pitch", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetAngles().p; }));
	classDefTransform.property("yaw", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetAngles().y; }));
	classDefTransform.property("roll", static_cast<float (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &pose) { return pose.GetAngles().r; }));
	classDefTransform.def("ToPlane", &umath::Transform::ToPlane);
	classDefTransform.def("GetAngles", &umath::Transform::GetAngles);
	classDefTransform.def("SetAngles", &umath::Transform::SetAngles);
	classDefTransform.def("GetForward", &umath::Transform::GetForward);
	classDefTransform.def("GetRight", &umath::Transform::GetRight);
	classDefTransform.def("GetUp", &umath::Transform::GetUp);
	classDefTransform.def("Set", static_cast<void (*)(lua_State *, umath::Transform &, const umath::Transform &)>([](lua_State *l, umath::Transform &t, const umath::Transform &tOther) { t = tOther; }));
	classDefTransform.def("GetOrigin", static_cast<Vector3 &(umath::Transform::*)()>(&umath::Transform::GetOrigin), luabind::copy_policy<0> {});
	classDefTransform.def("GetRotation", static_cast<Quat &(umath::Transform::*)()>(&umath::Transform::GetRotation), luabind::copy_policy<0> {});
	classDefTransform.def("SetOrigin", &umath::Transform::SetOrigin);
	classDefTransform.def("SetRotation", &umath::Transform::SetRotation);
	classDefTransform.def("SetIdentity", &umath::Transform::SetIdentity);
	classDefTransform.def("IsIdentity", static_cast<bool (*)(lua_State *, umath::Transform &)>([](lua_State *l, umath::Transform &t) -> bool {
		auto &origin = t.GetOrigin();
		auto &rotation = t.GetRotation();
		return umath::abs(origin.x) < 0.001f && umath::abs(origin.y) < 0.001f && umath::abs(origin.z) < 0.001f && umath::abs(1.f - rotation.w) < 0.001f && umath::abs(rotation.x) < 0.001f && umath::abs(rotation.y) < 0.001f && umath::abs(rotation.z) < 0.001f;
	}));
	classDefTransform.def("TranslateGlobal", &umath::Transform::TranslateGlobal);
	classDefTransform.def("TranslateLocal", &umath::Transform::TranslateLocal);
	classDefTransform.def("RotateGlobal", &umath::Transform::RotateGlobal);
	classDefTransform.def("RotateLocal", &umath::Transform::RotateLocal);
	classDefTransform.def("TransformGlobal", static_cast<void (*)(lua_State *, umath::Transform &, const umath::Transform &)>([](lua_State *l, umath::Transform &t, const umath::Transform &t2) { t = t2 * t; }));
	classDefTransform.def("TransformLocal", static_cast<void (*)(lua_State *, umath::Transform &, const umath::Transform &)>([](lua_State *l, umath::Transform &t, const umath::Transform &t2) { t *= t2; }));
	classDefTransform.def("GetInverse", &umath::Transform::GetInverse);
	classDefTransform.def("ToMatrix", &umath::Transform::ToMatrix);
	classDefTransform.def("SetMatrix", static_cast<void (*)(lua_State *, umath::Transform &, const Mat4 &)>([](lua_State *l, umath::Transform &t, const Mat4 &m) {
		Mat4 transformation;
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::decompose(transformation, scale, rotation, translation, skew, perspective);
		t.SetOrigin(translation);
		t.SetRotation(rotation);
	}));
	classDefTransform.def("Interpolate", &umath::Transform::Interpolate);
	classDefTransform.def("InterpolateToIdentity", &umath::Transform::InterpolateToIdentity);
	classDefTransform.def("Reflect", &umath::Transform::Reflect);
	classDefTransform.def(luabind::const_self * luabind::const_self);
	classDefTransform.def(luabind::const_self * umath::ScaledTransform());
	classDefTransform.def(luabind::const_self * Vector3());
	classDefTransform.def(luabind::const_self * Quat());
	classDefTransform.def(luabind::const_self * umath::Plane());

	modMath[classDefTransform];
	Lua::RegisterLibraryValues<umath::Transform>(lua.GetState(), "math.Transform", {std::pair<std::string, umath::Transform> {"IDENTITY", umath::Transform {}}});

	auto classDefScaledTransform = luabind::class_<umath::ScaledTransform, umath::Transform>("ScaledTransform");
	classDefScaledTransform.def(luabind::constructor<const Mat4 &>());
	classDefScaledTransform.def(luabind::constructor<const Vector3 &, const Quat &>());
	classDefScaledTransform.def(luabind::constructor<const Vector3 &, const Quat &, const Vector3 &>());
	classDefScaledTransform.def(luabind::constructor<const umath::Transform &, const Vector3 &>());
	classDefScaledTransform.def(luabind::constructor<const umath::Transform &>());
	classDefScaledTransform.def(luabind::constructor<>());
	classDefScaledTransform.def(luabind::const_self == luabind::const_self);
	classDefScaledTransform.def(luabind::tostring(luabind::self));
	classDefScaledTransform.def("Copy", static_cast<umath::ScaledTransform (*)(lua_State *, umath::ScaledTransform &)>([](lua_State *l, umath::ScaledTransform &t) -> umath::ScaledTransform { return t; }));
	classDefScaledTransform.def("Set", static_cast<void (*)(lua_State *, umath::ScaledTransform &, const umath::ScaledTransform &)>([](lua_State *l, umath::ScaledTransform &t, const umath::ScaledTransform &tOther) { t = tOther; }));
	classDefScaledTransform.def("GetScale", static_cast<Vector3 &(umath::ScaledTransform::*)()>(&umath::ScaledTransform::GetScale), luabind::copy_policy<0> {});
	classDefScaledTransform.def("SetScale", &umath::ScaledTransform::SetScale);
	classDefScaledTransform.def("Scale", &umath::ScaledTransform::Scale);
	classDefScaledTransform.def("GetInverse", &umath::ScaledTransform::GetInverse);
	classDefScaledTransform.def("Interpolate", &umath::ScaledTransform::Interpolate);
	classDefScaledTransform.def("InterpolateToIdentity", &umath::ScaledTransform::InterpolateToIdentity);
	classDefScaledTransform.def(luabind::const_self * umath::Transform());
	classDefScaledTransform.def(luabind::const_self * umath::ScaledTransform()); // Note: We use umath::ScaledTransform instead of luabind::const_self, because otherwise the overload of the base class ("Transform") would be used if two ScaledTransforms are multiplied
	classDefScaledTransform.def(luabind::const_self * Vector3());
	classDefScaledTransform.def(luabind::const_self * Quat());
	classDefScaledTransform.def(luabind::const_self * umath::Plane());
	modMath[classDefScaledTransform];
	Lua::RegisterLibraryValues<umath::ScaledTransform>(lua.GetState(), "math.ScaledTransform", {std::pair<std::string, umath::ScaledTransform> {"IDENTITY", umath::ScaledTransform {}}});

	// PID Controller
	auto defPIDController = luabind::class_<util::PIDController>("PIDController");
	defPIDController.def(luabind::constructor<>());
	defPIDController.def(luabind::constructor<float, float, float>());
	defPIDController.def(luabind::constructor<float, float, float, float, float>());
	defPIDController.def("SetProportionalTerm", &util::PIDController::SetProportionalTerm);
	defPIDController.def("SetIntegralTerm", &util::PIDController::SetIntegralTerm);
	defPIDController.def("SetDerivativeTerm", &util::PIDController::SetDerivativeTerm);
	defPIDController.def("SetTerms", &util::PIDController::SetTerms);
	defPIDController.def("GetProportionalTerm", &util::PIDController::GetProportionalTerm);
	defPIDController.def("GetIntegralTerm", &util::PIDController::GetIntegralTerm);
	defPIDController.def("GetDerivativeTerm", &util::PIDController::GetDerivativeTerm);
	defPIDController.def("GetTerms", static_cast<void (*)(lua_State *, const util::PIDController &)>([](lua_State *l, const util::PIDController &pidController) {
		auto p = 0.f;
		auto i = 0.f;
		auto d = 0.f;
		pidController.GetTerms(p, i, d);
		Lua::PushNumber(l, p);
		Lua::PushNumber(l, i);
		Lua::PushNumber(l, d);
	}));
	defPIDController.def("SetRange", &util::PIDController::SetRange);
	defPIDController.def("GetRange", static_cast<void (*)(lua_State *, const util::PIDController &)>([](lua_State *l, const util::PIDController &pidController) {
		auto range = pidController.GetRange();
		Lua::PushNumber(l, range.first);
		Lua::PushNumber(l, range.second);
	}));
	defPIDController.def("Calculate", &util::PIDController::Calculate);
	defPIDController.def("Reset", &util::PIDController::Reset);
	defPIDController.def("ClearRange", &util::PIDController::ClearRange);
	defPIDController.def("SetMin", &util::PIDController::SetMin);
	defPIDController.def("SetMax", &util::PIDController::SetMax);
	modMath[defPIDController];

	// Noise
	auto defNoiseModule = luabind::class_<NoiseBaseModule>("NoiseModule");
	defNoiseModule.def("GetValue", &Lua::noise::NoiseModule::GetValue);
	defNoiseModule.def("SetScale", &Lua::noise::NoiseModule::SetScale);

	auto defNoiseAbs = luabind::class_<NoiseAbs, NoiseBaseModule>("Abs");
	defNoiseModule.scope[defNoiseAbs];

	auto defNoiseBillow = luabind::class_<NoiseBillow, NoiseBaseModule>("Billow");
	defNoiseBillow.def("GetFrequency", &Lua::noise::BillowNoise::GetFrequency);
	defNoiseBillow.def("GetLacunarity", &Lua::noise::BillowNoise::GetLacunarity);
	defNoiseBillow.def("GetNoiseQuality", &Lua::noise::BillowNoise::GetNoiseQuality);
	defNoiseBillow.def("GetOctaveCount", &Lua::noise::BillowNoise::GetOctaveCount);
	defNoiseBillow.def("GetPersistence", &Lua::noise::BillowNoise::GetPersistence);
	defNoiseBillow.def("GetSeed", &Lua::noise::BillowNoise::GetSeed);
	defNoiseBillow.def("SetFrequency", &Lua::noise::BillowNoise::SetFrequency);
	defNoiseBillow.def("SetLacunarity", &Lua::noise::BillowNoise::SetLacunarity);
	defNoiseBillow.def("SetNoiseQuality", &Lua::noise::BillowNoise::SetNoiseQuality);
	defNoiseBillow.def("SetOctaveCount", &Lua::noise::BillowNoise::SetOctaveCount);
	defNoiseBillow.def("SetPersistence", &Lua::noise::BillowNoise::SetPersistence);
	defNoiseBillow.def("SetSeed", &Lua::noise::BillowNoise::SetSeed);
	defNoiseModule.scope[defNoiseBillow];

	auto defNoiseBlend = luabind::class_<NoiseBlend, NoiseBaseModule>("Blend");
	defNoiseModule.scope[defNoiseBlend];

	auto defNoiseCache = luabind::class_<NoiseCache, NoiseBaseModule>("Cache");
	defNoiseModule.scope[defNoiseCache];

	auto defNoiseCheckerboard = luabind::class_<NoiseCheckerboard, NoiseBaseModule>("Checkerboard");
	defNoiseModule.scope[defNoiseCheckerboard];

	auto defNoiseClamp = luabind::class_<NoiseClamp, NoiseBaseModule>("Clamp");
	defNoiseModule.scope[defNoiseClamp];

	auto defNoiseConst = luabind::class_<NoiseConst, NoiseBaseModule>("Const");
	defNoiseModule.scope[defNoiseConst];

	auto defNoiseCurve = luabind::class_<NoiseCurve, NoiseBaseModule>("Curve");
	defNoiseModule.scope[defNoiseCurve];

	auto defNoiseCylinders = luabind::class_<NoiseCylinders, NoiseBaseModule>("Cylinders");
	defNoiseModule.scope[defNoiseCylinders];

	auto defNoiseDisplace = luabind::class_<NoiseDisplace, NoiseBaseModule>("Displace");
	defNoiseModule.scope[defNoiseDisplace];

	auto defNoiseExponent = luabind::class_<NoiseExponent, NoiseBaseModule>("Exponent");
	defNoiseModule.scope[defNoiseExponent];

	auto defNoiseInvert = luabind::class_<NoiseInvert, NoiseBaseModule>("Invert");
	defNoiseModule.scope[defNoiseInvert];

	auto defNoiseMax = luabind::class_<NoiseMax, NoiseBaseModule>("Max");
	defNoiseModule.scope[defNoiseMax];

	auto defNoiseMin = luabind::class_<NoiseMin, NoiseBaseModule>("Min");
	defNoiseModule.scope[defNoiseMin];

	auto defNoiseMultiply = luabind::class_<NoiseMultiply, NoiseBaseModule>("Multiply");
	defNoiseModule.scope[defNoiseMultiply];

	auto defNoisePerlin = luabind::class_<NoisePerlin, NoiseBaseModule>("Perlin");
	defNoisePerlin.def("GetFrequency", &Lua::noise::PerlinNoise::GetFrequency);
	defNoisePerlin.def("GetLacunarity", &Lua::noise::PerlinNoise::GetLacunarity);
	defNoisePerlin.def("GetNoiseQuality", &Lua::noise::PerlinNoise::GetNoiseQuality);
	defNoisePerlin.def("GetOctaveCount", &Lua::noise::PerlinNoise::GetOctaveCount);
	defNoisePerlin.def("GetPersistence", &Lua::noise::PerlinNoise::GetPersistence);
	defNoisePerlin.def("GetSeed", &Lua::noise::PerlinNoise::GetSeed);
	defNoisePerlin.def("SetFrequency", &Lua::noise::PerlinNoise::SetFrequency);
	defNoisePerlin.def("SetLacunarity", &Lua::noise::PerlinNoise::SetLacunarity);
	defNoisePerlin.def("SetNoiseQuality", &Lua::noise::PerlinNoise::SetNoiseQuality);
	defNoisePerlin.def("SetOctaveCount", &Lua::noise::PerlinNoise::SetOctaveCount);
	defNoisePerlin.def("SetPersistence", &Lua::noise::PerlinNoise::SetPersistence);
	defNoisePerlin.def("SetSeed", &Lua::noise::PerlinNoise::SetSeed);
	defNoiseModule.scope[defNoisePerlin];

	auto defNoisePower = luabind::class_<NoisePower, NoiseBaseModule>("Power");
	defNoiseModule.scope[defNoisePower];

	auto defNoiseRidgedMulti = luabind::class_<NoiseRidgedMulti, NoiseBaseModule>("RidgedMulti");
	defNoiseRidgedMulti.def("GetFrequency", &Lua::noise::RidgedMultiNoise::GetFrequency);
	defNoiseRidgedMulti.def("GetLacunarity", &Lua::noise::RidgedMultiNoise::GetLacunarity);
	defNoiseRidgedMulti.def("GetNoiseQuality", &Lua::noise::RidgedMultiNoise::GetNoiseQuality);
	defNoiseRidgedMulti.def("GetOctaveCount", &Lua::noise::RidgedMultiNoise::GetOctaveCount);
	defNoiseRidgedMulti.def("GetSeed", &Lua::noise::RidgedMultiNoise::GetSeed);
	defNoiseRidgedMulti.def("SetFrequency", &Lua::noise::RidgedMultiNoise::SetFrequency);
	defNoiseRidgedMulti.def("SetLacunarity", &Lua::noise::RidgedMultiNoise::SetLacunarity);
	defNoiseRidgedMulti.def("SetNoiseQuality", &Lua::noise::RidgedMultiNoise::SetNoiseQuality);
	defNoiseRidgedMulti.def("SetOctaveCount", &Lua::noise::RidgedMultiNoise::SetOctaveCount);
	defNoiseRidgedMulti.def("SetSeed", &Lua::noise::RidgedMultiNoise::SetSeed);
	defNoiseModule.scope[defNoiseRidgedMulti];

	auto defNoiseRotatePoint = luabind::class_<NoiseRotatePoint, NoiseBaseModule>("RotatePoint");
	defNoiseModule.scope[defNoiseRotatePoint];

	auto defNoiseScaleBias = luabind::class_<NoiseScaleBias, NoiseBaseModule>("ScaleBias");
	defNoiseModule.scope[defNoiseScaleBias];

	auto noiseScalePoint = luabind::class_<NoiseScalePoint, NoiseBaseModule>("ScalePoint");
	defNoiseModule.scope[noiseScalePoint];

	auto noiseSelect = luabind::class_<NoiseSelect, NoiseBaseModule>("Select");
	defNoiseModule.scope[noiseSelect];

	auto noiseSpheres = luabind::class_<NoiseSpheres, NoiseBaseModule>("Spheres");
	defNoiseModule.scope[noiseSpheres];

	auto noiseTerrace = luabind::class_<NoiseTerrace, NoiseBaseModule>("Terrace");
	defNoiseModule.scope[noiseTerrace];

	auto noiseTransplatePoint = luabind::class_<NoiseTranslatePoint, NoiseBaseModule>("TranslatePoint");
	defNoiseModule.scope[noiseTransplatePoint];

	auto noiseTurbulance = luabind::class_<NoiseTurbulance, NoiseBaseModule>("Turbulance");
	defNoiseModule.scope[noiseTurbulance];

	auto noiseVoroni = luabind::class_<NoiseVoronoi, NoiseBaseModule>("Voronoi");
	noiseVoroni.def("GetDisplacement", &Lua::noise::VoronoiNoise::GetDisplacement);
	noiseVoroni.def("GetFrequency", &Lua::noise::VoronoiNoise::GetFrequency);
	noiseVoroni.def("GetSeed", &Lua::noise::VoronoiNoise::GetSeed);
	noiseVoroni.def("SetDisplacement", &Lua::noise::VoronoiNoise::SetDisplacement);
	noiseVoroni.def("SetFrequency", &Lua::noise::VoronoiNoise::SetFrequency);
	noiseVoroni.def("SetSeed", &Lua::noise::VoronoiNoise::SetSeed);
	defNoiseModule.scope[noiseVoroni];

	modMath[defNoiseModule];

	auto noiseMap = luabind::class_<noise::utils::NoiseMap>("NoiseMap");
	noiseMap.def("GetValue", &Lua::noise::NoiseMap::GetValue);
	noiseMap.def("GetHeight", &Lua::noise::NoiseMap::GetHeight);
	noiseMap.def("GetWidth", &Lua::noise::NoiseMap::GetWidth);
	modMath[noiseMap];
	//

	auto defVectori = pragma::lua::register_class<Vector3i>(lua.GetState(), "Vectori");
	defVectori->def(luabind::constructor<>());
	defVectori->def(luabind::constructor<int32_t, int32_t, int32_t>());
	defVectori->def(-luabind::const_self);
	defVectori->def_readwrite("x", &Vector3i::x);
	defVectori->def_readwrite("y", &Vector3i::y);
	defVectori->def_readwrite("z", &Vector3i::z);
	defVectori->def(luabind::const_self / int32_t());
	defVectori->def(luabind::const_self * int32_t());
	defVectori->def(luabind::const_self + luabind::const_self);
	defVectori->def(luabind::const_self - luabind::const_self);
	defVectori->def(luabind::const_self == luabind::const_self);
	defVectori->def(int32_t() / luabind::const_self);
	defVectori->def(int32_t() * luabind::const_self);
	defVectori->def("Copy", &Lua::Vectori::Copy);
	defVectori->def("Get", static_cast<void (*)(lua_State *, const Vector3i &, uint32_t)>([](lua_State *l, const Vector3i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVectori];
	register_string_to_vector_type_constructor<Vector3i>(lua.GetState());

	auto defVector2i = pragma::lua::register_class<Vector2i>(lua.GetState(), "Vector2i");
	defVector2i->def(luabind::constructor<>());
	defVector2i->def(luabind::constructor<int32_t, int32_t>());
	defVector2i->def(-luabind::const_self);
	defVector2i->def_readwrite("x", &Vector2i::x);
	defVector2i->def_readwrite("y", &Vector2i::y);
	defVector2i->def(luabind::const_self / int32_t());
	defVector2i->def(luabind::const_self * int32_t());
	defVector2i->def(luabind::const_self + luabind::const_self);
	defVector2i->def(luabind::const_self - luabind::const_self);
	defVector2i->def(luabind::const_self == luabind::const_self);
	defVector2i->def(int32_t() / luabind::const_self);
	defVector2i->def(int32_t() * luabind::const_self);
	defVector2i->def("Copy", &Lua::Vector2i::Copy);
	defVector2i->def("Get", static_cast<void (*)(lua_State *, const Vector2i &, uint32_t)>([](lua_State *l, const Vector2i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVector2i];
	register_string_to_vector_type_constructor<Vector2i>(lua.GetState());

	auto defVector4i = pragma::lua::register_class<Vector4i>(lua.GetState(), "Vector4i");
	defVector4i->def(luabind::constructor<>());
	defVector4i->def(luabind::constructor<int32_t, int32_t, int32_t, int32_t>());
	defVector4i->def(-luabind::const_self);
	defVector4i->def_readwrite("w", &Vector4i::w);
	defVector4i->def_readwrite("x", &Vector4i::x);
	defVector4i->def_readwrite("y", &Vector4i::y);
	defVector4i->def_readwrite("z", &Vector4i::z);
	defVector4i->def(luabind::const_self / int32_t());
	defVector4i->def(luabind::const_self * int32_t());
	defVector4i->def(luabind::const_self + luabind::const_self);
	defVector4i->def(luabind::const_self - luabind::const_self);
	defVector4i->def(luabind::const_self == luabind::const_self);
	defVector4i->def(luabind::const_self * luabind::const_self);
	defVector4i->def(int32_t() / luabind::const_self);
	defVector4i->def(int32_t() * luabind::const_self);
	defVector4i->def("Copy", &Lua::Vector4i::Copy);
	defVector4i->def("Get", static_cast<void (*)(lua_State *, const Vector4i &, uint32_t)>([](lua_State *l, const Vector4i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVector4i];
	register_string_to_vector_type_constructor<Vector4i>(lua.GetState());

	auto defVector = pragma::lua::register_class<Vector3>(lua.GetState(), "Vector");
	defVector->def(luabind::constructor<>());
	defVector->def(luabind::constructor<float, float, float>());
	defVector->def(luabind::constructor<const Vector2 &, float>());
	defVector->def(-luabind::const_self);
	defVector->def_readwrite("x", &Vector3::x);
	defVector->def_readwrite("y", &Vector3::y);
	defVector->def_readwrite("z", &Vector3::z);
	defVector->def_readwrite("r", &Vector3::r);
	defVector->def_readwrite("g", &Vector3::g);
	defVector->def_readwrite("b", &Vector3::b);
	defVector->def(luabind::const_self / float());
	defVector->def(luabind::const_self * float());
	defVector->def(luabind::const_self * Vector3());
	defVector->def(luabind::const_self + luabind::const_self);
	defVector->def(luabind::const_self - luabind::const_self);
	defVector->def(luabind::const_self == luabind::const_self);
	defVector->def(luabind::const_self * Quat());
	//defVector->def(luabind::const_self *umath::Transform());
	//defVector->def(luabind::const_self *umath::ScaledTransform());
	defVector->def("Mul", static_cast<void (*)(lua_State *, const Vector3 &, const umath::Transform &)>([](lua_State *l, const Vector3 &a, const umath::Transform &b) { Lua::Push<Vector3>(l, a * b); }));
	defVector->def("Mul", static_cast<void (*)(lua_State *, const Vector3 &, const umath::ScaledTransform &)>([](lua_State *l, const Vector3 &a, const umath::ScaledTransform &b) { Lua::Push<Vector3>(l, a * b); }));
	defVector->def(float() / luabind::const_self);
	defVector->def(float() * luabind::const_self);
	defVector->def(Quat() * luabind::const_self);
	defVector->def("GetNormal", static_cast<Vector3 (*)(Vector3)>(&uvec::get_normal));
	defVector->def("Normalize", &Lua::Vector::Normalize);
	defVector->def("ToEulerAngles", static_cast<EulerAngles (*)(const Vector3 &)>(uvec::to_angle));
	defVector->def("Length", uvec::length);
	defVector->def("LengthSqr", uvec::length_sqr);
	defVector->def("Distance", uvec::distance);
	defVector->def("DistanceSqr", uvec::distance_sqr);
	defVector->def("PlanarDistance", uvec::planar_distance);
	defVector->def("PlanarDistanceSqr", uvec::planar_distance_sqr);
	defVector->def("Cross", uvec::cross);
	defVector->def("DotProduct", uvec::dot);
	defVector->def("GetRotation", uvec::get_rotation);
	defVector->def("Rotate", static_cast<void (*)(lua_State *, Vector3 &, const EulerAngles &)>(&Lua::Vector::Rotate));
	defVector->def("Rotate", static_cast<void (*)(lua_State *, Vector3 &, const Vector3 &, float)>(&Lua::Vector::Rotate));
	defVector->def("Rotate", static_cast<void (*)(lua_State *, Vector3 &, const Quat &)>(&Lua::Vector::Rotate));
	defVector->def("RotateAround", &Lua::Vector::RotateAround);
	defVector->def("Lerp", &Lua::Vector::Lerp);
	defVector->def("Reflect", &uvec::reflect);
	defVector->def("Equals",
	  static_cast<void (*)(lua_State *, const Vector3 &, const Vector3 &, float)>([](lua_State *l, const Vector3 &a, const Vector3 &b, float epsilon) { Lua::PushBool(l, umath::abs(a.x - b.x) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.z - b.z) <= epsilon); }));
	defVector->def("Equals", static_cast<void (*)(lua_State *, const Vector3 &, const Vector3 &)>([](lua_State *l, const Vector3 &a, const Vector3 &b) {
		float epsilon = 0.001f;
		Lua::PushBool(l, umath::abs(a.x - b.x) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.z - b.z) <= epsilon);
	}));
	defVector->def("GetAngle", static_cast<float (*)(lua_State *, const Vector3 &, const Vector3 &)>([](lua_State *l, const Vector3 &a, const Vector3 &b) -> float { return umath::deg_to_rad(uvec::get_angle(a, b)); }));
	defVector->def("Slerp", static_cast<void (*)(lua_State *, const Vector3 &, const Vector3 &, float)>([](lua_State *l, const Vector3 &a, const Vector3 &b, float factor) {
		auto result = glm::slerp(a, b, factor);
		Lua::Push<Vector3>(l, result);
	}));
	defVector->def("Copy", &Lua::Vector::Copy);
	defVector->def("Set", static_cast<void (*)(lua_State *, Vector3 &, const Vector3 &)>(&Lua::Vector::Set));
	defVector->def("Set", static_cast<void (*)(lua_State *, Vector3 &, float, float, float)>(&Lua::Vector::Set));
	defVector->def("Set", static_cast<void (*)(lua_State *, Vector3 &, uint32_t, float)>([](lua_State *l, Vector3 &v, uint32_t idx, float val) { v[idx] = val; }));
	defVector->def("Get", static_cast<void (*)(lua_State *, const Vector3 &, uint32_t)>([](lua_State *l, const Vector3 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector->def("GetYaw", &uvec::get_yaw);
	defVector->def("GetPitch", &uvec::get_pitch);
	defVector->def("ToMatrix", &Lua::Vector::ToMatrix);
	defVector->def("SnapToGrid", static_cast<void (*)(lua_State *, Vector3 &)>(&Lua::Vector::SnapToGrid));
	defVector->def("SnapToGrid", static_cast<void (*)(lua_State *, Vector3 &, UInt32)>(&Lua::Vector::SnapToGrid));
	defVector->def("Project", uvec::project);
	defVector->def("ProjectToPlane", uvec::project_to_plane);
	defVector->def("GetPerpendicular", uvec::get_perpendicular);
	defVector->def("OuterProduct", &uvec::calc_outer_product);
	defVector->def("ToScreenUv", &umat::to_screen_uv);
	modMath[*defVector];
	register_string_to_vector_type_constructor<Vector3>(lua.GetState());

	auto defVector2 = pragma::lua::register_class<Vector2>(lua.GetState(), "Vector2");
	defVector2->def(luabind::constructor<>());
	defVector2->def(luabind::constructor<float, float>());
	defVector2->def(-luabind::const_self);
	defVector2->def_readwrite("x", &Vector2::x);
	defVector2->def_readwrite("y", &Vector2::y);
	defVector2->def(luabind::const_self / float());
	defVector2->def(luabind::const_self * float());
	defVector2->def(luabind::const_self * Vector2());
	defVector2->def(luabind::const_self + luabind::const_self);
	defVector2->def(luabind::const_self - luabind::const_self);
	defVector2->def(luabind::const_self == luabind::const_self);
	defVector2->def(float() / luabind::const_self);
	defVector2->def(float() * luabind::const_self);
	defVector2->def("GetNormal", &Lua::Vector2::GetNormal);
	defVector2->def("Normalize", &Lua::Vector2::Normalize);
	defVector2->def("Length", &Lua::Vector2::Length);
	defVector2->def("LengthSqr", &Lua::Vector2::LengthSqr);
	defVector2->def("Distance", &Lua::Vector2::Distance);
	defVector2->def("DistanceSqr", &Lua::Vector2::DistanceSqr);
	defVector2->def("Cross", &Lua::Vector2::Cross);
	defVector2->def("DotProduct", &Lua::Vector2::DotProduct);
	defVector2->def("Rotate", &Lua::Vector2::Rotate);
	defVector2->def("RotateAround", &Lua::Vector2::RotateAround);
	defVector2->def("Lerp", &Lua::Vector2::Lerp);
	defVector2->def("Copy", &Lua::Vector2::Copy);
	defVector2->def("GetAngle", static_cast<float (*)(lua_State *, const Vector2 &, const Vector2 &)>([](lua_State *l, const Vector2 &a, const Vector2 &b) -> float { return umath::deg_to_rad(uvec::get_angle(Vector3 {a, 0.f}, Vector3 {b, 0.f})); }));
	defVector2->def("Set", static_cast<void (*)(lua_State *, Vector2 &, const Vector2 &)>(&Lua::Vector2::Set));
	defVector2->def("Set", static_cast<void (*)(lua_State *, Vector2 &, float, float)>(&Lua::Vector2::Set));
	defVector2->def("Get", static_cast<void (*)(lua_State *, const Vector2 &, uint32_t)>([](lua_State *l, const Vector2 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector2->def("Project", &Lua::Vector2::Project);
	modMath[*defVector2];
	register_string_to_vector_type_constructor<Vector2>(lua.GetState());

	auto defVector4 = pragma::lua::register_class<Vector4>(lua.GetState(), "Vector4");
	defVector4->def(luabind::constructor<>());
	defVector4->def(luabind::constructor<float, float, float, float>());
	defVector4->def(luabind::constructor<const Vector3 &, float>());
	defVector4->def(-luabind::const_self);
	defVector4->def_readwrite("w", &Vector4::w);
	defVector4->def_readwrite("x", &Vector4::x);
	defVector4->def_readwrite("y", &Vector4::y);
	defVector4->def_readwrite("z", &Vector4::z);
	defVector4->def_readwrite("r", &Vector4::r);
	defVector4->def_readwrite("g", &Vector4::g);
	defVector4->def_readwrite("b", &Vector4::b);
	defVector4->def_readwrite("a", &Vector4::a);
	defVector4->def(luabind::const_self / float());
	defVector4->def(luabind::const_self * float());
	defVector4->def(luabind::const_self * Vector4());
	defVector4->def(luabind::const_self + luabind::const_self);
	defVector4->def(luabind::const_self - luabind::const_self);
	defVector4->def(luabind::const_self == luabind::const_self);
	defVector4->def(luabind::const_self * Mat4());
	defVector4->def(float() / luabind::const_self);
	defVector4->def(float() * luabind::const_self);
	defVector4->def("GetNormal", &Lua::Vector4::GetNormal);
	defVector4->def("Normalize", &Lua::Vector4::Normalize);
	defVector4->def("Length", &Lua::Vector4::Length);
	defVector4->def("LengthSqr", &Lua::Vector4::LengthSqr);
	defVector4->def("Distance", &Lua::Vector4::Distance);
	defVector4->def("DistanceSqr", &Lua::Vector4::DistanceSqr);
	defVector4->def("DotProduct", &Lua::Vector4::DotProduct);
	defVector4->def("Lerp", &Lua::Vector4::Lerp);
	defVector4->def("Copy", &Lua::Vector4::Copy);
	defVector4->def("Set", static_cast<void (*)(lua_State *, Vector4 &, const Vector4 &)>(&Lua::Vector4::Set));
	defVector4->def("Set", static_cast<void (*)(lua_State *, Vector4 &, float, float, float, float)>(&Lua::Vector4::Set));
	defVector4->def("Get", static_cast<void (*)(lua_State *, const Vector4 &, uint32_t)>([](lua_State *l, const Vector4 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector4->def("Project", &Lua::Vector4::Project);
	modMath[*defVector4];
	register_string_to_vector_type_constructor<Vector4>(lua.GetState());

	auto defEulerAngles = pragma::lua::register_class<EulerAngles>(lua.GetState(), "EulerAngles");
	defEulerAngles->def(luabind::constructor<>());
	defEulerAngles->def(luabind::constructor<float, float, float>());
	defEulerAngles->def(luabind::constructor<const EulerAngles &>());
	defEulerAngles->def(luabind::constructor<const Mat4 &>());
	defEulerAngles->def(luabind::constructor<const Vector3 &>());
	defEulerAngles->def(luabind::constructor<const Vector3 &, const Vector3 &>());
	defEulerAngles->def(luabind::constructor<const Quat &>());
	defEulerAngles->def(luabind::constructor<const std::string &>());
	defEulerAngles->def(-luabind::const_self);
	defEulerAngles->def_readwrite("p", &EulerAngles::p);
	defEulerAngles->def_readwrite("y", &EulerAngles::y);
	defEulerAngles->def_readwrite("r", &EulerAngles::r);
	defEulerAngles->def(luabind::const_self / float());
	defEulerAngles->def(luabind::const_self * float());
	defEulerAngles->def(luabind::const_self + luabind::const_self);
	defEulerAngles->def(luabind::const_self - luabind::const_self);
	defEulerAngles->def(luabind::const_self == luabind::const_self);
	defEulerAngles->def(float() * luabind::const_self);
	defEulerAngles->def("GetForward", &EulerAngles::Forward);
	defEulerAngles->def("GetRight", &EulerAngles::Right);
	defEulerAngles->def("GetUp", static_cast<Vector3 (EulerAngles::*)() const>(&EulerAngles::Up));
	defEulerAngles->def("GetOrientation", &Lua::Angle::Orientation);
	defEulerAngles->def("Normalize", static_cast<void (EulerAngles::*)()>(&EulerAngles::Normalize));
	defEulerAngles->def("Normalize", static_cast<void (EulerAngles::*)(float)>(&EulerAngles::Normalize));
	defEulerAngles->def("ToMatrix", &EulerAngles::ToMatrix);
	defEulerAngles->def("Copy", &Lua::Angle::Copy);
	defEulerAngles->def("Equals",
	  static_cast<bool (*)(lua_State *, const EulerAngles &, const EulerAngles &, float)>([](lua_State *l, const EulerAngles &a, const EulerAngles &b, float epsilon) { return umath::abs(a.p - b.p) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.r - b.r) <= epsilon; }));
	defEulerAngles->def("Equals", static_cast<bool (*)(lua_State *, const EulerAngles &, const EulerAngles &)>([](lua_State *l, const EulerAngles &a, const EulerAngles &b) {
		float epsilon = 0.001f;
		return umath::abs(a.p - b.p) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.r - b.r) <= epsilon;
	}));
	defEulerAngles->def("ToQuaternion", Lua::Angle::ToQuaternion);
	defEulerAngles->def("ToQuaternion", static_cast<void (*)(lua_State *, const EulerAngles &)>([](lua_State *l, const EulerAngles &ang) { Lua::Angle::ToQuaternion(l, ang, umath::to_integral(pragma::RotationOrder::YXZ)); }));
	defEulerAngles->def("Set", static_cast<void (EulerAngles::*)(const EulerAngles &)>(&EulerAngles::Set));
	defEulerAngles->def("Set", &Lua::Angle::Set);
	defEulerAngles->def("Set", static_cast<void (*)(lua_State *, EulerAngles &, uint32_t, float value)>([](lua_State *l, EulerAngles &ang, uint32_t idx, float value) { ang[idx] = value; }));
	defEulerAngles->def("Get", static_cast<float (*)(lua_State *, const EulerAngles &, uint32_t)>([](lua_State *l, const EulerAngles &ang, uint32_t idx) { return ang[idx]; }));
	modMath[*defEulerAngles];

	auto defQuat = pragma::lua::register_class<Quat>(lua.GetState(), "Quaternion");
	defQuat->def(luabind::constructor<float, float, float, float>());
	defQuat->def(luabind::constructor<const Quat &>());
	defQuat->def_readwrite("w", &Quat::w);
	defQuat->def_readwrite("x", &Quat::x);
	defQuat->def_readwrite("y", &Quat::y);
	defQuat->def_readwrite("z", &Quat::z);
	defQuat->def(-luabind::const_self);
	defQuat->def(luabind::const_self / float());
	defQuat->def(luabind::const_self * float());
	defQuat->def(luabind::const_self * Vector3());
	defQuat->def(luabind::const_self * luabind::const_self);
	defQuat->def(luabind::const_self == luabind::const_self);
	//defQuat->def(luabind::const_self *umath::Transform());
	//defQuat->def(luabind::const_self *umath::ScaledTransform());
	defQuat->def("Mul", static_cast<Quat (*)(lua_State *, const Quat &, const umath::Transform &)>([](lua_State *l, const Quat &a, const umath::Transform &b) { return a * b; }));
	defQuat->def("Mul", static_cast<Quat (*)(lua_State *, const Quat &, const umath::ScaledTransform &)>([](lua_State *l, const Quat &a, const umath::ScaledTransform &b) { return a * b; }));
	defQuat->def(float() * luabind::const_self);
	defQuat->def("MirrorAxis", &uquat::mirror_on_axis);
	defQuat->def("GetForward", &uquat::forward);
	defQuat->def("GetRight", &uquat::right);
	defQuat->def("GetUp", &uquat::up);
	defQuat->def("GetOrientation", &Lua::Quaternion::GetOrientation);
	defQuat->def("DotProduct", &uquat::dot_product);
	defQuat->def("Inverse", &uquat::inverse);
	defQuat->def("GetInverse", &uquat::get_inverse);
	defQuat->def("Length", &uquat::length);
	defQuat->def("Normalize", &uquat::normalize);
	defQuat->def("GetNormal", &uquat::get_normal);
	defQuat->def("Copy", &Lua::Quaternion::Copy);
	defQuat->def("ToMatrix", static_cast<Mat4 (*)(const Quat &)>(&glm::toMat4));
	defQuat->def("ToMatrix3", static_cast<Mat3 (*)(const Quat &)>(&glm::toMat3));
	defQuat->def("Lerp", &uquat::lerp);
	defQuat->def("Slerp", &uquat::slerp);
	defQuat->def("ToEulerAngles", Lua::Quaternion::ToEulerAngles);
	defQuat->def("ToEulerAngles", static_cast<EulerAngles (*)(lua_State *, Quat &)>([](lua_State *l, Quat &rot) { return Lua::Quaternion::ToEulerAngles(l, rot, umath::to_integral(pragma::RotationOrder::YXZ)); }));
	defQuat->def("ToAxisAngle", &Lua::Quaternion::ToAxisAngle);
	defQuat->def(
	  "GetAxisVector", +[](const Quat &rot, pragma::SignedAxis axis) {
		  auto dir = pragma::axis_to_vector(axis);
		  uvec::rotate(&dir, rot);
		  return dir;
	  });
	defQuat->def("Set", &Lua::Quaternion::Set);
	defQuat->def("Set", static_cast<void (*)(Quat &, const Quat &)>([](Quat &rot, const Quat &rotNew) { rot = rotNew; }));
	defQuat->def("Set", static_cast<void (*)(Quat &, uint32_t, float)>([](Quat &rot, uint32_t idx, float value) {
		constexpr std::array<uint8_t, 4> quatIndices = {3, 0, 1, 2};
		rot[quatIndices.at(idx)] = value;
	}));
	defQuat->def("Get", static_cast<void (*)(lua_State *, Quat &, uint32_t)>([](lua_State *l, Quat &rot, uint32_t idx) {
		constexpr std::array<uint8_t, 4> quatIndices = {3, 0, 1, 2};
		Lua::PushNumber(l, rot[quatIndices.at(idx)]);
	}));
	defQuat->def("RotateX", static_cast<void (*)(Quat &, float)>(&uquat::rotate_x));
	defQuat->def("RotateY", static_cast<void (*)(Quat &, float)>(&uquat::rotate_y));
	defQuat->def("RotateZ", static_cast<void (*)(Quat &, float)>(&uquat::rotate_z));
	defQuat->def("Rotate", static_cast<void (*)(Quat &, const Vector3 &, float)>(&uquat::rotate));
	defQuat->def("Rotate", static_cast<void (*)(Quat &, const EulerAngles &)>(&uquat::rotate));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, ::Vector2> (*)(lua_State *, const Quat &, const Vector3 &, const Vector3 &, const ::Vector2 &, const ::Vector2 *, const ::Vector2 *, const Quat *, const EulerAngles *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, ::Vector2> (*)(lua_State *, const Quat &, const Vector3 &, const Vector3 &, const ::Vector2 &, const ::Vector2 *, const ::Vector2 *, const Quat *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, ::Vector2> (*)(lua_State *, const Quat &, const Vector3 &, const Vector3 &, const ::Vector2 &, const ::Vector2 *, const ::Vector2 *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, ::Vector2> (*)(lua_State *, const Quat &, const Vector3 &, const Vector3 &, const ::Vector2 &, const ::Vector2 *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, ::Vector2> (*)(lua_State *, const Quat &, const Vector3 &, const Vector3 &, const ::Vector2 &)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ClampRotation", static_cast<Quat (*)(lua_State *, Quat &, const EulerAngles &, const EulerAngles &)>([](lua_State *l, Quat &rot, const EulerAngles &minBounds, const EulerAngles &maxBounds) -> Quat { return uquat::clamp_rotation(rot, minBounds, maxBounds); }));
	defQuat->def("ClampRotation", static_cast<Quat (*)(lua_State *, Quat &, const EulerAngles &)>([](lua_State *l, Quat &rot, const EulerAngles &bounds) -> Quat { return uquat::clamp_rotation(rot, -bounds, bounds); }));
	defQuat->def("Distance", &uquat::distance);
	defQuat->def("GetConjugate", static_cast<Quat (*)(const Quat &)>(&glm::conjugate));
	defQuat->def("AlignToAxis", &uquat::align_rotation_to_axis);
	defQuat->def(
	  "Equals", +[](const Quat &a, const Quat &b, float epsilon) { return umath::abs(a.x - b.x) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.z - b.z) <= epsilon && umath::abs(a.w - b.w) <= epsilon; });
	defQuat->def(
	  "Equals", +[](const Quat &a, const Quat &b) {
		  float epsilon = 0.001f;
		  return umath::abs(a.x - b.x) <= epsilon && umath::abs(a.y - b.y) <= epsilon && umath::abs(a.z - b.z) <= epsilon && umath::abs(a.w - b.w) <= epsilon;
	  });
	modMath[*defQuat];
	pragma::lua::define_custom_constructor<Quat, &uquat::identity>(lua.GetState());
	pragma::lua::define_custom_constructor<Quat, static_cast<Quat (*)(const Vector3 &, float)>(&uquat::create), const Vector3 &, float>(lua.GetState());
	pragma::lua::define_custom_constructor<Quat,
	  [](const Vector3 &a, const Vector3 &b, const Vector3 &c) -> Quat {
		  auto m = umat::create_from_axes(a, b, c);
		  return Quat(m);
	  },
	  const Vector3 &, const Vector3 &, const Vector3 &>(lua.GetState());
	pragma::lua::define_custom_constructor<Quat, &uquat::create_look_rotation, const Vector3 &, const Vector3 &>(lua.GetState());

	auto _G = luabind::globals(lua.GetState());
	_G["Vector2i"] = _G["math"]["Vector2i"];
	_G["Vector"] = _G["math"]["Vector"];
	_G["Vector2"] = _G["math"]["Vector2"];
	_G["Vector4"] = _G["math"]["Vector4"];
	_G["EulerAngles"] = _G["math"]["EulerAngles"];
	_G["Quaternion"] = _G["math"]["Quaternion"];

	RegisterLuaMatrices(lua);
	RegisterIk(lua);
	//modelMeshClassDef.scope[luabind::def("Create",&Lua::ModelMesh::Client::Create)];
}

static bool operator==(const EntityHandle &v, const LEntityProperty &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LEntityProperty &v)
{
	if((*v)->valid())
		const_cast<BaseEntity *>((*v)->get())->print(str);
	else
		str << "NULL";
	return str;
}
void Lua::Property::push(lua_State *l, pragma::EntityProperty &prop) { Lua::Property::push_property<LEntityPropertyWrapper>(l, prop); }
namespace pragma::lua {
	void register_thread_pool(lua_State *l, luabind::module_ &modUtil);
};

void Game::RegisterLuaClasses()
{
	NetworkState::RegisterSharedLuaClasses(GetLuaInterface());

	// Entity
	auto &modUtil = GetLuaInterface().RegisterLibrary("util");
	pragma::lua::register_thread_pool(GetLuaState(), modUtil);
	auto entDef = luabind::class_<LEntityProperty, LBasePropertyWrapper>("EntityProperty");
	//Lua::Property::add_generic_methods<LEntityProperty,EntityHandle,luabind::class_<LEntityProperty,LBasePropertyWrapper>>(entDef);
	entDef.def(luabind::constructor<>());
	entDef.def(luabind::constructor<EntityHandle>());
	entDef.def(luabind::tostring(luabind::const_self));
	entDef.def("Link", static_cast<void (*)(lua_State *, LEntityProperty &, LEntityProperty &)>(&Lua::Property::link<LEntityProperty, EntityHandle>));
	modUtil[entDef];

	auto defSplashDamageInfo = luabind::class_<util::SplashDamageInfo>("SplashDamageInfo");
	defSplashDamageInfo.def(luabind::constructor<>());
	defSplashDamageInfo.def_readwrite("origin", &util::SplashDamageInfo::origin);
	defSplashDamageInfo.def_readwrite("radius", &util::SplashDamageInfo::radius);
	defSplashDamageInfo.def_readwrite("damageInfo", &util::SplashDamageInfo::damageInfo);
	defSplashDamageInfo.def("SetCone", static_cast<void (*)(lua_State *, util::SplashDamageInfo &, const Vector3 &, float)>([](lua_State *l, util::SplashDamageInfo &splashDamageInfo, const Vector3 &coneDirection, float coneAngle) { splashDamageInfo.cone = {{coneDirection, coneAngle}}; }));
	defSplashDamageInfo.def("SetCallback", static_cast<void (*)(lua_State *, util::SplashDamageInfo &, luabind::object)>([](lua_State *l, util::SplashDamageInfo &splashDamageInfo, luabind::object oCallback) {
		Lua::CheckFunction(l, 2);
		splashDamageInfo.callback = [l, oCallback](BaseEntity *ent, DamageInfo &dmgInfo) -> bool {
			auto r = Lua::CallFunction(
			  l,
			  [ent, &dmgInfo, &oCallback](lua_State *l) -> Lua::StatusCode {
				  oCallback.push(l);
				  if(ent != nullptr)
					  ent->GetLuaObject().push(l);
				  else
					  Lua::PushNil(l);
				  Lua::Push<DamageInfo *>(l, &dmgInfo);
				  return Lua::StatusCode::Ok;
			  },
			  1);
			if(r == Lua::StatusCode::Ok) {
				auto res = Lua::IsSet(l, -1) && Lua::CheckBool(l, -1);
				Lua::Pop(l);
				return res;
			}
			return false;
		};
	}));
	modUtil[defSplashDamageInfo];

	auto &modMath = m_lua->RegisterLibrary("math");

	Lua::RegisterLibraryEnums(m_lua->GetState(), "math",
	  {
	    {"COORDINATE_SPACE_LOCAL", umath::to_integral(umath::CoordinateSpace::Local)},
	    {"COORDINATE_SPACE_WORLD", umath::to_integral(umath::CoordinateSpace::World)},
	    {"COORDINATE_SPACE_OBJECT", umath::to_integral(umath::CoordinateSpace::Object)},
	    {"COORDINATE_SPACE_VIEW", umath::to_integral(umath::CoordinateSpace::View)},
	    {"COORDINATE_SPACE_SCREEN", umath::to_integral(umath::CoordinateSpace::Screen)},
	  });
	modMath[luabind::def(
	  "coordinate_space_to_string", +[](umath::CoordinateSpace space) -> std::string { return std::string {magic_enum::enum_name(space)}; })];
	modMath[luabind::def(
	  "string_to_coordinate_space", +[](const std::string &space) -> std::optional<umath::CoordinateSpace> { return magic_enum::enum_cast<umath::CoordinateSpace>(space); })];

	auto defPlane = luabind::class_<umath::Plane>("Plane");
	defPlane.def(luabind::constructor<Vector3, Vector3, Vector3>());
	defPlane.def(luabind::constructor<Vector3, Vector3>());
	defPlane.def(luabind::constructor<Vector3, double>());
	defPlane.def(
	  "__tostring", +[](umath::Plane &plane) -> std::string {
		  std::stringstream ss;
		  ss << "Plane[" << plane.GetNormal() << "][" << plane.GetDistance() << "]";
		  return ss.str();
	  });
	defPlane.def("Copy", static_cast<void (*)(lua_State *, umath::Plane &)>([](lua_State *l, umath::Plane &plane) { Lua::Push<umath::Plane>(l, umath::Plane {plane}); }));
	defPlane.def("SetNormal", &umath::Plane::SetNormal);
	defPlane.def("SetDistance", &umath::Plane::SetDistance);
	defPlane.def("GetNormal", static_cast<const Vector3 &(umath::Plane::*)() const>(&umath::Plane::GetNormal), luabind::copy_policy<0> {});
	defPlane.def("GetPos", static_cast<Vector3 (*)(const umath::Plane &)>([](const umath::Plane &plane) { return plane.GetPos(); }));
	defPlane.def("GetDistance", static_cast<double (umath::Plane::*)() const>(&umath::Plane::GetDistance));
	defPlane.def("GetDistance", static_cast<float (umath::Plane::*)(const Vector3 &) const>(&umath::Plane::GetDistance));
	defPlane.def("MoveToPos", static_cast<void (*)(umath::Plane &, const Vector3 &pos)>([](umath::Plane &plane, const Vector3 &pos) { plane.MoveToPos(pos); }));
	defPlane.def("Rotate", &umath::Plane::Rotate);
	defPlane.def("GetCenterPos", static_cast<Vector3 (*)(const umath::Plane &)>([](const umath::Plane &plane) { return plane.GetCenterPos(); }));
	defPlane.def("Transform", static_cast<void (*)(lua_State *, umath::Plane &, const Mat4 &)>([](lua_State *l, umath::Plane &plane, const Mat4 &transform) {
		const auto &n = plane.GetNormal();
		auto p = n * static_cast<float>(plane.GetDistance());
		auto n0 = uvec::get_perpendicular(n);
		uvec::normalize(&n0);
		auto n1 = uvec::cross(n, n0);
		uvec::normalize(&n1);
		auto p04 = transform * Vector4 {p.x, p.y, p.z, 1.f};
		auto p1 = p + n0 * 10.f;
		auto p14 = transform * Vector4 {p1.x, p1.y, p1.z, 1.f};
		auto p2 = p + n1 * 10.f;
		auto p24 = transform * Vector4 {p2.x, p2.y, p2.z, 1.f};
		plane = umath::Plane {Vector3 {p04.x, p04.y, p04.z}, Vector3 {p14.x, p14.y, p14.z}, Vector3 {p24.x, p24.y, p24.z}};
	}));
	modMath[defPlane];
}

LuaEntityIterator Lua::ents::create_lua_entity_iterator(lua_State *l, const tb<LuaEntityIteratorFilterBase> &filterTable, EntityIterator::FilterFlags filterFlags)
{
	auto r = LuaEntityIterator {l, filterFlags};
	if(filterTable) {
		filterTable.push(l);
		luabind::detail::stack_pop sp {l, 1};

		auto t = Lua::GetStackTop(l);
		Lua::CheckTable(l, t);
		auto numFilters = Lua::GetObjectLength(l, t);
		for(auto i = decltype(numFilters) {0u}; i < numFilters; ++i) {
			Lua::PushInt(l, i + 1u);
			Lua::GetTableValue(l, t);

			auto *filter = Lua::CheckEntityIteratorFilter(l, -1);
			r.AttachFilter(*filter);

			Lua::Pop(l, 1);
		}
	}
	return r;
}

LuaEntityComponentIterator Lua::ents::create_lua_entity_component_iterator(lua_State *l, pragma::ComponentId componentId, const tb<LuaEntityIteratorFilterBase> &filterTable, EntityIterator::FilterFlags filterFlags)
{
	auto r = LuaEntityComponentIterator {l, componentId, filterFlags};
	if(filterTable) {
		filterTable.push(l);
		luabind::detail::stack_pop sp {l, 1};

		auto t = Lua::GetStackTop(l);
		Lua::CheckTable(l, t);
		auto numFilters = Lua::GetObjectLength(l, t);
		for(auto i = decltype(numFilters) {0u}; i < numFilters; ++i) {
			Lua::PushInt(l, i + 1u);
			Lua::GetTableValue(l, t);

			auto *filter = Lua::CheckEntityIteratorFilter(l, -1);
			r.AttachFilter(*filter);

			Lua::Pop(l, 1);
		}
	}
	return r;
}

namespace Lua {
	void register_bullet_info(luabind::module_ &gameMod);
};

struct LuaEntityIteratorFilterFunction : public LuaEntityIteratorFilterBase {
	LuaEntityIteratorFilterFunction(Lua::func<bool> oFunc);
	virtual void Attach(EntityIterator &iterator) override;
  private:
	luabind::object m_function;
};

LuaEntityIteratorFilterFunction::LuaEntityIteratorFilterFunction(Lua::func<bool> oFunc) : m_function {oFunc} {}
void LuaEntityIteratorFilterFunction::Attach(EntityIterator &iterator)
{
	auto *data = iterator.GetIteratorData();
	auto *components = (data && data->entities && typeid(*data->entities) == typeid(ComponentContainer)) ? static_cast<ComponentContainer *>(data->entities.get()) : nullptr;
	auto *l = m_function.interpreter();
	iterator.AttachFilter<EntityIteratorFilterUser>([this, l, components](BaseEntity &ent, std::size_t index) -> bool {
		auto r = Lua::CallFunction(
		  l,
		  [this, &ent, index, components](lua_State *l) -> Lua::StatusCode {
			  m_function.push(l);
			  ent.GetLuaObject().push(l);
			  if(components)
				  components->components[index]->GetLuaObject().push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(r == Lua::StatusCode::Ok) {
			auto res = Lua::CheckBool(l, -1);
			Lua::Pop(l, 1);
			return res;
		}
		return false;
	});
}

/////////

static std::optional<LuaEntityIterator> s_entIterator {};           // HACK: This is a workaround for a bug in luabind, which causes errors when compiled with gcc.
static std::optional<LuaEntityComponentIterator> s_centIterator {}; // HACK: This is a workaround for a bug in luabind, which causes errors when compiled with gcc.

static LuaEntityComponentIterator &citerator(lua_State *l, pragma::ComponentId componentId)
{
	s_centIterator = LuaEntityComponentIterator {l, componentId};
	return *s_centIterator;
}
static LuaEntityComponentIterator &citerator(lua_State *l, pragma::ComponentId componentId, const Lua::var<EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags)
{
	auto filterFlags = EntityIterator::FilterFlags::Default;
	luabind::object filterTable {};
	if(Lua::IsNumber(l, 2))
		filterFlags = static_cast<EntityIterator::FilterFlags>(luabind::object_cast<uint32_t>(oFilterOrFlags));
	else
		filterTable = oFilterOrFlags;
	s_centIterator = Lua::ents::create_lua_entity_component_iterator(l, componentId, filterTable, filterFlags);
	return *s_centIterator;
}
static LuaEntityComponentIterator &citerator(lua_State *l, pragma::ComponentId componentId, EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter)
{
	s_centIterator = Lua::ents::create_lua_entity_component_iterator(l, componentId, oFilter, filterFlags);
	return *s_centIterator;
}

void Game::RegisterLuaGameClasses(luabind::module_ &gameMod)
{
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents");
	RegisterLuaEntityComponents(modEnts);
	modEnts[luabind::def(
	  "iterator",
	  +[](lua_State *l) -> LuaEntityIterator & {
		  s_entIterator = LuaEntityIterator {l};
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "iterator",
	  +[](lua_State *l, const Lua::var<EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags) -> LuaEntityIterator & {
		  auto filterFlags = EntityIterator::FilterFlags::Default;
		  luabind::object filterTable {};
		  if(Lua::IsNumber(l, 1))
			  filterFlags = static_cast<EntityIterator::FilterFlags>(luabind::object_cast<uint32_t>(oFilterOrFlags));
		  else
			  filterTable = oFilterOrFlags;
		  s_entIterator = Lua::ents::create_lua_entity_iterator(l, filterTable, filterFlags);
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "iterator",
	  +[](lua_State *l, EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter) -> LuaEntityIterator & {
		  s_entIterator = Lua::ents::create_lua_entity_iterator(l, oFilter, filterFlags);
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "citerator", +[](lua_State *l, Lua::nil_type) -> LuaEntityComponentIterator & { return citerator(l, pragma::INVALID_COMPONENT_ID); }, luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua_State *, pragma::ComponentId)>(&citerator), luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua_State *, pragma::ComponentId, const Lua::var<EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &)>(&citerator), luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua_State *, pragma::ComponentId, EntityIterator::FilterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &)>(&citerator), luabind::return_stl_iterator {})];

	modEnts[luabind::def(
	  "citerator",
	  +[](lua_State *l, Game &game, const std::string &componentName) -> LuaEntityComponentIterator & {
		  pragma::ComponentId componentId;
		  if(!game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId))
			  Lua::Error(l, "Unknown component type '" + componentName + "'!");
		  return citerator(l, componentId);
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "citerator",
	  +[](lua_State *l, Game &game, const std::string &componentName, const Lua::var<EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags) -> LuaEntityComponentIterator & {
		  pragma::ComponentId componentId;
		  if(!game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId))
			  Lua::Error(l, "Unknown component type '" + componentName + "'!");
		  return citerator(l, componentId, oFilterOrFlags);
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "citerator",
	  +[](lua_State *l, Game &game, const std::string &componentName, EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter) -> LuaEntityComponentIterator & {
		  pragma::ComponentId componentId;
		  if(!game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId))
			  Lua::Error(l, "Unknown component type '" + componentName + "'!");
		  return citerator(l, componentId, filterFlags, oFilter);
	  },
	  luabind::return_stl_iterator {})];

	auto defItFilter = luabind::class_<LuaEntityIteratorFilterBase>("IteratorFilter");
	modEnts[defItFilter];

	auto defItFilterClass = luabind::class_<LuaEntityIteratorFilterClass, LuaEntityIteratorFilterBase>("IteratorFilterClass");
	defItFilterClass.def(luabind::constructor<const std::string &>());
	defItFilterClass.def(luabind::constructor<const std::string &, bool>());
	defItFilterClass.def(luabind::constructor<const std::string &, bool, bool>());
	modEnts[defItFilterClass];

	auto defItFilterName = luabind::class_<LuaEntityIteratorFilterName, LuaEntityIteratorFilterBase>("IteratorFilterName");
	defItFilterName.def(luabind::constructor<const std::string &>());
	defItFilterName.def(luabind::constructor<const std::string &, bool>());
	defItFilterName.def(luabind::constructor<const std::string &, bool, bool>());
	modEnts[defItFilterName];

	auto defItFilterUuid = luabind::class_<LuaEntityIteratorFilterUuid, LuaEntityIteratorFilterBase>("IteratorFilterUuid");
	defItFilterUuid.def(luabind::constructor<const std::string &>());
	modEnts[defItFilterUuid];

	auto defItFilterNameOrClass = luabind::class_<LuaEntityIteratorFilterNameOrClass, LuaEntityIteratorFilterBase>("IteratorFilterNameOrClass");
	defItFilterNameOrClass.def(luabind::constructor<const std::string &>());
	defItFilterNameOrClass.def(luabind::constructor<const std::string &, bool>());
	defItFilterNameOrClass.def(luabind::constructor<const std::string &, bool, bool>());
	modEnts[defItFilterNameOrClass];

	auto defItFilterEntity = luabind::class_<LuaEntityIteratorFilterEntity, LuaEntityIteratorFilterBase>("IteratorFilterEntity");
	defItFilterEntity.def(luabind::constructor<const std::string &>());
	modEnts[defItFilterEntity];

	auto defItFilterModel = luabind::class_<LuaEntityIteratorFilterModel, LuaEntityIteratorFilterBase>("IteratorFilterModel");
	defItFilterModel.def(luabind::constructor<const std::string &>());
	modEnts[defItFilterModel];

	auto defItFilterSphere = luabind::class_<LuaEntityIteratorFilterSphere, LuaEntityIteratorFilterBase>("IteratorFilterSphere");
	defItFilterSphere.def(luabind::constructor<const Vector3 &, float>());
	modEnts[defItFilterSphere];

	auto defItFilterBox = luabind::class_<LuaEntityIteratorFilterBox, LuaEntityIteratorFilterBase>("IteratorFilterBox");
	defItFilterBox.def(luabind::constructor<const Vector3 &, const Vector3 &>());
	modEnts[defItFilterBox];

	auto defItFilterCone = luabind::class_<LuaEntityIteratorFilterCone, LuaEntityIteratorFilterBase>("IteratorFilterCone");
	defItFilterCone.def(luabind::constructor<const Vector3 &, const Vector3 &, float, float>());
	modEnts[defItFilterCone];

	auto defItFilterComponent = luabind::class_<LuaEntityIteratorFilterComponent, LuaEntityIteratorFilterBase>("IteratorFilterComponent");
	defItFilterComponent.def(luabind::constructor<luabind::object>());
	defItFilterComponent.def(luabind::constructor<pragma::ComponentId>());
	defItFilterComponent.def(luabind::constructor<lua_State *, const std::string &>());
	modEnts[defItFilterComponent];

	auto defItFilterFunction = luabind::class_<LuaEntityIteratorFilterFunction, LuaEntityIteratorFilterBase>("IteratorFilterFunction");
	defItFilterFunction.def(luabind::constructor<Lua::func<bool>>());
	modEnts[defItFilterFunction];

	Lua::RegisterLibraryEnums(GetLuaState(), "ents",
	  {
	    {"ITERATOR_FILTER_BIT_NONE", umath::to_integral(EntityIterator::FilterFlags::None)},
	    {"ITERATOR_FILTER_BIT_SPAWNED", umath::to_integral(EntityIterator::FilterFlags::Spawned)},
	    {"ITERATOR_FILTER_BIT_PENDING", umath::to_integral(EntityIterator::FilterFlags::Pending)},
	    {"ITERATOR_FILTER_BIT_INCLUDE_SHARED", umath::to_integral(EntityIterator::FilterFlags::IncludeShared)},
	    {"ITERATOR_FILTER_BIT_INCLUDE_NETWORK_LOCAL", umath::to_integral(EntityIterator::FilterFlags::IncludeNetworkLocal)},

	    {"ITERATOR_FILTER_BIT_CHARACTER", umath::to_integral(EntityIterator::FilterFlags::Character)},
	    {"ITERATOR_FILTER_BIT_PLAYER", umath::to_integral(EntityIterator::FilterFlags::Player)},
	    {"ITERATOR_FILTER_BIT_WEAPON", umath::to_integral(EntityIterator::FilterFlags::Weapon)},
	    {"ITERATOR_FILTER_BIT_VEHICLE", umath::to_integral(EntityIterator::FilterFlags::Vehicle)},
	    {"ITERATOR_FILTER_BIT_NPC", umath::to_integral(EntityIterator::FilterFlags::NPC)},
	    {"ITERATOR_FILTER_BIT_PHYSICAL", umath::to_integral(EntityIterator::FilterFlags::Physical)},
	    {"ITERATOR_FILTER_BIT_SCRIPTED", umath::to_integral(EntityIterator::FilterFlags::Scripted)},
	    {"ITERATOR_FILTER_BIT_MAP_ENTITY", umath::to_integral(EntityIterator::FilterFlags::MapEntity)},

	    {"ITERATOR_FILTER_BIT_HAS_TRANSFORM", umath::to_integral(EntityIterator::FilterFlags::HasTransform)},
	    {"ITERATOR_FILTER_BIT_HAS_MODEL", umath::to_integral(EntityIterator::FilterFlags::HasModel)},

	    {"ITERATOR_FILTER_ANY_TYPE", umath::to_integral(EntityIterator::FilterFlags::AnyType)},
	    {"ITERATOR_FILTER_ANY", umath::to_integral(EntityIterator::FilterFlags::Any)},
	    {"ITERATOR_FILTER_DEFAULT", umath::to_integral(EntityIterator::FilterFlags::Default)},

	    {"TICK_POLICY_ALWAYS", umath::to_integral(pragma::TickPolicy::Always)},
	    {"TICK_POLICY_NEVER", umath::to_integral(pragma::TickPolicy::Never)},
	    {"TICK_POLICY_WHEN_VISIBLE", umath::to_integral(pragma::TickPolicy::WhenVisible)},
	  });

	auto surfaceMatDef = pragma::lua::register_class<SurfaceMaterial>(GetLuaState(), "SurfaceMaterial");
	surfaceMatDef->def("GetName", &::SurfaceMaterial::GetIdentifier);
	surfaceMatDef->def("GetIndex", &::SurfaceMaterial::GetIndex);
	surfaceMatDef->def("SetFriction", &::SurfaceMaterial::SetFriction);
	surfaceMatDef->def("SetStaticFriction", &::SurfaceMaterial::SetStaticFriction);
	surfaceMatDef->def("SetDynamicFriction", &::SurfaceMaterial::SetDynamicFriction);
	surfaceMatDef->def("GetStaticFriction", &::SurfaceMaterial::GetStaticFriction);
	surfaceMatDef->def("GetDynamicFriction", &::SurfaceMaterial::GetDynamicFriction);
	surfaceMatDef->def("GetRestitution", &::SurfaceMaterial::GetRestitution);
	surfaceMatDef->def("SetRestitution", &::SurfaceMaterial::SetRestitution);
	surfaceMatDef->def("GetFootstepSound", &::SurfaceMaterial::GetFootstepType);
	surfaceMatDef->def("SetFootstepSound", &::SurfaceMaterial::SetFootstepType);
	surfaceMatDef->def("SetImpactParticleEffect", &::SurfaceMaterial::SetImpactParticleEffect);
	surfaceMatDef->def("GetImpactParticleEffect", &::SurfaceMaterial::GetImpactParticleEffect);
	surfaceMatDef->def("GetBulletImpactSound", &::SurfaceMaterial::GetBulletImpactSound);
	surfaceMatDef->def("SetBulletImpactSound", &::SurfaceMaterial::SetBulletImpactSound);
	surfaceMatDef->def("SetHardImpactSound", &::SurfaceMaterial::SetHardImpactSound);
	surfaceMatDef->def("GetHardImpactSound", &::SurfaceMaterial::GetHardImpactSound);
	surfaceMatDef->def("SetSoftImpactSound", &::SurfaceMaterial::SetSoftImpactSound);
	surfaceMatDef->def("GetSoftImpactSound", &::SurfaceMaterial::GetSoftImpactSound);
	surfaceMatDef->def("GetIOR", &::SurfaceMaterial::GetIOR);
	surfaceMatDef->def("SetIOR", &::SurfaceMaterial::SetIOR);
	surfaceMatDef->def("ClearIOR", &::SurfaceMaterial::ClearIOR);

	surfaceMatDef->def("SetAudioLowFrequencyAbsorption", &::SurfaceMaterial::SetAudioLowFrequencyAbsorption);
	surfaceMatDef->def("GetAudioLowFrequencyAbsorption", &::SurfaceMaterial::GetAudioLowFrequencyAbsorption);
	surfaceMatDef->def("SetAudioMidFrequencyAbsorption", &::SurfaceMaterial::SetAudioMidFrequencyAbsorption);
	surfaceMatDef->def("GetAudioMidFrequencyAbsorption", &::SurfaceMaterial::GetAudioMidFrequencyAbsorption);
	surfaceMatDef->def("SetAudioHighFrequencyAbsorption", &::SurfaceMaterial::SetAudioHighFrequencyAbsorption);
	surfaceMatDef->def("GetAudioHighFrequencyAbsorption", &::SurfaceMaterial::GetAudioHighFrequencyAbsorption);
	surfaceMatDef->def("SetAudioScattering", &::SurfaceMaterial::SetAudioScattering);
	surfaceMatDef->def("GetAudioScattering", &::SurfaceMaterial::GetAudioScattering);
	surfaceMatDef->def("SetAudioLowFrequencyTransmission", &::SurfaceMaterial::SetAudioLowFrequencyTransmission);
	surfaceMatDef->def("GetAudioLowFrequencyTransmission", &::SurfaceMaterial::GetAudioLowFrequencyTransmission);
	surfaceMatDef->def("SetAudioMidFrequencyTransmission", &::SurfaceMaterial::SetAudioMidFrequencyTransmission);
	surfaceMatDef->def("GetAudioMidFrequencyTransmission", &::SurfaceMaterial::GetAudioMidFrequencyTransmission);
	surfaceMatDef->def("SetAudioHighFrequencyTransmission", &::SurfaceMaterial::SetAudioHighFrequencyTransmission);
	surfaceMatDef->def("GetAudioHighFrequencyTransmission", &::SurfaceMaterial::GetAudioHighFrequencyTransmission);

	surfaceMatDef->def("GetNavigationFlags", &::SurfaceMaterial::GetNavigationFlags);
	surfaceMatDef->def("SetNavigationFlags", &::SurfaceMaterial::SetNavigationFlags);
	surfaceMatDef->def("SetDensity", &::SurfaceMaterial::SetDensity);
	surfaceMatDef->def("GetDensity", &::SurfaceMaterial::GetDensity);
	surfaceMatDef->def("SetLinearDragCoefficient", &::SurfaceMaterial::SetLinearDragCoefficient);
	surfaceMatDef->def("GetLinearDragCoefficient", &::SurfaceMaterial::GetLinearDragCoefficient);
	surfaceMatDef->def("SetTorqueDragCoefficient", &::SurfaceMaterial::SetTorqueDragCoefficient);
	surfaceMatDef->def("GetTorqueDragCoefficient", &::SurfaceMaterial::GetTorqueDragCoefficient);
	surfaceMatDef->def("SetWaveStiffness", &::SurfaceMaterial::SetWaveStiffness);
	surfaceMatDef->def("GetWaveStiffness", &::SurfaceMaterial::GetWaveStiffness);
	surfaceMatDef->def("SetWavePropagation", &::SurfaceMaterial::SetWavePropagation);
	surfaceMatDef->def("GetWavePropagation", &::SurfaceMaterial::GetWavePropagation);
	surfaceMatDef->def("GetPBRMetalness", static_cast<float (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().metalness; }));
	surfaceMatDef->def("GetPBRRoughness", static_cast<float (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().roughness; }));
	surfaceMatDef->def("GetSubsurfaceFactor", static_cast<float (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.factor; }));
	surfaceMatDef->def("SetSubsurfaceFactor", static_cast<void (*)(lua_State *, SurfaceMaterial &, float)>([](lua_State *l, SurfaceMaterial &surfMat, float factor) { surfMat.GetPBRInfo().subsurface.factor = factor; }));
	surfaceMatDef->def("GetSubsurfaceScatterColor", static_cast<Vector3 (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.scatterColor; }));
	surfaceMatDef->def("SetSubsurfaceScatterColor", static_cast<void (*)(lua_State *, SurfaceMaterial &, const Vector3 &)>([](lua_State *l, SurfaceMaterial &surfMat, const Vector3 &radiusRGB) { surfMat.GetPBRInfo().subsurface.scatterColor = radiusRGB; }));
	surfaceMatDef->def("GetSubsurfaceRadiusMM", static_cast<Vector3 (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.radiusMM; }));
	surfaceMatDef->def("SetSubsurfaceRadiusMM", static_cast<void (*)(lua_State *, SurfaceMaterial &, const Vector3 &)>([](lua_State *l, SurfaceMaterial &surfMat, const Vector3 &radiusMM) { surfMat.GetPBRInfo().subsurface.radiusMM = radiusMM; }));
	surfaceMatDef->def("GetSubsurfaceColor", static_cast<Color (*)(lua_State *, SurfaceMaterial &)>([](lua_State *l, SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.color; }));
	surfaceMatDef->def("SetSubsurfaceColor", static_cast<void (*)(lua_State *, SurfaceMaterial &, const Color &)>([](lua_State *l, SurfaceMaterial &surfMat, const Color &color) { surfMat.GetPBRInfo().subsurface.color = color; }));
	gameMod[*surfaceMatDef];

	auto gibletCreateInfo = luabind::class_<GibletCreateInfo>("GibletCreateInfo");
	gibletCreateInfo.def(luabind::constructor<>());
	gibletCreateInfo.def_readwrite("model", &GibletCreateInfo::model);
	gibletCreateInfo.def_readwrite("skin", &GibletCreateInfo::skin);
	gibletCreateInfo.def_readwrite("scale", &GibletCreateInfo::scale);
	gibletCreateInfo.def_readwrite("mass", &GibletCreateInfo::mass);
	gibletCreateInfo.def_readwrite("lifetime", &GibletCreateInfo::lifetime);
	gibletCreateInfo.def_readwrite("position", &GibletCreateInfo::position);
	gibletCreateInfo.def_readwrite("rotation", &GibletCreateInfo::rotation);
	gibletCreateInfo.def_readwrite("velocity", &GibletCreateInfo::velocity);
	gibletCreateInfo.def_readwrite("angularVelocity", &GibletCreateInfo::angularVelocity);

	gibletCreateInfo.def_readwrite("physTranslationOffset", &GibletCreateInfo::physTranslationOffset);
	gibletCreateInfo.def_readwrite("physRotationOffset", &GibletCreateInfo::physRotationOffset);
	gibletCreateInfo.def_readwrite("physRadius", &GibletCreateInfo::physRadius);
	gibletCreateInfo.def_readwrite("physHeight", &GibletCreateInfo::physHeight);
	gibletCreateInfo.def_readwrite("physShape", reinterpret_cast<std::underlying_type_t<decltype(GibletCreateInfo::physShape)> GibletCreateInfo::*>(&GibletCreateInfo::physShape));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_MODEL", umath::to_integral(GibletCreateInfo::PhysShape::Model));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_NONE", umath::to_integral(GibletCreateInfo::PhysShape::None));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_SPHERE", umath::to_integral(GibletCreateInfo::PhysShape::Sphere));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_BOX", umath::to_integral(GibletCreateInfo::PhysShape::Box));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_CYLINDER", umath::to_integral(GibletCreateInfo::PhysShape::Cylinder));
	gameMod[gibletCreateInfo];

	Lua::register_bullet_info(gameMod);

	auto classDefDamageInfo = luabind::class_<DamageInfo>("DamageInfo");
	classDefDamageInfo.def(luabind::constructor<>());
	classDefDamageInfo.def(luabind::tostring(luabind::self));
	classDefDamageInfo.def("SetDamage", &::DamageInfo::SetDamage);
	classDefDamageInfo.def("AddDamage", &::DamageInfo::AddDamage);
	classDefDamageInfo.def("ScaleDamage", &::DamageInfo::ScaleDamage);
	classDefDamageInfo.def("GetDamage", &::DamageInfo::GetDamage);
	classDefDamageInfo.def("GetAttacker", &::DamageInfo::GetAttacker);
	classDefDamageInfo.def("SetAttacker", static_cast<void (::DamageInfo::*)(const BaseEntity *)>(&::DamageInfo::SetAttacker));
	classDefDamageInfo.def("GetInflictor", &::DamageInfo::GetInflictor);
	classDefDamageInfo.def("SetInflictor", static_cast<void (::DamageInfo::*)(const BaseEntity *)>(&::DamageInfo::SetInflictor));
	classDefDamageInfo.def("GetDamageTypes", &::DamageInfo::GetDamageTypes);
	classDefDamageInfo.def("SetDamageType", &::DamageInfo::SetDamageType);
	classDefDamageInfo.def("AddDamageType", &::DamageInfo::AddDamageType);
	classDefDamageInfo.def("RemoveDamageType", &::DamageInfo::RemoveDamageType);
	classDefDamageInfo.def("IsDamageType", &::DamageInfo::IsDamageType);
	classDefDamageInfo.def("SetSource", &::DamageInfo::SetSource);
	classDefDamageInfo.def("GetSource", &::DamageInfo::GetSource, luabind::copy_policy<0> {});
	classDefDamageInfo.def("SetHitPosition", &::DamageInfo::SetHitPosition);
	classDefDamageInfo.def("GetHitPosition", &::DamageInfo::GetHitPosition, luabind::copy_policy<0> {});
	classDefDamageInfo.def("SetForce", &::DamageInfo::SetForce);
	classDefDamageInfo.def("GetForce", &::DamageInfo::GetForce, luabind::copy_policy<0> {});
	classDefDamageInfo.def("GetHitGroup", &::DamageInfo::GetHitGroup);
	classDefDamageInfo.def("SetHitGroup", &::DamageInfo::SetHitGroup);
	gameMod[classDefDamageInfo];
}

#define LUA_MATRIX_MEMBERS_CLASSDEF(defMat, type)                                                                                                                                                                                                                                                \
	defMat.def(luabind::constructor<>());                                                                                                                                                                                                                                                        \
	defMat.def(luabind::constructor<float>());                                                                                                                                                                                                                                                   \
	defMat.def(luabind::const_self / float());                                                                                                                                                                                                                                                   \
	defMat.def(luabind::const_self *float());                                                                                                                                                                                                                                                    \
	defMat.def(float() / luabind::const_self);                                                                                                                                                                                                                                                   \
	defMat.def(float() * luabind::const_self);                                                                                                                                                                                                                                                   \
	defMat.def(luabind::tostring(luabind::self));                                                                                                                                                                                                                                                \
	defMat.def(luabind::constructor<Mat2>());                                                                                                                                                                                                                                                    \
	defMat.def(luabind::constructor<Mat2x3>());                                                                                                                                                                                                                                                  \
	defMat.def(luabind::constructor<Mat2x4>());                                                                                                                                                                                                                                                  \
	defMat.def(luabind::constructor<Mat3>());                                                                                                                                                                                                                                                    \
	defMat.def(luabind::constructor<Mat3x2>());                                                                                                                                                                                                                                                  \
	defMat.def(luabind::constructor<Mat3x4>());                                                                                                                                                                                                                                                  \
	defMat.def(luabind::constructor<Mat4>());                                                                                                                                                                                                                                                    \
	defMat.def(luabind::constructor<Mat4x2>());                                                                                                                                                                                                                                                  \
	defMat.def(luabind::constructor<Mat4x3>());                                                                                                                                                                                                                                                  \
	defMat.def("Copy", &Lua::Mat##type::Copy);                                                                                                                                                                                                                                                   \
	defMat.def("Get", &Lua::Mat##type::Get);                                                                                                                                                                                                                                                     \
	defMat.def("Set", static_cast<void (*)(lua_State *, Mat##type &, int, int, float)>(&Lua::Mat##type::Set));                                                                                                                                                                                   \
	defMat.def("Transpose", &Lua::Mat##type::Transpose);                                                                                                                                                                                                                                         \
	defMat.def("GetTranspose", &Lua::Mat##type::GetTransposition);

static void RegisterLuaMatrices(Lua::Interface &lua)
{
	auto &modMath = lua.RegisterLibrary("math");
	auto defMat2 = luabind::class_<Mat2>("Mat2");
	defMat2.def(luabind::constructor<float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2, 2);
	defMat2.def(luabind::const_self == luabind::const_self);
	defMat2.def(luabind::const_self + luabind::const_self);
	defMat2.def(luabind::const_self - luabind::const_self);
	defMat2.def(luabind::const_self * luabind::const_self);
	defMat2.def(luabind::const_self * Vector2());
	defMat2.def("Inverse", &Lua::Mat2::Inverse);
	defMat2.def("GetInverse", &Lua::Mat2::GetInverse);
	defMat2.def("Set", static_cast<void (*)(lua_State *, ::Mat2 &, float, float, float, float)>(&Lua::Mat2::Set));
	defMat2.def("Set", static_cast<void (*)(lua_State *, ::Mat2 &, const ::Mat2 &)>(&Lua::Mat2::Set));
	modMath[defMat2];
	register_string_to_vector_type_constructor<Mat2>(lua.GetState());

	auto defMat2x3 = luabind::class_<Mat2x3>("Mat2x3");
	defMat2x3.def(luabind::constructor<float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x3, 2x3);
	defMat2x3.def(luabind::const_self == luabind::const_self);
	defMat2x3.def(luabind::const_self + luabind::const_self);
	defMat2x3.def(luabind::const_self - luabind::const_self);
	defMat2x3.def(luabind::const_self * Vector2());
	defMat2x3.def("Set", static_cast<void (*)(lua_State *, ::Mat2x3 &, float, float, float, float, float, float)>(&Lua::Mat2x3::Set));
	defMat2x3.def("Set", static_cast<void (*)(lua_State *, ::Mat2x3 &, const ::Mat2x3 &)>(&Lua::Mat2x3::Set));
	modMath[defMat2x3];
	register_string_to_vector_type_constructor<Mat2x3>(lua.GetState());

	auto defMat2x4 = luabind::class_<Mat2x4>("Mat2x4");
	defMat2x4.def(luabind::constructor<float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x4, 2x4);
	defMat2x4.def(luabind::const_self == luabind::const_self);
	defMat2x4.def(luabind::const_self + luabind::const_self);
	defMat2x4.def(luabind::const_self - luabind::const_self);
	defMat2x4.def(luabind::const_self * Vector2());
	defMat2x4.def("Set", static_cast<void (*)(lua_State *, ::Mat2x4 &, float, float, float, float, float, float, float, float)>(&Lua::Mat2x4::Set));
	defMat2x4.def("Set", static_cast<void (*)(lua_State *, ::Mat2x4 &, const ::Mat2x4 &)>(&Lua::Mat2x4::Set));
	modMath[defMat2x4];
	register_string_to_vector_type_constructor<Mat2x4>(lua.GetState());

	auto defMat3 = luabind::class_<Mat3>("Mat3");
	defMat3.def(luabind::constructor<float, float, float, float, float, float, float, float, float>());
	defMat3.def(luabind::constructor<Quat>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3, 3);
	defMat3.def(luabind::const_self == luabind::const_self);
	defMat3.def(luabind::const_self + luabind::const_self);
	defMat3.def(luabind::const_self - luabind::const_self);
	defMat3.def(luabind::const_self * luabind::const_self);
	defMat3.def(luabind::const_self * Vector3());
	defMat3.def("Inverse", &Lua::Mat3::Inverse);
	defMat3.def("GetInverse", &Lua::Mat3::GetInverse);
	defMat3.def("Set", static_cast<void (*)(lua_State *, ::Mat3 &, float, float, float, float, float, float, float, float, float)>(&Lua::Mat3::Set));
	defMat3.def("Set", static_cast<void (*)(lua_State *, ::Mat3 &, const ::Mat3 &)>(&Lua::Mat3::Set));
	defMat3.def("CalcEigenValues", &Lua::Mat3::CalcEigenValues);
	modMath[defMat3];
	register_string_to_vector_type_constructor<Mat3>(lua.GetState());

	auto defMat3x2 = luabind::class_<Mat3x2>("Mat3x2");
	defMat3x2.def(luabind::constructor<float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3x2, 3x2);
	defMat3x2.def(luabind::const_self == luabind::const_self);
	defMat3x2.def(luabind::const_self + luabind::const_self);
	defMat3x2.def(luabind::const_self - luabind::const_self);
	defMat3x2.def(luabind::const_self * Vector3());
	defMat3x2.def("Set", static_cast<void (*)(lua_State *, ::Mat3x2 &, float, float, float, float, float, float)>(&Lua::Mat3x2::Set));
	defMat3x2.def("Set", static_cast<void (*)(lua_State *, ::Mat3x2 &, const ::Mat3x2 &)>(&Lua::Mat3x2::Set));
	modMath[defMat3x2];
	register_string_to_vector_type_constructor<Mat3x2>(lua.GetState());

	auto defMat3x4 = luabind::class_<Mat3x4>("Mat3x4");
	defMat3x4.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3x4, 3x4);
	defMat3x4.def(luabind::const_self == luabind::const_self);
	defMat3x4.def(luabind::const_self + luabind::const_self);
	defMat3x4.def(luabind::const_self - luabind::const_self);
	defMat3x4.def(luabind::const_self * Vector3());
	defMat3x4.def("Set", static_cast<void (*)(lua_State *, ::Mat3x4 &, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat3x4::Set));
	defMat3x4.def("Set", static_cast<void (*)(lua_State *, ::Mat3x4 &, const ::Mat3x4 &)>(&Lua::Mat3x4::Set));
	modMath[defMat3x4];
	register_string_to_vector_type_constructor<Mat3x4>(lua.GetState());

	auto defMat4 = luabind::class_<Mat4>("Mat4");
	defMat4.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4, 4);
	defMat4.def("Translate", &Lua::Mat4::Translate);
	defMat4.def("Rotate", static_cast<void (*)(lua_State *, ::Mat4 &, const EulerAngles &)>(&Lua::Mat4::Rotate));
	defMat4.def("Rotate", static_cast<void (*)(lua_State *, ::Mat4 &, const Vector3 &, float)>(&Lua::Mat4::Rotate));
	defMat4.def("Scale", &Lua::Mat4::Scale);
	defMat4.def("ToEulerAngles", &Lua::Mat4::ToEulerAngles);
	defMat4.def("ToQuaternion", &Lua::Mat4::ToQuaternion);
	defMat4.def("Decompose", &Lua::Mat4::Decompose);
	defMat4.def("MulRow", static_cast<void (*)(lua_State *, Mat4 &, uint32_t, float)>([](lua_State *l, Mat4 &m, uint32_t rowIndex, float factor) {
		for(uint8_t i = 0; i < 4; ++i)
			m[rowIndex][i] *= factor;
	}));
	defMat4.def("MulCol", static_cast<void (*)(lua_State *, Mat4 &, uint32_t, float)>([](lua_State *l, Mat4 &m, uint32_t colIndex, float factor) {
		for(uint8_t i = 0; i < 4; ++i)
			m[i][colIndex] *= factor;
	}));
	defMat4.def("SwapRows", static_cast<void (*)(lua_State *, Mat4 &, uint32_t, uint32_t)>([](lua_State *l, Mat4 &m, uint32_t rowSrc, uint32_t rowDst) {
		std::array<float, 4> tmpRow = {m[rowSrc][0], m[rowSrc][1], m[rowSrc][2], m[rowSrc][3]};
		for(uint8_t i = 0; i < 4; ++i) {
			m[rowSrc][i] = m[rowDst][i];
			m[rowDst][i] = tmpRow.at(i);
		}
	}));
	defMat4.def("SwapColumns", static_cast<void (*)(lua_State *, Mat4 &, uint32_t, uint32_t)>([](lua_State *l, Mat4 &m, uint32_t colSrc, uint32_t colDst) {
		std::array<float, 4> tmpCol = {m[0][colSrc], m[1][colSrc], m[2][colSrc], m[3][colSrc]};
		for(uint8_t i = 0; i < 4; ++i) {
			m[i][colSrc] = m[i][colDst];
			m[i][colDst] = tmpCol.at(i);
		}
	}));
	defMat4.def("ApplyProjectionDepthBiasOffset", static_cast<void (*)(lua_State *, Mat4 &, float, float, float, float)>([](lua_State *l, Mat4 &p, float nearZ, float farZ, float d, float delta) { umat::apply_projection_depth_bias_offset(p, nearZ, farZ, d, delta); }));
	defMat4.def(luabind::const_self == luabind::const_self);
	defMat4.def(luabind::const_self + luabind::const_self);
	defMat4.def(luabind::const_self - luabind::const_self);
	defMat4.def(luabind::const_self * luabind::const_self);
	defMat4.def(luabind::const_self * Vector4());
	defMat4.def("Inverse", &Lua::Mat4::Inverse);
	defMat4.def("GetInverse", &Lua::Mat4::GetInverse);
	defMat4.def("Set", static_cast<void (*)(lua_State *, ::Mat4 &, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat4::Set));
	defMat4.def("Set", static_cast<void (*)(lua_State *, ::Mat4 &, const ::Mat4 &)>(&Lua::Mat4::Set));
	modMath[defMat4];
	register_string_to_vector_type_constructor<Mat4>(lua.GetState());
	auto _G = luabind::globals(lua.GetState());
	_G["Mat4"] = _G["math"]["Mat4"];

	auto defMat4x2 = luabind::class_<Mat4x2>("Mat4x2");
	defMat4x2.def(luabind::constructor<float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4x2, 4x2);
	defMat4x2.def(luabind::const_self == luabind::const_self);
	defMat4x2.def(luabind::const_self + luabind::const_self);
	defMat4x2.def(luabind::const_self - luabind::const_self);
	defMat4x2.def(luabind::const_self * Vector4());
	defMat4x2.def("Set", static_cast<void (*)(lua_State *, ::Mat4x2 &, float, float, float, float, float, float, float, float)>(&Lua::Mat4x2::Set));
	defMat4x2.def("Set", static_cast<void (*)(lua_State *, ::Mat4x2 &, const ::Mat4x2 &)>(&Lua::Mat4x2::Set));
	modMath[defMat4x2];
	register_string_to_vector_type_constructor<Mat4x2>(lua.GetState());

	auto defMat4x3 = luabind::class_<Mat4x3>("Mat4x3");
	defMat4x3.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4x3, 4x3);
	defMat4x3.def(luabind::const_self == luabind::const_self);
	defMat4x3.def(luabind::const_self + luabind::const_self);
	defMat4x3.def(luabind::const_self - luabind::const_self);
	defMat4x3.def(luabind::const_self * Vector4());
	defMat4x3.def("Set", static_cast<void (*)(lua_State *, ::Mat4x3 &, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat4x3::Set));
	defMat4x3.def("Set", static_cast<void (*)(lua_State *, ::Mat4x3 &, const ::Mat4x3 &)>(&Lua::Mat4x3::Set));
	modMath[defMat4x3];
	register_string_to_vector_type_constructor<Mat4x3>(lua.GetState());
}

class IkLuaConstraint : public uvec::ik::IkConstraint {
  public:
	IkLuaConstraint(uvec::ik::IkJoint &joint, luabind::object o) : IkConstraint {joint}, m_function {o} {}
	virtual void Apply(int i) override;
  private:
	luabind::object m_function;
};
void IkLuaConstraint::Apply(int i) { m_function(i); }

static void RegisterIk(Lua::Interface &lua)
{
	auto defIkSolver = luabind::class_<uvec::ik::IkSolver>("IkSolver");
	defIkSolver.def("GetGlobalTransform", &uvec::ik::IkSolver::GetGlobalTransform);
	defIkSolver.def("SetLocalTransform", &uvec::ik::IkSolver::SetLocalTransform);
	defIkSolver.def("GetLocalTransform", &uvec::ik::IkSolver::GetLocalTransform);
	defIkSolver.def("Solve", &uvec::ik::IkSolver::Solve);
	defIkSolver.def("Resize", &uvec::ik::IkSolver::Resize);
	defIkSolver.def("Size", &uvec::ik::IkSolver::Size);
	defIkSolver.def("AddHingeConstraint", static_cast<uvec::ik::IkHingeConstraint *(*)(uvec::ik::IkSolver &, uint32_t, const Vector3 &)>([](uvec::ik::IkSolver &solver, uint32_t idx, const Vector3 &axis) { return &solver.GetJoint(idx).AddConstraint<uvec::ik::IkHingeConstraint>(axis); }));
	defIkSolver.def("AddBallSocketConstraint", static_cast<uvec::ik::IkBallSocketConstraint *(*)(uvec::ik::IkSolver &, uint32_t, float)>([](uvec::ik::IkSolver &solver, uint32_t idx, float limit) { return &solver.GetJoint(idx).AddConstraint<uvec::ik::IkBallSocketConstraint>(limit); }));
	defIkSolver.def("AddCustomConstraint",
	  static_cast<uvec::ik::IkConstraint *(*)(lua_State *, uvec::ik::IkSolver &, uint32_t, const Lua::func<void, int32_t> &)>(
	    [](lua_State *l, uvec::ik::IkSolver &solver, uint32_t idx, const Lua::func<void, int32_t> &function) -> uvec::ik::IkConstraint * { return &solver.GetJoint(idx).AddConstraint<IkLuaConstraint>(function); }));

	auto &modIk = lua.RegisterLibrary("ik");
	auto defIkConstraint = luabind::class_<uvec::ik::IkConstraint>("IkConstraint");
	defIkConstraint.def("GetJointIndex", static_cast<uint32_t (*)(lua_State *, uvec::ik::IkConstraint &)>([](lua_State *l, uvec::ik::IkConstraint &constraint) -> uint32_t { return constraint.GetJoint().GetJointIndex(); }));
	modIk[defIkConstraint];

	auto defIkHingeConstraint = luabind::class_<uvec::ik::IkHingeConstraint, uvec::ik::IkConstraint>("IkHingeConstraint");
	defIkHingeConstraint.def("SetLimits", &uvec::ik::IkHingeConstraint::SetLimits);
	defIkHingeConstraint.def("ClearLimits", &uvec::ik::IkHingeConstraint::ClearLimits);
	defIkHingeConstraint.def("GetLimits", &uvec::ik::IkHingeConstraint::GetLimits);
	modIk[defIkHingeConstraint];

	auto defIkBallSocketConstraint = luabind::class_<uvec::ik::IkBallSocketConstraint, uvec::ik::IkConstraint>("IkBallSocketConstraint");
	defIkBallSocketConstraint.def("SetLimit", &uvec::ik::IkBallSocketConstraint::SetLimit);
	defIkBallSocketConstraint.def("GetLimit", static_cast<std::optional<float> (*)(lua_State *, uvec::ik::IkBallSocketConstraint &)>([](lua_State *l, uvec::ik::IkBallSocketConstraint &constraint) -> std::optional<float> {
		float limit;
		if(constraint.GetLimit(limit) == false)
			return {};
		return limit;
	}));
	modIk[defIkBallSocketConstraint];

	auto defCcdSolver = luabind::class_<uvec::ik::CCDSolver, uvec::ik::IkSolver>("CCDIkSolver");
	defCcdSolver.def(luabind::constructor<>());

	auto defFABRIKSolver = luabind::class_<uvec::ik::FABRIKSolver, uvec::ik::IkSolver>("FABRIkSolver");
	defFABRIKSolver.def(luabind::constructor<>());

	modIk[defIkSolver];
	modIk[defCcdSolver];
	modIk[defFABRIKSolver];
}
