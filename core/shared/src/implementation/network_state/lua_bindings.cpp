// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "noiseutils.h"

#include "pragma/lua/ostream_operator_alias.hpp"
#include <cassert>

module pragma.shared;

import :network_state;
import pragma.string.unicode;

std::ostream &operator<<(std::ostream &out, const pragma::audio::ALSound &snd)
{
	auto state = snd.GetState();
	out << "ALSound[" << snd.GetIndex() << "][";
	switch(state) {
	case pragma::audio::ALState::Initial:
		out << "Initial";
		break;
	case pragma::audio::ALState::Playing:
		out << "Playing";
		break;
	case pragma::audio::ALState::Paused:
		out << "Paused";
		break;
	case pragma::audio::ALState::Stopped:
		out << "Stopped";
		break;
	}
	out << "][";
	auto type = snd.GetType();
	auto values = pragma::math::get_power_of_2_values(static_cast<uint64_t>(type));
	auto bStart = true;
	for(auto v : values) {
		if(bStart == false)
			out << " | ";
		else
			bStart = true;
		if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Effect))
			out << "Effect";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Music))
			out << "Music";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Voice))
			out << "Voice";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Weapon))
			out << "Weapon";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::NPC))
			out << "NPC";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Player))
			out << "Player";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Vehicle))
			out << "Vehicle";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Physics))
			out << "Physics";
		else if(v == static_cast<uint64_t>(pragma::audio::ALSoundType::Environment))
			out << "Environment";
	}
	out << "]";
	return out;
}

static void RegisterLuaMatrices(Lua::Interface &lua);

static void create_directory_change_listener(lua::State *l, const std::string &path, luabind::object callback, pragma::fs::DirectoryWatcherCallback::WatchFlags flags)
{
	Lua::CheckFunction(l, 2);
	try {
		auto listener = pragma::util::make_shared<pragma::fs::DirectoryWatcherCallback>(path, [callback](const std::string &fileName) mutable { callback(fileName); }, flags);
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

static void register_utf8_string(lua::State *l, luabind::module_ &modStr)
{
	auto defBaseStr = pragma::LuaCore::register_class<"BaseUtf8String", pragma::string::BaseUtf8String>(l);
	defBaseStr->def("GetSize", &pragma::string::BaseUtf8String::size);
	defBaseStr->def("GetLength", &pragma::string::BaseUtf8String::length);
	defBaseStr->def("IsEmpty", &pragma::string::BaseUtf8String::empty);
	defBaseStr->def("GetAt", &pragma::string::BaseUtf8String::at);
	defBaseStr->def("GetFront", &pragma::string::BaseUtf8String::front);
	defBaseStr->def("GetBack", &pragma::string::BaseUtf8String::back);
	defBaseStr->def("ToUpper", &pragma::string::BaseUtf8String::toUpper);
	defBaseStr->def("ToLower", &pragma::string::BaseUtf8String::toLower);
	defBaseStr->def("ToString", +[](const pragma::string::BaseUtf8String &str) -> std::string { return static_cast<std::string>(str); });
	modStr[*defBaseStr];

	auto defStr = pragma::LuaCore::register_class<"Utf8String", pragma::string::Utf8String, pragma::string::BaseUtf8String>(l);
	defStr->def(luabind::constructor<>());
	defStr->def(luabind::constructor<const std::string &>());
	defStr->def(luabind::constructor<const pragma::string::Utf8String &>());
	defStr->def(luabind::const_self == luabind::const_self);
	defStr->def(luabind::tostring(luabind::self));
	defStr->def("SubStr", +[](const pragma::string::Utf8String &str, size_t start, size_t count) -> pragma::string::Utf8String { return str.substr(start, count); });
	defStr->def("SubStr", +[](const pragma::string::Utf8String &str, size_t start) -> pragma::string::Utf8String { return str.substr(start); });
	defStr->def("Find", +[](const pragma::string::Utf8String &str, const std::string &s) -> size_t { return str.find(s); });
	defStr->def("Find", +[](const pragma::string::Utf8String &str, const pragma::string::BaseUtf8String &s) -> size_t { return str.find(s); });
	modStr[*defStr];
}

static void register_directory_watcher(lua::State *l, luabind::module_ &modUtil)
{
	auto defListener = pragma::LuaCore::register_class<"DirectoryChangeListener", pragma::fs::DirectoryWatcherCallback>(l);
	defListener->add_static_constant("LISTENER_FLAG_NONE", pragma::math::to_integral(pragma::fs::DirectoryWatcherCallback::WatchFlags::None));
	defListener->add_static_constant("LISTENER_FLAG_BIT_WATCH_SUB_DIRECTORIES", pragma::math::to_integral(pragma::fs::DirectoryWatcherCallback::WatchFlags::WatchSubDirectories));
	defListener->add_static_constant("LISTENER_FLAG_ABSOLUTE_PATH", pragma::math::to_integral(pragma::fs::DirectoryWatcherCallback::WatchFlags::AbsolutePath));
	defListener->add_static_constant("LISTENER_FLAG_START_DISABLED", pragma::math::to_integral(pragma::fs::DirectoryWatcherCallback::WatchFlags::StartDisabled));
	defListener->add_static_constant("LISTENER_FLAG_WATCH_DIRECTORY_CHANGES", pragma::math::to_integral(pragma::fs::DirectoryWatcherCallback::WatchFlags::WatchDirectoryChanges));
	static_assert(magic_enum::enum_count<pragma::fs::DirectoryWatcherCallback::WatchFlags>() == 4);
	defListener
	  ->scope[luabind::def("create", static_cast<void (*)(lua::State *, const std::string &, luabind::object)>([](lua::State *l, const std::string &path, luabind::object callback) { create_directory_change_listener(l, path, callback, pragma::fs::DirectoryWatcherCallback::WatchFlags::None); }))];
	defListener->scope[luabind::def("create", static_cast<void (*)(lua::State *, const std::string &, luabind::object, pragma::fs::DirectoryWatcherCallback::WatchFlags)>([](lua::State *l, const std::string &path, luabind::object callback, pragma::fs::DirectoryWatcherCallback::WatchFlags flags) {
		create_directory_change_listener(l, path, callback, flags);
	}))];
	defListener->def("Poll", static_cast<uint32_t (*)(lua::State *, pragma::fs::DirectoryWatcherCallback &)>([](lua::State *l, pragma::fs::DirectoryWatcherCallback &listener) { return listener.Poll(); }));
	defListener->def("SetEnabled", static_cast<void (*)(lua::State *, pragma::fs::DirectoryWatcherCallback &, bool)>([](lua::State *l, pragma::fs::DirectoryWatcherCallback &listener, bool enabled) { listener.SetEnabled(enabled); }));
	defListener->def("IsEnabled", static_cast<bool (*)(lua::State *, pragma::fs::DirectoryWatcherCallback &)>([](lua::State *l, pragma::fs::DirectoryWatcherCallback &listener) { return listener.IsEnabled(); }));
	modUtil[*defListener];
}

namespace pragma::math {
	std::ostream &operator<<(std::ostream &out, const Transform &t)
	{
		auto &origin = t.GetOrigin();
		auto &rot = t.GetRotation();
		auto ang = EulerAngles {rot};
		out << "Transform[" << origin.x << "," << origin.y << "," << origin.z << "][" << ang.p << "," << ang.y << "," << ang.r << "]";
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const ScaledTransform &t)
	{
		auto &origin = t.GetOrigin();
		auto &rot = t.GetRotation();
		auto ang = EulerAngles {rot};
		auto &scale = t.GetScale();
		out << "ScaledTransform[" << origin.x << "," << origin.y << "," << origin.z << "][" << ang.p << "," << ang.y << "," << ang.r << "][" << scale.x << "," << scale.y << "," << scale.z << "]";
		return out;
	}
}
namespace pragma::image {
	std::ostream &operator<<(std::ostream &out, const ImageLayerSet &layerSet)
	{
		out << "ImageLayerSet[" << layerSet.images.size() << "]";
		return out;
	}
}
template<typename T>
static void register_string_to_vector_type_constructor(lua::State *l)
{
	pragma::LuaCore::define_custom_constructor<T,
	  +[](const std::string &str) -> T {
		  T r;
		  using ValueType = typename decltype(r)::value_type;
		  pragma::string::string_to_array<ValueType>(str, reinterpret_cast<ValueType *>(&r[0]), pragma::string::cstring_to_number<float>, decltype(r)::length());
		  return r;
	  },
	  const std::string &>(l);
}

// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::string, Utf8String);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, Transform);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, ScaledTransform);

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::util, BaseParallelJob);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::image, ImageBuffer);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::util, Version);
#endif

// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(uimg, ImageLayerSet);

// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector3i);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector4i);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector2);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector3);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Vector4);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(glm, Quat);

/* namespace panima
{
std::ostream &operator<<(std::ostream &out,const pragma::animation::Bone &o)
{
    return ::operator<<(out,o);
}
}; */

#undef DEFINE_OSTEAM_OPERATOR_NAMESPACE_ALIAS

//namespace glm {
//	std::ostream &operator<<(std::ostream &out, Vector3 &o) { return ::operator<<(out, o); }
//	std::ostream &operator<<(std::ostream &out, Vector4 &o) { return ::operator<<(out, o); }
//
//#define DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(type)                                                                                                                                                                                                                                            \
//	std::ostream &operator<<(std::ostream &out, const Mat##type &o) { return ::operator<<(out, o); }
//
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x2)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x3)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(2x4)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x2)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x3)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(3x4)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x2)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x3)
//	DEFINE_OSTREAM_MATRIX_OPERATOR_INTERNAL(4x4)
//
//};

std::string pragma::LuaCore::detail::tostring(const luabind::object &o)
{
	auto oToString = o["__tostring"];
	auto t = static_cast<Lua::Type>(luabind::type(oToString));
	if(t != Lua::Type::Function)
		return "ERROR: No __tostring method!";
	auto oStr = luabind::call_member<luabind::object>(o, "__tostring");
	return luabind::object_cast_nothrow<std::string>(oStr, std::string {"ERROR: Failed to cast to string!"});
}

void pragma::LuaCore::detail::register_lua_debug_tostring(lua::State *l, const std::type_info &typeInfo)
{
	// This is required to display __tostring with lua-debug
	// https://github.com/actboy168/lua-debug/issues/237#issuecomment-1475533297
	auto *registry = luabind::detail::class_registry::get_registry(l);
	auto *crep = registry->find_class(typeInfo);
	assert(crep);
	lua::raw_get(l, Lua::RegistryIndex, crep->metatable_ref());
	auto o = luabind::object {luabind::from_stack(l, -1)};
	o["__debugger_tostring"] = luabind::make_function(l, +[](const luabind::object &o) -> std::string { return tostring(o); });
	o["__name"] = crep->name();
	Lua::Pop(l, 1);
}

static void add_task(lua::State *l, pragma::LuaCore::LuaWorker &worker, const luabind::object &o0, const luabind::object &o1, float taskProgress)
{
	using TNonLuaTask = std::shared_ptr<pragma::util::ParallelJob<luabind::object>>;
	auto *nonLuaTask = luabind::object_cast_nothrow<TNonLuaTask *>(o0, static_cast<TNonLuaTask *>(nullptr));
	if(nonLuaTask) {
		auto &onCompleteTask = o1;
		if(static_cast<Lua::Type>(luabind::type(onCompleteTask)) != Lua::Type::Function)
			Lua::Error(l, "onCompleteTask is not a function!");
		worker.AddLuaTask(*nonLuaTask, onCompleteTask, taskProgress);
		return;
	}
	if(static_cast<Lua::Type>(luabind::type(o0)) == Lua::Type::Function) {
		auto &task = o0;
		auto &cancel = o1;
		if(static_cast<Lua::Type>(luabind::type(cancel)) != Lua::Type::Function)
			Lua::Error(l, "cancel is not a function!");
		worker.AddLuaTask(task, cancel, taskProgress);
		return;
	}
	auto &subJob = o0;
	auto &onCompleteTask = o1;
	worker.AddTask(subJob, onCompleteTask, taskProgress);
}

static void add_task(pragma::LuaCore::LuaWorker &worker, const std::shared_ptr<pragma::util::ParallelJob<luabind::object>> &subJob, float taskProgress) { worker.AddLuaTask(subJob, taskProgress); }

static int util_file_path(lua::State *l)
{
	int n = Lua::GetStackTop(l); /* number of arguments */
	int i;
	std::vector<pragma::util::Path> args;
	args.reserve(n);
	for(i = 1; i <= n; i++) {
		if(Lua::IsType<pragma::util::Path>(l, i))
			args.push_back(Lua::Check<pragma::util::Path>(l, i));
		else
			args.push_back({Lua::CheckString(l, i)});
	}
	pragma::util::Path path {};
	if(args.size() > 1) {
		for(size_t i = 0; i < (args.size() - 1); ++i)
			path = pragma::util::DirPath(path, args[i]);
		path = pragma::util::FilePath(path, args.back());
	}
	else if(args.size() == 1)
		path = pragma::util::FilePath(args.back());

	auto &str = path.GetString();
	if(!str.empty() && str.front() == '/')
		path.PopFront();

	Lua::Push(l, path);
	return 1;
}

static int util_dir_path(lua::State *l)
{
	int n = Lua::GetStackTop(l); /* number of arguments */
	int i;
	std::vector<pragma::util::Path> args;
	args.reserve(n);
	for(i = 1; i <= n; i++) {
		if(Lua::IsType<pragma::util::Path>(l, i))
			args.push_back(Lua::Check<pragma::util::Path>(l, i));
		else
			args.push_back({Lua::CheckString(l, i)});
	}
	pragma::util::Path path {};
	for(size_t i = 0; i < args.size(); ++i)
		path = pragma::util::DirPath(path, args[i]);

	auto &str = path.GetString();
	if(!str.empty() && str.front() == '/')
		path.PopFront();

	Lua::Push(l, path);
	return 1;
}

template<typename T>
void glm_type_to_string(lua::State *l, const T &v) {
	std::stringstream ss;
	ss<<v;
	Lua::PushString(l, ss.str());
}

void pragma::NetworkState::RegisterSharedLuaClasses(Lua::Interface &lua)
{
	auto modString = luabind::module_(lua.GetState(), "string");
	register_utf8_string(lua.GetState(), modString);
	modString[(luabind::def("snake_case_to_camel_case", Lua::string::snake_case_to_camel_case), luabind::def("camel_case_to_snake_case", Lua::string::camel_case_to_snake_case), luabind::def("calc_levenshtein_distance", Lua::string::calc_levenshtein_distance),
	  luabind::def("calc_levenshtein_similarity", Lua::string::calc_levenshtein_similarity),
	  luabind::def("find_longest_common_substring", Lua::string::find_longest_common_substring, luabind::meta::join<luabind::pure_out_value<3>, luabind::pure_out_value<4>, luabind::pure_out_value<5>>::type {}),
	  luabind::def("find_similar_elements", Lua::string::find_similar_elements, luabind::meta::join<luabind::pure_out_value<5>, luabind::pure_out_value<6>>::type {}), luabind::def("is_integer", string::is_integer), luabind::def("is_number", string::is_number),
	  luabind::def("split", Lua::string::split), luabind::def("join", static_cast<std::string (*)(lua::State *, luabind::table<>, const std::string &)>(Lua::string::join)),
	  luabind::def("join", static_cast<std::string (*)(lua::State *, luabind::table<>)>([](lua::State *l, luabind::table<> values) { return Lua::string::join(l, values); })), luabind::def("remove_whitespace", Lua::string::remove_whitespace),
	  luabind::def("remove_quotes", Lua::string::remove_quotes), luabind::def("replace", static_cast<std::string (*)(const std::string &, const std::string &, const std::string &)>([](const std::string &subject, const std::string &from, const std::string &to) -> std::string {
		  auto tmp = subject;
		  string::replace(tmp, from, to);
		  return tmp;
	  })),
	  luabind::def("fill_zeroes", string::fill_zeroes), luabind::def("compare", static_cast<bool (*)(const char *, const char *, bool, size_t)>(string::compare)), luabind::def("compare", static_cast<bool (*)(const std::string &, const std::string &, bool)>(string::compare)),
	  luabind::def("compare", static_cast<bool (*)(const std::string &, const std::string &)>([](const std::string &a, const std::string &b) -> bool { return string::compare(a, b, true); })),
	  luabind::def("hash", static_cast<std::string (*)(const std::string &)>([](const std::string &str) -> std::string { return std::to_string(std::hash<std::string> {}(str)); })))];

	auto modLight = luabind::module_(lua.GetState(), "light");
	modLight[(luabind::def("get_color_temperature", static_cast<void (*)(ulighting::NaturalLightType, Kelvin &, Kelvin &)>([](ulighting::NaturalLightType type, Kelvin &outMin, Kelvin &outMax) {
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
	    +[](float intensity, BaseEnvLightComponent::LightIntensityType srcType, BaseEnvLightComponent::LightIntensityType dstType, float coneAngle) -> float {
		    auto result = 0.f;
		    switch(dstType) {
		    case BaseEnvLightComponent::LightIntensityType::Candela:
			    result = BaseEnvLightComponent::GetLightIntensityCandela(intensity, srcType, coneAngle);
			    break;
		    case BaseEnvLightComponent::LightIntensityType::Lumen:
			    result = BaseEnvLightComponent::GetLightIntensityLumen(intensity, srcType, coneAngle);
			    break;
		    default:
			    break;
		    }
		    return result;
	    }),
	  luabind::def(
	    "convert_light_intensity", +[](float intensity, BaseEnvLightComponent::LightIntensityType srcType, BaseEnvLightComponent::LightIntensityType dstType) -> float {
		    auto result = 0.f;
		    switch(dstType) {
		    case BaseEnvLightComponent::LightIntensityType::Candela:
			    result = BaseEnvLightComponent::GetLightIntensityCandela(intensity, srcType);
			    break;
		    case BaseEnvLightComponent::LightIntensityType::Lumen:
			    result = BaseEnvLightComponent::GetLightIntensityLumen(intensity, srcType);
			    break;
		    default:
			    break;
		    }
		    return result;
	    }))];

	Lua::RegisterLibraryEnums(lua.GetState(), "light",
	  {{"NATURAL_LIGHT_TYPE_MATCH_FLAME", math::to_integral(ulighting::NaturalLightType::MatchFlame)}, {"NATURAL_LIGHT_TYPE_CANDLE", math::to_integral(ulighting::NaturalLightType::Candle)}, {"NATURAL_LIGHT_TYPE_FLAME", math::to_integral(ulighting::NaturalLightType::Flame)},
	    {"NATURAL_LIGHT_TYPE_SUNSET", math::to_integral(ulighting::NaturalLightType::Sunset)}, {"NATURAL_LIGHT_TYPE_SUNRISE", math::to_integral(ulighting::NaturalLightType::Sunrise)},
	    {"NATURAL_LIGHT_TYPE_HOUSEHOLD_TUNGSTEN_BULB", math::to_integral(ulighting::NaturalLightType::HouseholdTungstenBulb)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_500W_TO_1K", math::to_integral(ulighting::NaturalLightType::TungstenLamp500WTo1K)},
	    {"NATURAL_LIGHT_TYPE_INCANDESCENT_LAMP", math::to_integral(ulighting::NaturalLightType::IncandescentLamp)}, {"NATURAL_LIGHT_TYPE_WARM_FLUORESCENT_LAMP", math::to_integral(ulighting::NaturalLightType::WarmFluorescentLamp)},
	    {"NATURAL_LIGHT_TYPE_LED_LAMP", math::to_integral(ulighting::NaturalLightType::LEDLamp)}, {"NATURAL_LIGHT_TYPE_QUARTZ_LIGHT", math::to_integral(ulighting::NaturalLightType::QuartzLight)},
	    {"NATURAL_LIGHT_TYPE_STUDIO_LAMP", math::to_integral(ulighting::NaturalLightType::StudioLamp)}, {"NATURAL_LIGHT_TYPE_FLOODLIGHT", math::to_integral(ulighting::NaturalLightType::Floodlight)},
	    {"NATURAL_LIGHT_TYPE_FLUORESCENT_LIGHT", math::to_integral(ulighting::NaturalLightType::FluorescentLight)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_2K", math::to_integral(ulighting::NaturalLightType::TungstenLamp2K)},
	    {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_5K", math::to_integral(ulighting::NaturalLightType::TungstenLamp5K)}, {"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_10K", math::to_integral(ulighting::NaturalLightType::TungstenLamp10K)},
	    {"NATURAL_LIGHT_TYPE_MOONLIGHT", math::to_integral(ulighting::NaturalLightType::Moonlight)}, {"NATURAL_LIGHT_TYPE_HORIZON_DAYLIGHT", math::to_integral(ulighting::NaturalLightType::HorizonDaylight)},
	    {"NATURAL_LIGHT_TYPE_TUBULAR_FLUORESCENT_LAMP", math::to_integral(ulighting::NaturalLightType::TubularFluorescentLamp)}, {"NATURAL_LIGHT_TYPE_VERTICAL_DAYLIGHT", math::to_integral(ulighting::NaturalLightType::VerticalDaylight)},
	    {"NATURAL_LIGHT_TYPE_SUN_AT_NOON", math::to_integral(ulighting::NaturalLightType::SunAtNoon)}, {"NATURAL_LIGHT_TYPE_DAYLIGHT", math::to_integral(ulighting::NaturalLightType::Daylight)},
	    {"NATURAL_LIGHT_TYPE_SUN_THROUGH_CLOUDS", math::to_integral(ulighting::NaturalLightType::SunThroughClouds)}, {"NATURAL_LIGHT_TYPE_OVERCAST", math::to_integral(ulighting::NaturalLightType::Overcast)},
	    {"NATURAL_LIGHT_TYPE_RGB_MONITOR_WHITE_POINT", math::to_integral(ulighting::NaturalLightType::RGBMonitorWhitePoint)}, {"NATURAL_LIGHT_TYPE_OUTDOOR_SHADE", math::to_integral(ulighting::NaturalLightType::OutdoorShade)},
	    {"NATURAL_LIGHT_TYPE_PARTLY_CLOUDY", math::to_integral(ulighting::NaturalLightType::PartlyCloudy)}, {"NATURAL_LIGHT_TYPE_CLEAR_BLUESKY", math::to_integral(ulighting::NaturalLightType::ClearBlueSky)},
	    {"NATURAL_LIGHT_TYPE_CLEAR_COUNT", math::to_integral(ulighting::NaturalLightType::Count)},

	    {"LIGHT_SOURCE_TYPE_TUNGSTEN_INCANDESCENT_LIGHT_BULB", math::to_integral(ulighting::LightSourceType::TungstenIncandescentLightBulb)}, {"LIGHT_SOURCE_TYPE_HALOGEN_LAMP", math::to_integral(ulighting::LightSourceType::HalogenLamp)},
	    {"LIGHT_SOURCE_TYPE_FLUORESCENT_LAMP", math::to_integral(ulighting::LightSourceType::FluorescentLamp)}, {"LIGHT_SOURCE_TYPE_LED_LAMP", math::to_integral(ulighting::LightSourceType::LEDLamp)},
	    {"LIGHT_SOURCE_TYPE_METAL_HALIDE_LAMP", math::to_integral(ulighting::LightSourceType::MetalHalideLamp)}, {"LIGHT_SOURCE_TYPE_HIGH_PRESSURE_SODIUM_VAPOR_LAMP", math::to_integral(ulighting::LightSourceType::HighPressureSodiumVaporLamp)},
	    {"LIGHT_SOURCE_TYPE_LOW_PRESSURE_SODIUM_VAPOR_LAMP", math::to_integral(ulighting::LightSourceType::LowPressureSodiumVaporLamp)}, {"LIGHT_SOURCE_TYPE_MERCURY_VAPOR_LAMP", math::to_integral(ulighting::LightSourceType::MercuryVaporLamp)},
	    {"LIGHT_SOURCE_TYPE_D65_STANDARD_ILLUMINANT", math::to_integral(ulighting::LightSourceType::D65StandardIlluminant)}});

	auto &modUtil = lua.RegisterLibrary("util");
	register_directory_watcher(lua.GetState(), modUtil);

	auto defParallelJob = pragma::LuaCore::register_class<util::BaseParallelJob>(lua.GetState(), "ParallelJob");
	defParallelJob->add_static_constant("JOB_STATUS_FAILED", math::to_integral(util::JobStatus::Failed));
	defParallelJob->add_static_constant("JOB_STATUS_SUCCESSFUL", math::to_integral(util::JobStatus::Successful));
	defParallelJob->add_static_constant("JOB_STATUS_INITIAL", math::to_integral(util::JobStatus::Initial));
	defParallelJob->add_static_constant("JOB_STATUS_CANCELLED", math::to_integral(util::JobStatus::Cancelled));
	defParallelJob->add_static_constant("JOB_STATUS_PENDING", math::to_integral(util::JobStatus::Pending));
	defParallelJob->add_static_constant("JOB_STATUS_INVALID", math::to_integral(util::JobStatus::Invalid));
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

	auto defImageBuffer = pragma::LuaCore::register_class<image::ImageBuffer>(lua.GetState(), "ImageBuffer");
	defImageBuffer->add_static_constant("FORMAT_NONE", math::to_integral(image::Format::None));
	defImageBuffer->add_static_constant("FORMAT_RGB8", math::to_integral(image::Format::RGB8));
	defImageBuffer->add_static_constant("FORMAT_RGBA8", math::to_integral(image::Format::RGBA8));
	defImageBuffer->add_static_constant("FORMAT_RGB16", math::to_integral(image::Format::RGB16));
	defImageBuffer->add_static_constant("FORMAT_RGBA16", math::to_integral(image::Format::RGBA16));
	defImageBuffer->add_static_constant("FORMAT_RGB32", math::to_integral(image::Format::RGB32));
	defImageBuffer->add_static_constant("FORMAT_RGBA32", math::to_integral(image::Format::RGBA32));
	defImageBuffer->add_static_constant("FORMAT_COUNT", math::to_integral(image::Format::Count));

	defImageBuffer->add_static_constant("FORMAT_RGB_LDR", math::to_integral(image::Format::RGB_LDR));
	defImageBuffer->add_static_constant("FORMAT_RGBA_LDR", math::to_integral(image::Format::RGBA_LDR));
	defImageBuffer->add_static_constant("FORMAT_RGB_HDR", math::to_integral(image::Format::RGB_HDR));
	defImageBuffer->add_static_constant("FORMAT_RGBA_HDR", math::to_integral(image::Format::RGBA_HDR));
	defImageBuffer->add_static_constant("FORMAT_RGB_FLOAT", math::to_integral(image::Format::RGB_FLOAT));
	defImageBuffer->add_static_constant("FORMAT_RGBA_FLOAT", math::to_integral(image::Format::RGBA_FLOAT));

	defImageBuffer->add_static_constant("CHANNEL_RED", math::to_integral(image::Channel::Red));
	defImageBuffer->add_static_constant("CHANNEL_GREEN", math::to_integral(image::Channel::Green));
	defImageBuffer->add_static_constant("CHANNEL_BLUE", math::to_integral(image::Channel::Blue));
	defImageBuffer->add_static_constant("CHANNEL_ALPHA", math::to_integral(image::Channel::Alpha));
	defImageBuffer->add_static_constant("CHANNEL_R", math::to_integral(image::Channel::R));
	defImageBuffer->add_static_constant("CHANNEL_G", math::to_integral(image::Channel::G));
	defImageBuffer->add_static_constant("CHANNEL_B", math::to_integral(image::Channel::B));
	defImageBuffer->add_static_constant("CHANNEL_A", math::to_integral(image::Channel::A));

	defImageBuffer->add_static_constant("TONE_MAPPING_GAMMA_CORRECTION", math::to_integral(image::ToneMapping::GammaCorrection));
	defImageBuffer->add_static_constant("TONE_MAPPING_REINHARD", math::to_integral(image::ToneMapping::Reinhard));
	defImageBuffer->add_static_constant("TONE_MAPPING_HEJIL_RICHARD", math::to_integral(image::ToneMapping::HejilRichard));
	defImageBuffer->add_static_constant("TONE_MAPPING_UNCHARTED", math::to_integral(image::ToneMapping::Uncharted));
	defImageBuffer->add_static_constant("TONE_MAPPING_ACES", math::to_integral(image::ToneMapping::Aces));
	defImageBuffer->add_static_constant("TONE_MAPPING_GRAN_TURISMO", math::to_integral(image::ToneMapping::GranTurismo));

	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua::State *, uint32_t, uint32_t, uint32_t, util::DataStream &)>([](lua::State *l, uint32_t width, uint32_t height, uint32_t format, util::DataStream &ds) {
		auto imgBuffer = image::ImageBuffer::Create(ds->GetData(), width, height, static_cast<image::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua::State *, uint32_t, uint32_t, uint32_t)>([](lua::State *l, uint32_t width, uint32_t height, uint32_t format) {
		auto imgBuffer = image::ImageBuffer::Create(width, height, static_cast<image::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("Create", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
		auto imgBuffer = image::ImageBuffer::Create(parent, x, y, w, h);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->scope[luabind::def("CreateCubemap", static_cast<void (*)(lua::State *, luabind::object)>([](lua::State *l, luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l, t);
		std::array<std::shared_ptr<image::ImageBuffer>, 6> cubemapSides {};
		for(uint8_t i = 0; i < 6; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, t);
			auto &img = Lua::Check<image::ImageBuffer>(l, -1);
			cubemapSides.at(i) = img.shared_from_this();
			Lua::Pop(l, 1);
		}
		auto imgBuffer = image::ImageBuffer::CreateCubemap(cubemapSides);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l, imgBuffer);
	}))];
	defImageBuffer->def("GetData", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) {
		auto *data = imgBuffer.GetData();
		auto dataSize = imgBuffer.GetSize();
		util::DataStream ds {data, static_cast<uint32_t>(dataSize)};
		ds->SetOffset(0);
		Lua::Push(l, ds);
	}));
	defImageBuffer->def("GetFormat", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, math::to_integral(imgBuffer.GetFormat())); }));
	defImageBuffer->def("GetWidth", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetWidth()); }));
	defImageBuffer->def("GetHeight", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetHeight()); }));
	defImageBuffer->def("GetChannelCount", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetChannelCount()); }));
	defImageBuffer->def("GetChannelSize", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetChannelSize()); }));
	defImageBuffer->def("GetPixelSize", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetPixelSize()); }));
	defImageBuffer->def("GetPixelCount", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetPixelCount()); }));
	defImageBuffer->def("HasAlphaChannel", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.HasAlphaChannel()); }));
	defImageBuffer->def("IsLDRFormat", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsLDRFormat()); }));
	defImageBuffer->def("IsHDRFormat", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsHDRFormat()); }));
	defImageBuffer->def("IsFloatFormat", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushBool(l, imgBuffer.IsFloatFormat()); }));
	defImageBuffer->def("Insert", static_cast<void (image::ImageBuffer::*)(const image::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(&image::ImageBuffer::Insert));
	defImageBuffer->def("Insert", static_cast<void (image::ImageBuffer::*)(const image::ImageBuffer &, uint32_t, uint32_t)>(&image::ImageBuffer::Insert));
	defImageBuffer->def("Copy", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::Push(l, imgBuffer.Copy()); }));
	defImageBuffer->def("Copy", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t format) { Lua::Push(l, imgBuffer.Copy(static_cast<image::Format>(format))); }));
	defImageBuffer->def("Copy",
	  static_cast<void (*)(lua::State *, image::ImageBuffer &, image::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua::State *l, image::ImageBuffer &imgBuffer, image::ImageBuffer &dst, uint32_t xSrc, uint32_t ySrc, uint32_t xDst, uint32_t yDst, uint32_t w, uint32_t h) { imgBuffer.Copy(dst, xSrc, ySrc, xDst, yDst, w, h); }));
	defImageBuffer->def("Convert", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t format) { imgBuffer.Convert(static_cast<image::Format>(format)); }));
	defImageBuffer->def(
	  "ToLDR", +[](lua::State *l, image::ImageBuffer &imgBuffer, const LuaCore::LuaThreadWrapper &tw) {
		  auto pImgBuffer = imgBuffer.shared_from_this();
		  auto task = [pImgBuffer]() -> LuaCore::LuaThreadPool::ResultHandler {
			  pImgBuffer->ToLDR();
			  return {};
		  };
		  if(tw.IsTask())
			  tw.GetTask()->AddSubTask(task);
		  else
			  tw.GetPool().AddTask(task);
	  });
	defImageBuffer->def("ToLDR", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.ToLDR(); }));
	defImageBuffer->def("ToHDR", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.ToHDR(); }));
	defImageBuffer->def("ToFloat", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.ToFloat(); }));
	defImageBuffer->def("GetSize", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { Lua::PushInt(l, imgBuffer.GetSize()); }));
	defImageBuffer->def("Clear", static_cast<void (*)(lua::State *, image::ImageBuffer &, const Color &)>([](lua::State *l, image::ImageBuffer &imgBuffer, const Color &color) { imgBuffer.Clear(color); }));
	defImageBuffer->def("Clear", static_cast<void (*)(lua::State *, image::ImageBuffer &, const Vector4 &)>([](lua::State *l, image::ImageBuffer &imgBuffer, const Vector4 &color) { imgBuffer.Clear(color); }));
	defImageBuffer->def("ClearAlpha", static_cast<void (*)(lua::State *, image::ImageBuffer &, float)>([](lua::State *l, image::ImageBuffer &imgBuffer, float a) {
		a = math::clamp(a, 0.f, 1.f);
		imgBuffer.ClearAlpha(a * std::numeric_limits<uint8_t>::max());
	}));
	defImageBuffer->def("GetPixelIndex", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelIndex(x, y)); }));
	defImageBuffer->def("GetPixelOffset", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelOffset(x, y)); }));
	defImageBuffer->def("Resize", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) { imgBuffer.Resize(w, h); }));
	defImageBuffer->def("FlipHorizontally", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.FlipHorizontally(); }));
	defImageBuffer->def("FlipVertically", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.FlipVertically(); }));
	defImageBuffer->def("Flip", static_cast<void (*)(lua::State *, image::ImageBuffer &, bool, bool)>([](lua::State *l, image::ImageBuffer &imgBuffer, bool flipH, bool flipV) { imgBuffer.Flip(flipH, flipV); }));
	defImageBuffer->def("SwapChannels", static_cast<void (*)(lua::State *, image::ImageBuffer &, image::Channel, image::Channel)>([](lua::State *l, image::ImageBuffer &imgBuffer, image::Channel channel0, image::Channel channel1) { imgBuffer.SwapChannels(channel0, channel1); }));
	defImageBuffer->def(
	  "SwapChannels", +[](lua::State *l, image::ImageBuffer &imgBuffer, image::Channel channel0, image::Channel channel1, const LuaCore::LuaThreadWrapper &tw) {
		  auto pImgBuffer = imgBuffer.shared_from_this();
		  auto task = [pImgBuffer, channel0, channel1]() -> LuaCore::LuaThreadPool::ResultHandler {
			  pImgBuffer->SwapChannels(channel0, channel1);
			  return {};
		  };
		  if(tw.IsTask())
			  tw.GetTask()->AddSubTask(task);
		  else
			  tw.GetPool().AddTask(task);
	  });
	defImageBuffer->def("ApplyToneMapping", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t toneMapping) {
		auto tonemappedImg = imgBuffer.ApplyToneMapping(static_cast<image::ToneMapping>(toneMapping));
		if(tonemappedImg == nullptr)
			return;
		Lua::Push(l, tonemappedImg);
	}));
	defImageBuffer->def("ApplyGammaCorrection", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) { imgBuffer.ApplyGammaCorrection(); }));
	defImageBuffer->def("ApplyGammaCorrection", static_cast<void (*)(lua::State *, image::ImageBuffer &, float)>([](lua::State *l, image::ImageBuffer &imgBuffer, float gamma) { imgBuffer.ApplyGammaCorrection(gamma); }));
	defImageBuffer->def("ApplyExposure", static_cast<void (*)(lua::State *, image::ImageBuffer &, float)>([](lua::State *l, image::ImageBuffer &imgBuffer, float exposure) { imgBuffer.ApplyExposure(exposure); }));

	defImageBuffer->def("GetPixelOffset", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelOffset(x, y)); }));
	defImageBuffer->def("GetPixelIndex", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y) { Lua::PushInt(l, imgBuffer.GetPixelIndex(x, y)); }));
	defImageBuffer->def("GetPixelValue", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t, uint32_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel) {
		Lua::PushNumber(l, imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).GetFloatValue(static_cast<image::Channel>(channel)));
	}));
	defImageBuffer->def("SetPixelValue", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t, uint32_t, float)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, float value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<image::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelValueLDR", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint8_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, uint8_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<image::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelValueHDR", static_cast<void (*)(lua::State *, image::ImageBuffer &, uint32_t, uint32_t, uint32_t, uint16_t)>([](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, uint32_t channel, uint16_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x, y)).SetValue(static_cast<image::Channel>(channel), value);
	}));
	defImageBuffer->def("SetPixelColor", static_cast<void (image::ImageBuffer::*)(uint32_t, uint32_t, const Vector4 &)>(&image::ImageBuffer::SetPixelColor));
	defImageBuffer->def("SetPixelColor", static_cast<void (image::ImageBuffer::*)(image::ImageBuffer::PixelIndex, const Vector4 &)>(&image::ImageBuffer::SetPixelColor));
	defImageBuffer->def("SetPixelColor", +[](lua::State *l, image::ImageBuffer &imgBuffer, uint32_t x, uint32_t y, const Color &color) { imgBuffer.SetPixelColor(x, y, color.ToVector4()); });
	defImageBuffer->def("SetPixelColor", +[](lua::State *l, image::ImageBuffer &imgBuffer, image::ImageBuffer::PixelIndex pixelIdx, const Color &color) { imgBuffer.SetPixelColor(pixelIdx, color.ToVector4()); });
	defImageBuffer->def("CalcLuminance", static_cast<void (*)(lua::State *, image::ImageBuffer &)>([](lua::State *l, image::ImageBuffer &imgBuffer) {
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

	auto defImageLayerSet = pragma::LuaCore::register_class<image::ImageLayerSet>(lua.GetState(), "ImageLayerSet");
	defImageLayerSet->def(
	  "GetImage", +[](const image::ImageLayerSet &layerSet, const std::string &name) -> std::shared_ptr<image::ImageBuffer> {
		  auto it = layerSet.images.find(name);
		  if(it == layerSet.images.end())
			  return nullptr;
		  return it->second;
	  });
	defImageLayerSet->def("GetImages", +[](const image::ImageLayerSet &layerSet) { return layerSet.images; });
	modUtil[*defImageLayerSet];

	auto defWorker = luabind::class_<LuaCore::LuaWorker>("Worker");
	defWorker.add_static_constant("TASK_STATUS_PENDING", math::to_integral(LuaCore::LuaWorker::TaskStatus::Pending));
	defWorker.add_static_constant("TASK_STATUS_COMPLETE", math::to_integral(LuaCore::LuaWorker::TaskStatus::Complete));
	defWorker.def("GetResult", &LuaCore::LuaWorker::GetResult);
	defWorker.def("SetResult", &LuaCore::LuaWorker::SetResult);
	defWorker.def("SetStatus", &LuaCore::LuaWorker::SetStatus);
	defWorker.def("SetStatus", +[](LuaCore::LuaWorker &worker, util::JobStatus jobStatus, const std::optional<std::string> &resultMsg) { worker.SetStatus(jobStatus, resultMsg); });
	defWorker.def("SetStatus", +[](LuaCore::LuaWorker &worker, util::JobStatus jobStatus) { worker.SetStatus(jobStatus); });
	defWorker.def("UpdateProgress", &LuaCore::LuaWorker::UpdateProgress);
	defWorker.def("AddTask", static_cast<void (*)(LuaCore::LuaWorker &, const std::shared_ptr<util::ParallelJob<luabind::object>> &, float)>(&add_task));
	defWorker.def("AddTask", static_cast<void (*)(lua::State *, LuaCore::LuaWorker &, const luabind::object &, const luabind::object &, float)>(&add_task));
	defWorker.def("SetProgressCallback", &LuaCore::LuaWorker::SetProgressCallback);
	defWorker.def("Cancel", +[](LuaCore::LuaWorker &worker) { worker.Cancel(); });
	defWorker.def("IsComplete", +[](LuaCore::LuaWorker &worker) { return worker.IsComplete(); });
	defWorker.def("IsPending", +[](LuaCore::LuaWorker &worker) { return worker.IsPending(); });
	defWorker.def("IsCancelled", +[](LuaCore::LuaWorker &worker) { return worker.IsCancelled(); });
	defWorker.def("IsSuccessful", +[](LuaCore::LuaWorker &worker) { return worker.IsSuccessful(); });
	defWorker.def("IsThreadActive", +[](LuaCore::LuaWorker &worker) { return worker.IsThreadActive(); });
	defWorker.def("GetProgress", +[](LuaCore::LuaWorker &worker) { return worker.GetProgress(); });
	defWorker.def("GetStatus", +[](LuaCore::LuaWorker &worker) { return worker.GetStatus(); });
	defWorker.def("GetResultMessage", +[](LuaCore::LuaWorker &worker) { return worker.GetResultMessage(); });
	defWorker.def("GetResultCode", +[](LuaCore::LuaWorker &worker) { return worker.GetResultCode(); });
	defWorker.def("IsValid", +[](LuaCore::LuaWorker &worker) { return worker.IsValid(); });
	modUtil[defWorker];

	auto defLuaParallelJob = luabind::class_<util::ParallelJob<luabind::object>, util::BaseParallelJob>("ParallelJob");
	defLuaParallelJob.def("GetResult", static_cast<luabind::object (*)(lua::State *, util::ParallelJob<luabind::object> &)>([](lua::State *l, util::ParallelJob<luabind::object> &job) -> luabind::object { return job.GetResult(); }));
	defLuaParallelJob.def("CallOnComplete", +[](util::ParallelJob<luabind::object> &job, const Lua::func<void> &onComplete) { static_cast<LuaCore::LuaWorker &>(job.GetWorker()).CallOnComplete(onComplete); });
	defLuaParallelJob.def("SetProgressCallback", +[](util::ParallelJob<luabind::object> &job, const Lua::func<float> &func) { static_cast<LuaCore::LuaWorker &>(job.GetWorker()).SetProgressCallback(func); });
	modUtil[defLuaParallelJob];
	modUtil[luabind::def(
	  "create_parallel_job", +[](Game &game, const std::string &name, const Lua::func<void> &func, const Lua::func<void> &cancelFunc) -> std::shared_ptr<util::ParallelJob<luabind::object>> {
		  auto job = pragma::util::make_shared<util::ParallelJob<luabind::object>>(pragma::util::create_parallel_job<LuaCore::LuaWorker>(game, name));
		  static_cast<LuaCore::LuaWorker &>(job->GetWorker()).AddLuaTask(func, cancelFunc, 0.f);
		  return job;
	  })];

	auto defGenericParallelJob = luabind::class_<util::ParallelJob<void>, util::BaseParallelJob>("ParallelJobGeneric");
	modUtil[defGenericParallelJob];

	auto defImgParallelJob = luabind::class_<util::ParallelJob<std::shared_ptr<image::ImageBuffer>>, util::BaseParallelJob>("ParallelJobImage");
	defImgParallelJob.def("GetResult", static_cast<void (*)(lua::State *, util::ParallelJob<std::shared_ptr<image::ImageBuffer>> &)>([](lua::State *l, util::ParallelJob<std::shared_ptr<image::ImageBuffer>> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defImgParallelJob];

	auto defImgLayerSetParallelJob = luabind::class_<util::ParallelJob<image::ImageLayerSet>, util::BaseParallelJob>("ParallelJobImageLayerSet");
	defImgLayerSetParallelJob.def("GetResult", +[](lua::State *l, util::ParallelJob<image::ImageLayerSet> &job) { Lua::Push(l, job.GetResult()); });
	defImgLayerSetParallelJob.def(
	  "GetImage", +[](lua::State *l, util::ParallelJob<image::ImageLayerSet> &job) -> std::shared_ptr<image::ImageBuffer> {
		  if(job.GetResult().images.empty())
			  return nullptr;
		  return job.GetResult().images.begin()->second;
	  });
	modUtil[defImgLayerSetParallelJob];

	auto defStringParallelJob = luabind::class_<util::ParallelJob<const std::string &>, util::BaseParallelJob>("ParallelJobString");
	defStringParallelJob.def("GetResult", static_cast<void (*)(lua::State *, util::ParallelJob<const std::string &> &)>([](lua::State *l, util::ParallelJob<const std::string &> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defStringParallelJob];

	auto defDataStreamJob = luabind::class_<util::ParallelJob<const util::DataStream &>, util::BaseParallelJob>("ParallelJobData");
	defDataStreamJob.def("GetResult", static_cast<void (*)(lua::State *, util::ParallelJob<const util::DataStream &> &)>([](lua::State *l, util::ParallelJob<const util::DataStream &> &job) { Lua::Push(l, job.GetResult()); }));
	modUtil[defDataStreamJob];

	auto defDataBlock = luabind::class_<datasystem::Block>("DataBlock");
	defDataBlock.scope[luabind::def("load", static_cast<void (*)(lua::State *, const std::string &)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("load", static_cast<void (*)(lua::State *, LFile &)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("create", static_cast<void (*)(lua::State *)>(Lua::DataBlock::create))];

	defDataBlock.def("GetInt", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector2", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetVector2));
	defDataBlock.def("GetVector4", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::GetVector4));
	defDataBlock.def("GetValue", &Lua::DataBlock::GetValue);

	defDataBlock.def("GetInt", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, int32_t)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, float)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, bool)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, const std::string &)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, const Color &)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, const Vector3 &)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector2", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, const Vector2 &)>(&Lua::DataBlock::GetVector2));
	defDataBlock.def("GetVector4", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, const Vector4 &)>(&Lua::DataBlock::GetVector4));

	defDataBlock.def("GetData", &Lua::DataBlock::GetData);
	defDataBlock.def("GetChildBlocks", &Lua::DataBlock::GetChildBlocks);
	defDataBlock.def("SetValue", &Lua::DataBlock::SetValue);
	defDataBlock.def("Merge", &Lua::DataBlock::Merge);
	defDataBlock.def("GetValueType", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>([](lua::State *l, datasystem::Block &dataBlock, const std::string &key) {
		auto val = dataBlock.GetDataValue(key);
		if(val == nullptr)
			return;
		Lua::PushString(l, val->GetTypeString());
	}));
	defDataBlock.def("GetKeys", static_cast<void (*)(lua::State *, datasystem::Block &)>([](lua::State *l, datasystem::Block &dataBlock) {
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
	defDataBlock.def("ToString", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, uint8_t)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua::State *, datasystem::Block &, uint8_t)>(&Lua::DataBlock::ToString));
	defDataBlock.def("ToString", static_cast<void (*)(lua::State *, datasystem::Block &)>(&Lua::DataBlock::ToString));
	defDataBlock.def("FindBlock", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &)>(&Lua::DataBlock::FindBlock));
	defDataBlock.def("FindBlock", static_cast<void (*)(lua::State *, datasystem::Block &, const std::string &, uint32_t)>(&Lua::DataBlock::FindBlock));
	modUtil[defDataBlock];

	// Version
	auto defVersion = pragma::LuaCore::register_class<util::Version>(lua.GetState(), "Version");
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
	auto defPath = pragma::LuaCore::register_class<util::Path>(lua.GetState(), "Path");
	defPath->scope[luabind::def("CreateFilePath", static_cast<void (*)(lua::State *, const std::string &)>([](lua::State *l, const std::string &path) { Lua::Push<util::Path>(l, util::Path::CreateFile(path)); }))];
	defPath->scope[luabind::def("CreateFilePath", static_cast<void (*)(lua::State *, const util::Path &)>([](lua::State *l, const util::Path &path) { Lua::Push<util::Path>(l, util::Path::CreateFile(path.GetString())); }))];
	defPath->scope[luabind::def("CreatePath", static_cast<void (*)(lua::State *, const std::string &)>([](lua::State *l, const std::string &path) { Lua::Push<util::Path>(l, util::Path::CreatePath(path)); }))];
	defPath->scope[luabind::def("CreatePath", static_cast<void (*)(lua::State *, const util::Path &)>([](lua::State *l, const util::Path &path) { Lua::Push<util::Path>(l, util::Path::CreatePath(path.GetString())); }))];
	defPath->scope[luabind::def("CreateFromComponents", static_cast<void (*)(lua::State *, luabind::object)>([](lua::State *l, luabind::object o) {
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
	
	defPath->def(luabind::self / luabind::const_self);
	defPath->def(luabind::self / std::string {});

	defPath->def(luabind::const_self == luabind::const_self);
	defPath->def(luabind::const_self == std::string {});

	defPath->def("Copy", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::Push<util::Path>(l, p); }));
	defPath->def("ToComponents", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) {
		auto components = p.ToComponents();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &c : components) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, c);
			Lua::SetTableValue(l, t);
		}
	}));
	defPath->def("GetString", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushString(l, p.GetString()); }));
	defPath->def("GetPath", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetPath()}); }));
	defPath->def("GetFileName", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetFileName()}); }));
	defPath->def("GetFront", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetFront()}); }));
	defPath->def("GetBack", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushString(l, std::string {p.GetBack()}); }));
	defPath->def("MoveUp", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { p.MoveUp(); }));
	defPath->def("PopFront", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { p.PopFront(); }));
	defPath->def("PopBack", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { p.PopBack(); }));
	defPath->def("Canonicalize", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { p.Canonicalize(); }));
	defPath->def("IsFile", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushBool(l, p.IsFile()); }));
	defPath->def("IsPath", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { Lua::PushBool(l, !p.IsFile()); }));
	defPath->def("GetFileExtension", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) {
		auto ext = p.GetFileExtension();
		if(ext.has_value() == false)
			return;
		Lua::PushString(l, *ext);
	}));
	defPath->def("RemoveFileExtension", static_cast<void (*)(lua::State *, util::Path &)>([](lua::State *l, util::Path &p) { p.RemoveFileExtension(); }));
	defPath->def("RemoveFileExtension", +[](lua::State *l, util::Path &p, const std::vector<std::string> &extensions) { p.RemoveFileExtension(extensions); });
	defPath->def("MakeRelative", +[](lua::State *l, util::Path &p, util::Path &pOther) { return p.MakeRelative(pOther); });
	defPath->def("MakeRelative", +[](lua::State *l, util::Path &p, const std::string &other) { return p.MakeRelative(other); });
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

	Lua::SetTableCFunction(lua.GetState(), "util", "FilePath", util_file_path);
	Lua::SetTableCFunction(lua.GetState(), "util", "DirPath", util_dir_path);

	// Properties
	Lua::Property::register_classes(lua);

	auto &modMath = lua.RegisterLibrary("math");

	// Transform
	auto classDefTransform = luabind::class_<math::Transform>("Transform");
	classDefTransform.def(luabind::constructor<const Mat4 &>());
	classDefTransform.def(luabind::constructor<const Vector3 &, const Quat &>());
	classDefTransform.def(luabind::constructor<const Vector3 &>());
	classDefTransform.def(luabind::constructor<const Quat &>());
	classDefTransform.def(luabind::constructor<const math::ScaledTransform &>());
	classDefTransform.def(luabind::constructor<>());
	classDefTransform.def(luabind::const_self == luabind::const_self);
	classDefTransform.def(luabind::tostring(luabind::self));
	classDefTransform.def("Copy", static_cast<math::Transform (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &t) -> math::Transform { return t; }));
	classDefTransform.property("x", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetOrigin().x; }),
	  static_cast<void (*)(lua::State *, math::Transform &, float)>([](lua::State *l, math::Transform &pose, float x) { pose.GetOrigin().x = x; }));
	classDefTransform.property("y", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetOrigin().y; }),
	  static_cast<void (*)(lua::State *, math::Transform &, float)>([](lua::State *l, math::Transform &pose, float y) { pose.GetOrigin().y = y; }));
	classDefTransform.property("z", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetOrigin().z; }),
	  static_cast<void (*)(lua::State *, math::Transform &, float)>([](lua::State *l, math::Transform &pose, float z) { pose.GetOrigin().z = z; }));
	classDefTransform.property("pitch", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetAngles().p; }));
	classDefTransform.property("yaw", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetAngles().y; }));
	classDefTransform.property("roll", static_cast<float (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &pose) { return pose.GetAngles().r; }));
	classDefTransform.def("ToPlane", &math::Transform::ToPlane);
	classDefTransform.def("GetAngles", &math::Transform::GetAngles);
	classDefTransform.def("SetAngles", &math::Transform::SetAngles);
	classDefTransform.def("GetForward", &math::Transform::GetForward);
	classDefTransform.def("GetRight", &math::Transform::GetRight);
	classDefTransform.def("GetUp", &math::Transform::GetUp);
	classDefTransform.def("Set", static_cast<void (*)(lua::State *, math::Transform &, const math::Transform &)>([](lua::State *l, math::Transform &t, const math::Transform &tOther) { t = tOther; }));
	classDefTransform.def("GetOrigin", static_cast<Vector3 &(math::Transform::*)()>(&math::Transform::GetOrigin), luabind::copy_policy<0> {});
	classDefTransform.def("GetRotation", static_cast<Quat &(math::Transform::*)()>(&math::Transform::GetRotation), luabind::copy_policy<0> {});
	classDefTransform.def("SetOrigin", &math::Transform::SetOrigin);
	classDefTransform.def("SetRotation", &math::Transform::SetRotation);
	classDefTransform.def("SetIdentity", &math::Transform::SetIdentity);
	classDefTransform.def("IsIdentity", static_cast<bool (*)(lua::State *, math::Transform &)>([](lua::State *l, math::Transform &t) -> bool {
		auto &origin = t.GetOrigin();
		auto &rotation = t.GetRotation();
		return math::abs(origin.x) < 0.001f && math::abs(origin.y) < 0.001f && math::abs(origin.z) < 0.001f && math::abs(1.f - rotation.w) < 0.001f && math::abs(rotation.x) < 0.001f && math::abs(rotation.y) < 0.001f && math::abs(rotation.z) < 0.001f;
	}));
	classDefTransform.def("TranslateGlobal", &math::Transform::TranslateGlobal);
	classDefTransform.def("TranslateLocal", &math::Transform::TranslateLocal);
	classDefTransform.def("RotateGlobal", &math::Transform::RotateGlobal);
	classDefTransform.def("RotateLocal", &math::Transform::RotateLocal);
	classDefTransform.def("TransformGlobal", static_cast<void (*)(lua::State *, math::Transform &, const math::Transform &)>([](lua::State *l, math::Transform &t, const math::Transform &t2) { t = t2 * t; }));
	classDefTransform.def("TransformLocal", static_cast<void (*)(lua::State *, math::Transform &, const math::Transform &)>([](lua::State *l, math::Transform &t, const math::Transform &t2) { t *= t2; }));
	classDefTransform.def("GetInverse", &math::Transform::GetInverse);
	classDefTransform.def("ToMatrix", &math::Transform::ToMatrix);
	classDefTransform.def("SetMatrix", static_cast<void (*)(lua::State *, math::Transform &, const Mat4 &)>([](lua::State *l, math::Transform &t, const Mat4 &m) {
		Mat4 transformation;
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::gtx::decompose(transformation, scale, rotation, translation, skew, perspective);
		t.SetOrigin(translation);
		t.SetRotation(rotation);
	}));
	classDefTransform.def("Interpolate", &math::Transform::Interpolate);
	classDefTransform.def("InterpolateToIdentity", &math::Transform::InterpolateToIdentity);
	classDefTransform.def("Reflect", &math::Transform::Reflect);
	classDefTransform.def(luabind::const_self * luabind::const_self);
	classDefTransform.def(luabind::const_self * math::ScaledTransform());
	classDefTransform.def(luabind::const_self * Vector3());
	classDefTransform.def(luabind::const_self * Quat());
	classDefTransform.def(luabind::const_self * math::Plane());

	modMath[classDefTransform];
	Lua::RegisterLibraryValues<math::Transform>(lua.GetState(), "math.Transform", {std::pair<std::string, math::Transform> {"IDENTITY", math::Transform {}}});

	auto classDefScaledTransform = luabind::class_<math::ScaledTransform, math::Transform>("ScaledTransform");
	classDefScaledTransform.def(luabind::constructor<const Mat4 &>());
	classDefScaledTransform.def(luabind::constructor<const Vector3 &, const Quat &>());
	classDefScaledTransform.def(luabind::constructor<const Vector3 &, const Quat &, const Vector3 &>());
	classDefScaledTransform.def(luabind::constructor<const math::Transform &, const Vector3 &>());
	classDefScaledTransform.def(luabind::constructor<const math::Transform &>());
	classDefScaledTransform.def(luabind::constructor<>());
	classDefScaledTransform.def(luabind::const_self == luabind::const_self);
	classDefScaledTransform.def(luabind::tostring(luabind::self));
	classDefScaledTransform.def("Copy", static_cast<math::ScaledTransform (*)(lua::State *, math::ScaledTransform &)>([](lua::State *l, math::ScaledTransform &t) -> math::ScaledTransform { return t; }));
	classDefScaledTransform.def("Set", static_cast<void (*)(lua::State *, math::ScaledTransform &, const math::ScaledTransform &)>([](lua::State *l, math::ScaledTransform &t, const math::ScaledTransform &tOther) { t = tOther; }));
	classDefScaledTransform.def("GetScale", static_cast<Vector3 &(math::ScaledTransform::*)()>(&math::ScaledTransform::GetScale), luabind::copy_policy<0> {});
	classDefScaledTransform.def("SetScale", &math::ScaledTransform::SetScale);
	classDefScaledTransform.def("Scale", &math::ScaledTransform::Scale);
	classDefScaledTransform.def("GetInverse", &math::ScaledTransform::GetInverse);
	classDefScaledTransform.def("Interpolate", &math::ScaledTransform::Interpolate);
	classDefScaledTransform.def("InterpolateToIdentity", &math::ScaledTransform::InterpolateToIdentity);
	classDefScaledTransform.def(luabind::const_self * math::Transform());
	classDefScaledTransform.def(luabind::const_self * math::ScaledTransform()); // Note: We use pragma::math::ScaledTransform instead of luabind::const_self, because otherwise the overload of the base class ("Transform") would be used if two ScaledTransforms are multiplied
	classDefScaledTransform.def(luabind::const_self * Vector3());
	classDefScaledTransform.def(luabind::const_self * Quat());
	classDefScaledTransform.def(luabind::const_self * math::Plane());
	modMath[classDefScaledTransform];
	Lua::RegisterLibraryValues<math::ScaledTransform>(lua.GetState(), "math.ScaledTransform", {std::pair<std::string, math::ScaledTransform> {"IDENTITY", math::ScaledTransform {}}});

	// PID Controller
	auto defPIDController = luabind::class_<math::PIDController>("PIDController");
	defPIDController.def(luabind::constructor<>());
	defPIDController.def(luabind::constructor<float, float, float>());
	defPIDController.def(luabind::constructor<float, float, float, float, float>());
	defPIDController.def("SetProportionalTerm", &math::PIDController::SetProportionalTerm);
	defPIDController.def("SetIntegralTerm", &math::PIDController::SetIntegralTerm);
	defPIDController.def("SetDerivativeTerm", &math::PIDController::SetDerivativeTerm);
	defPIDController.def("SetTerms", &math::PIDController::SetTerms);
	defPIDController.def("GetProportionalTerm", &math::PIDController::GetProportionalTerm);
	defPIDController.def("GetIntegralTerm", &math::PIDController::GetIntegralTerm);
	defPIDController.def("GetDerivativeTerm", &math::PIDController::GetDerivativeTerm);
	defPIDController.def("GetTerms", static_cast<void (*)(lua::State *, const math::PIDController &)>([](lua::State *l, const math::PIDController &pidController) {
		auto p = 0.f;
		auto i = 0.f;
		auto d = 0.f;
		pidController.GetTerms(p, i, d);
		Lua::PushNumber(l, p);
		Lua::PushNumber(l, i);
		Lua::PushNumber(l, d);
	}));
	defPIDController.def("SetRange", &math::PIDController::SetRange);
	defPIDController.def("GetRange", static_cast<void (*)(lua::State *, const math::PIDController &)>([](lua::State *l, const math::PIDController &pidController) {
		auto range = pidController.GetRange();
		Lua::PushNumber(l, range.first);
		Lua::PushNumber(l, range.second);
	}));
	defPIDController.def("Calculate", &math::PIDController::Calculate);
	defPIDController.def("Reset", &math::PIDController::Reset);
	defPIDController.def("ClearRange", &math::PIDController::ClearRange);
	defPIDController.def("SetMin", &math::PIDController::SetMin);
	defPIDController.def("SetMax", &math::PIDController::SetMax);
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

	auto noiseMap = luabind::class_<utils::NoiseMap>("NoiseMap");
	noiseMap.def("GetValue", &Lua::noise::NoiseMap::GetValue);
	noiseMap.def("GetHeight", &Lua::noise::NoiseMap::GetHeight);
	noiseMap.def("GetWidth", &Lua::noise::NoiseMap::GetWidth);
	modMath[noiseMap];
	//

	auto defVectori = pragma::LuaCore::register_class<Vector3i>(lua.GetState(), "Vectori", &glm_type_to_string<Vector3i>);
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
	defVectori->def("Get", static_cast<void (*)(lua::State *, const Vector3i &, uint32_t)>([](lua::State *l, const Vector3i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVectori];
	register_string_to_vector_type_constructor<Vector3i>(lua.GetState());

	auto defVector2i = pragma::LuaCore::register_class<Vector2i>(lua.GetState(), "Vector2i", &glm_type_to_string<Vector2i>);
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
	defVector2i->def("Get", static_cast<void (*)(lua::State *, const Vector2i &, uint32_t)>([](lua::State *l, const Vector2i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVector2i];
	register_string_to_vector_type_constructor<Vector2i>(lua.GetState());

	auto defVector4i = pragma::LuaCore::register_class<Vector4i>(lua.GetState(), "Vector4i", &glm_type_to_string<Vector4i>);
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
	defVector4i->def("Get", static_cast<void (*)(lua::State *, const Vector4i &, uint32_t)>([](lua::State *l, const Vector4i &v, uint32_t idx) { Lua::PushInt(l, v[idx]); }));
	modMath[*defVector4i];
	register_string_to_vector_type_constructor<Vector4i>(lua.GetState());

	auto defVector = pragma::LuaCore::register_class<Vector3>(lua.GetState(), "Vector", &glm_type_to_string<Vector3>);
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
	//defVector->def(luabind::const_self *pragma::math::Transform());
	//defVector->def(luabind::const_self *pragma::math::ScaledTransform());
	defVector->def("Mul", static_cast<void (*)(lua::State *, const Vector3 &, const math::Transform &)>([](lua::State *l, const Vector3 &a, const math::Transform &b) { Lua::Push<Vector3>(l, a * b); }));
	defVector->def("Mul", static_cast<void (*)(lua::State *, const Vector3 &, const math::ScaledTransform &)>([](lua::State *l, const Vector3 &a, const math::ScaledTransform &b) { Lua::Push<Vector3>(l, a * b); }));
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
	defVector->def("Rotate", static_cast<void (*)(lua::State *, Vector3 &, const EulerAngles &)>(&Lua::Vector::Rotate));
	defVector->def("Rotate", static_cast<void (*)(lua::State *, Vector3 &, const Vector3 &, float)>(&Lua::Vector::Rotate));
	defVector->def("Rotate", static_cast<void (*)(lua::State *, Vector3 &, const Quat &)>(&Lua::Vector::Rotate));
	defVector->def("RotateAround", &Lua::Vector::RotateAround);
	defVector->def("Lerp", &Lua::Vector::Lerp);
	defVector->def("Reflect", &uvec::reflect);
	defVector->def("Equals",
	  static_cast<void (*)(lua::State *, const Vector3 &, const Vector3 &, float)>([](lua::State *l, const Vector3 &a, const Vector3 &b, float epsilon) { Lua::PushBool(l, math::abs(a.x - b.x) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.z - b.z) <= epsilon); }));
	defVector->def("Equals", static_cast<void (*)(lua::State *, const Vector3 &, const Vector3 &)>([](lua::State *l, const Vector3 &a, const Vector3 &b) {
		float epsilon = 0.001f;
		Lua::PushBool(l, math::abs(a.x - b.x) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.z - b.z) <= epsilon);
	}));
	defVector->def("GetAngle", static_cast<float (*)(lua::State *, const Vector3 &, const Vector3 &)>([](lua::State *l, const Vector3 &a, const Vector3 &b) -> float { return math::deg_to_rad(uvec::get_angle(a, b)); }));
	defVector->def("Slerp", static_cast<void (*)(lua::State *, const Vector3 &, const Vector3 &, float)>([](lua::State *l, const Vector3 &a, const Vector3 &b, float factor) {
		auto result = glm::gtc::slerp(a, b, factor);
		Lua::Push<Vector3>(l, result);
	}));
	defVector->def("Copy", &Lua::Vector::Copy);
	defVector->def("Set", static_cast<void (*)(lua::State *, Vector3 &, const Vector3 &)>(&Lua::Vector::Set));
	defVector->def("Set", static_cast<void (*)(lua::State *, Vector3 &, float, float, float)>(&Lua::Vector::Set));
	defVector->def("Set", static_cast<void (*)(lua::State *, Vector3 &, uint32_t, float)>([](lua::State *l, Vector3 &v, uint32_t idx, float val) { v[idx] = val; }));
	defVector->def("Get", static_cast<void (*)(lua::State *, const Vector3 &, uint32_t)>([](lua::State *l, const Vector3 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector->def("GetYaw", &uvec::get_yaw);
	defVector->def("GetPitch", &uvec::get_pitch);
	defVector->def("ToMatrix", &Lua::Vector::ToMatrix);
	defVector->def("SnapToGrid", static_cast<void (*)(lua::State *, Vector3 &)>(&Lua::Vector::SnapToGrid));
	defVector->def("SnapToGrid", static_cast<void (*)(lua::State *, Vector3 &, UInt32)>(&Lua::Vector::SnapToGrid));
	defVector->def("Project", uvec::project);
	defVector->def("ProjectToPlane", uvec::project_to_plane);
	defVector->def("GetPerpendicular", uvec::get_perpendicular);
	defVector->def("OuterProduct", &uvec::calc_outer_product);
	defVector->def("ToScreenUv", &umat::to_screen_uv);
	modMath[*defVector];
	register_string_to_vector_type_constructor<Vector3>(lua.GetState());

	auto defVector2 = pragma::LuaCore::register_class<Vector2>(lua.GetState(), "Vector2", &glm_type_to_string<Vector2>);
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
	defVector2->def("GetAngle", static_cast<float (*)(lua::State *, const Vector2 &, const Vector2 &)>([](lua::State *l, const Vector2 &a, const Vector2 &b) -> float { return math::deg_to_rad(uvec::get_angle(Vector3 {a, 0.f}, Vector3 {b, 0.f})); }));
	defVector2->def("Set", static_cast<void (*)(lua::State *, Vector2 &, const Vector2 &)>(&Lua::Vector2::Set));
	defVector2->def("Set", static_cast<void (*)(lua::State *, Vector2 &, float, float)>(&Lua::Vector2::Set));
	defVector2->def("Get", static_cast<void (*)(lua::State *, const Vector2 &, uint32_t)>([](lua::State *l, const Vector2 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector2->def("Project", &Lua::Vector2::Project);
	modMath[*defVector2];
	register_string_to_vector_type_constructor<Vector2>(lua.GetState());

	auto defVector4 = pragma::LuaCore::register_class<Vector4>(lua.GetState(), "Vector4", &glm_type_to_string<Vector4>);
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
	defVector4->def("Set", static_cast<void (*)(lua::State *, Vector4 &, const Vector4 &)>(&Lua::Vector4::Set));
	defVector4->def("Set", static_cast<void (*)(lua::State *, Vector4 &, float, float, float, float)>(&Lua::Vector4::Set));
	defVector4->def("Get", static_cast<void (*)(lua::State *, const Vector4 &, uint32_t)>([](lua::State *l, const Vector4 &v, uint32_t idx) { Lua::PushNumber(l, v[idx]); }));
	defVector4->def("Project", &Lua::Vector4::Project);
	modMath[*defVector4];
	register_string_to_vector_type_constructor<Vector4>(lua.GetState());

	auto defEulerAngles = pragma::LuaCore::register_class<EulerAngles>(lua.GetState(), "EulerAngles", &glm_type_to_string<EulerAngles>);
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
	  static_cast<bool (*)(lua::State *, const EulerAngles &, const EulerAngles &, float)>([](lua::State *l, const EulerAngles &a, const EulerAngles &b, float epsilon) { return math::abs(a.p - b.p) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.r - b.r) <= epsilon; }));
	defEulerAngles->def("Equals", static_cast<bool (*)(lua::State *, const EulerAngles &, const EulerAngles &)>([](lua::State *l, const EulerAngles &a, const EulerAngles &b) {
		float epsilon = 0.001f;
		return math::abs(a.p - b.p) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.r - b.r) <= epsilon;
	}));
	defEulerAngles->def("ToQuaternion", Lua::Angle::ToQuaternion);
	defEulerAngles->def("ToQuaternion", static_cast<void (*)(lua::State *, const EulerAngles &)>([](lua::State *l, const EulerAngles &ang) { Lua::Angle::ToQuaternion(l, ang, math::to_integral(RotationOrder::YXZ)); }));
	defEulerAngles->def("Set", static_cast<void (EulerAngles::*)(const EulerAngles &)>(&EulerAngles::Set));
	defEulerAngles->def("Set", &Lua::Angle::Set);
	defEulerAngles->def("Set", static_cast<void (*)(lua::State *, EulerAngles &, uint32_t, float value)>([](lua::State *l, EulerAngles &ang, uint32_t idx, float value) { ang[idx] = value; }));
	defEulerAngles->def("Get", static_cast<float (*)(lua::State *, const EulerAngles &, uint32_t)>([](lua::State *l, const EulerAngles &ang, uint32_t idx) { return ang[idx]; }));
	modMath[*defEulerAngles];

	auto defQuat = pragma::LuaCore::register_class<Quat>(lua.GetState(), "Quaternion", &glm_type_to_string<Quat>);
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
	//defQuat->def(luabind::const_self *pragma::math::Transform());
	//defQuat->def(luabind::const_self *pragma::math::ScaledTransform());
	defQuat->def("Mul", static_cast<Quat (*)(lua::State *, const Quat &, const math::Transform &)>([](lua::State *l, const Quat &a, const math::Transform &b) { return a * b; }));
	defQuat->def("Mul", static_cast<Quat (*)(lua::State *, const Quat &, const math::ScaledTransform &)>([](lua::State *l, const Quat &a, const math::ScaledTransform &b) { return a * b; }));
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
	defQuat->def("ToMatrix", static_cast<Mat4 (*)(const Quat &)>(&glm::gtx::toMat4));
	defQuat->def("ToMatrix3", static_cast<Mat3 (*)(const Quat &)>(&glm::gtx::toMat3));
	defQuat->def("Lerp", &uquat::lerp);
	defQuat->def("Slerp", &uquat::slerp);
	defQuat->def("ToEulerAngles", Lua::Quaternion::ToEulerAngles);
	defQuat->def("ToEulerAngles", static_cast<EulerAngles (*)(lua::State *, Quat &)>([](lua::State *l, Quat &rot) { return Lua::Quaternion::ToEulerAngles(l, rot, math::to_integral(RotationOrder::YXZ)); }));
	defQuat->def("ToAxisAngle", &Lua::Quaternion::ToAxisAngle);
	defQuat->def(
	  "GetAxisVector", +[](const Quat &rot, SignedAxis axis) {
		  auto dir = axis_to_vector(axis);
		  uvec::rotate(&dir, rot);
		  return dir;
	  });
	defQuat->def("Set", &Lua::Quaternion::Set);
	defQuat->def("Set", static_cast<void (*)(Quat &, const Quat &)>([](Quat &rot, const Quat &rotNew) { rot = rotNew; }));
	defQuat->def("Set", static_cast<void (*)(Quat &, uint32_t, float)>([](Quat &rot, uint32_t idx, float value) {
		constexpr std::array<uint8_t, 4> quatIndices = {3, 0, 1, 2};
		rot[quatIndices.at(idx)] = value;
	}));
	defQuat->def("Get", static_cast<void (*)(lua::State *, Quat &, uint32_t)>([](lua::State *l, Quat &rot, uint32_t idx) {
		constexpr std::array<uint8_t, 4> quatIndices = {3, 0, 1, 2};
		Lua::PushNumber(l, rot[quatIndices.at(idx)]);
	}));
	defQuat->def("RotateX", static_cast<void (*)(Quat &, float)>(&uquat::rotate_x));
	defQuat->def("RotateY", static_cast<void (*)(Quat &, float)>(&uquat::rotate_y));
	defQuat->def("RotateZ", static_cast<void (*)(Quat &, float)>(&uquat::rotate_z));
	defQuat->def("Rotate", static_cast<void (*)(Quat &, const Vector3 &, float)>(&uquat::rotate));
	defQuat->def("Rotate", static_cast<void (*)(Quat &, const EulerAngles &)>(&uquat::rotate));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, Vector2> (*)(lua::State *, const Quat &, const Vector3 &, const Vector3 &, const Vector2 &, const Vector2 *, const Vector2 *, const Quat *, const EulerAngles *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, Vector2> (*)(lua::State *, const Quat &, const Vector3 &, const Vector3 &, const Vector2 &, const Vector2 *, const Vector2 *, const Quat *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, Vector2> (*)(lua::State *, const Quat &, const Vector3 &, const Vector3 &, const Vector2 &, const Vector2 *, const Vector2 *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, Vector2> (*)(lua::State *, const Quat &, const Vector3 &, const Vector3 &, const Vector2 &, const Vector2 *)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ApproachDirection", static_cast<luabind::mult<Quat, Vector2> (*)(lua::State *, const Quat &, const Vector3 &, const Vector3 &, const Vector2 &)>(&Lua::Quaternion::ApproachDirection));
	defQuat->def("ClampRotation", static_cast<Quat (*)(lua::State *, Quat &, const EulerAngles &, const EulerAngles &)>([](lua::State *l, Quat &rot, const EulerAngles &minBounds, const EulerAngles &maxBounds) -> Quat { return uquat::clamp_rotation(rot, minBounds, maxBounds); }));
	defQuat->def("ClampRotation", static_cast<Quat (*)(lua::State *, Quat &, const EulerAngles &)>([](lua::State *l, Quat &rot, const EulerAngles &bounds) -> Quat { return uquat::clamp_rotation(rot, -bounds, bounds); }));
	defQuat->def("Distance", &uquat::distance);
	defQuat->def("GetConjugate", static_cast<Quat (*)(const Quat &)>(&glm::gtc::conjugate));
	defQuat->def("AlignToAxis", &uquat::align_rotation_to_axis);
	defQuat->def("Equals", +[](const Quat &a, const Quat &b, float epsilon) { return math::abs(a.x - b.x) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.z - b.z) <= epsilon && math::abs(a.w - b.w) <= epsilon; });
	defQuat->def(
	  "Equals", +[](const Quat &a, const Quat &b) {
		  float epsilon = 0.001f;
		  return math::abs(a.x - b.x) <= epsilon && math::abs(a.y - b.y) <= epsilon && math::abs(a.z - b.z) <= epsilon && math::abs(a.w - b.w) <= epsilon;
	  });
	modMath[*defQuat];
	pragma::LuaCore::define_custom_constructor<Quat, &uquat::identity>(lua.GetState());
	pragma::LuaCore::define_custom_constructor<Quat, static_cast<Quat (*)(const Vector3 &, float)>(&uquat::create), const Vector3 &, float>(lua.GetState());
	pragma::LuaCore::define_custom_constructor<Quat,
	  +[](const Vector3 &a, const Vector3 &b, const Vector3 &c) -> Quat {
		  auto m = umat::create_from_axes(a, b, c);
		  return Quat(m);
	  },
	  const Vector3 &, const Vector3 &, const Vector3 &>(lua.GetState());
	pragma::LuaCore::define_custom_constructor<Quat, &uquat::create_look_rotation, const Vector3 &, const Vector3 &>(lua.GetState());

	auto _G = luabind::globals(lua.GetState());
	_G["Vector2i"] = _G["math"]["Vector2i"];
	_G["Vector"] = _G["math"]["Vector"];
	_G["Vector2"] = _G["math"]["Vector2"];
	_G["Vector4"] = _G["math"]["Vector4"];
	_G["EulerAngles"] = _G["math"]["EulerAngles"];
	_G["Quaternion"] = _G["math"]["Quaternion"];

	RegisterLuaMatrices(lua);
	//modelMeshClassDef.scope[luabind::def("Create",&Lua::ModelMesh::Client::Create)];
}

static bool operator==(const EntityHandle &v, const LEntityProperty &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LEntityProperty &v)
{
	if((*v)->valid())
		const_cast<pragma::ecs::BaseEntity *>((*v)->get())->print(str);
	else
		str << "NULL";
	return str;
}
void Lua::Property::push(lua::State *l, pragma::EntityProperty &prop) { Lua::Property::push_property<LEntityPropertyWrapper>(l, prop); }
namespace pragma::LuaCore {
	void register_thread_pool(lua::State *l, luabind::module_ &modUtil);
};

void pragma::Game::RegisterLuaClasses()
{
	NetworkState::RegisterSharedLuaClasses(GetLuaInterface());

	// Entity
	auto &modUtil = GetLuaInterface().RegisterLibrary("util");
	LuaCore::register_thread_pool(GetLuaState(), modUtil);
	auto entDef = luabind::class_<LEntityProperty, LBasePropertyWrapper>("EntityProperty");
	//Lua::Property::add_generic_methods<LEntityProperty,EntityHandle,luabind::class_<LEntityProperty,LBasePropertyWrapper>>(entDef);
	entDef.def(luabind::constructor<>());
	entDef.def(luabind::constructor<EntityHandle>());
	entDef.def(luabind::tostring(luabind::const_self));
	entDef.def("Link", static_cast<void (*)(lua::State *, LEntityProperty &, LEntityProperty &)>(&Lua::Property::link<LEntityProperty, EntityHandle>));
	modUtil[entDef];

	auto defSplashDamageInfo = luabind::class_<util::SplashDamageInfo>("SplashDamageInfo");
	defSplashDamageInfo.def(luabind::constructor<>());
	defSplashDamageInfo.def_readwrite("origin", &util::SplashDamageInfo::origin);
	defSplashDamageInfo.def_readwrite("radius", &util::SplashDamageInfo::radius);
	defSplashDamageInfo.def_readwrite("damageInfo", &util::SplashDamageInfo::damageInfo);
	defSplashDamageInfo.def("SetCone",
	  static_cast<void (*)(lua::State *, util::SplashDamageInfo &, const Vector3 &, float)>([](lua::State *l, util::SplashDamageInfo &splashDamageInfo, const Vector3 &coneDirection, float coneAngle) { splashDamageInfo.cone = {{coneDirection, coneAngle}}; }));
	defSplashDamageInfo.def("SetCallback", static_cast<void (*)(lua::State *, util::SplashDamageInfo &, luabind::object)>([](lua::State *l, util::SplashDamageInfo &splashDamageInfo, luabind::object oCallback) {
		Lua::CheckFunction(l, 2);
		splashDamageInfo.callback = [l, oCallback](ecs::BaseEntity *ent, game::DamageInfo &dmgInfo) -> bool {
			auto r = Lua::CallFunction(
			  l,
			  [ent, &dmgInfo, &oCallback](lua::State *l) -> Lua::StatusCode {
				  oCallback.push(l);
				  if(ent != nullptr)
					  ent->GetLuaObject().push(l);
				  else
					  Lua::PushNil(l);
				  Lua::Push<game::DamageInfo *>(l, &dmgInfo);
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
	    {"COORDINATE_SPACE_LOCAL", pragma::math::to_integral(math::CoordinateSpace::Local)},
	    {"COORDINATE_SPACE_WORLD", pragma::math::to_integral(math::CoordinateSpace::World)},
	    {"COORDINATE_SPACE_OBJECT", pragma::math::to_integral(math::CoordinateSpace::Object)},
	    {"COORDINATE_SPACE_VIEW", pragma::math::to_integral(math::CoordinateSpace::View)},
	    {"COORDINATE_SPACE_SCREEN", pragma::math::to_integral(math::CoordinateSpace::Screen)},
	  });
	modMath[luabind::def("coordinate_space_to_string", +[](math::CoordinateSpace space) -> std::string { return std::string {magic_enum::enum_name(space)}; })];
	modMath[luabind::def("string_to_coordinate_space", +[](const std::string &space) -> std::optional<math::CoordinateSpace> { return magic_enum::enum_cast<math::CoordinateSpace>(space); })];

	auto defPlane = luabind::class_<math::Plane>("Plane");
	defPlane.def(luabind::constructor<Vector3, Vector3, Vector3>());
	defPlane.def(luabind::constructor<Vector3, Vector3>());
	defPlane.def(luabind::constructor<Vector3, double>());
	defPlane.def(
	  "__tostring", +[](math::Plane &plane) -> std::string {
		  std::stringstream ss;
		  ss << "Plane[" << plane.GetNormal() << "][" << plane.GetDistance() << "]";
		  return ss.str();
	  });
	defPlane.def("Copy", static_cast<void (*)(lua::State *, math::Plane &)>([](lua::State *l, math::Plane &plane) { Lua::Push<math::Plane>(l, math::Plane {plane}); }));
	defPlane.def("SetNormal", &math::Plane::SetNormal);
	defPlane.def("SetDistance", &math::Plane::SetDistance);
	defPlane.def("GetNormal", static_cast<const Vector3 &(math::Plane::*)() const>(&math::Plane::GetNormal), luabind::copy_policy<0> {});
	defPlane.def("GetPos", static_cast<Vector3 (*)(const math::Plane &)>([](const math::Plane &plane) { return plane.GetPos(); }));
	defPlane.def("GetDistance", static_cast<double (math::Plane::*)() const>(&math::Plane::GetDistance));
	defPlane.def("GetDistance", static_cast<float (math::Plane::*)(const Vector3 &) const>(&math::Plane::GetDistance));
	defPlane.def("MoveToPos", static_cast<void (*)(math::Plane &, const Vector3 &pos)>([](math::Plane &plane, const Vector3 &pos) { plane.MoveToPos(pos); }));
	defPlane.def("Rotate", &math::Plane::Rotate);
	defPlane.def("GetCenterPos", static_cast<Vector3 (*)(const math::Plane &)>([](const math::Plane &plane) { return plane.GetCenterPos(); }));
	defPlane.def("Transform", static_cast<void (*)(lua::State *, math::Plane &, const Mat4 &)>([](lua::State *l, math::Plane &plane, const Mat4 &transform) {
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
		plane = math::Plane {Vector3 {p04.x, p04.y, p04.z}, Vector3 {p14.x, p14.y, p14.z}, Vector3 {p24.x, p24.y, p24.z}};
	}));
	modMath[defPlane];
}

LuaEntityIterator Lua::ents::create_lua_entity_iterator(lua::State *l, const tb<LuaEntityIteratorFilterBase> &filterTable, pragma::ecs::EntityIterator::FilterFlags filterFlags)
{
	auto r = LuaEntityIterator {l, filterFlags};
	if(filterTable) {
		filterTable.push(l);
		luabind::detail::stack_pop sp {l, 1};

		auto t = GetStackTop(l);
		CheckTable(l, t);
		auto numFilters = GetObjectLength(l, t);
		for(auto i = decltype(numFilters) {0u}; i < numFilters; ++i) {
			PushInt(l, i + 1u);
			GetTableValue(l, t);

			auto &filter = Lua::Check<LuaEntityIteratorFilterBase>(l, -1);
			r.AttachFilter(filter);

			Pop(l, 1);
		}
	}
	return r;
}

LuaEntityComponentIterator Lua::ents::create_lua_entity_component_iterator(lua::State *l, pragma::ComponentId componentId, const tb<LuaEntityIteratorFilterBase> &filterTable, pragma::ecs::EntityIterator::FilterFlags filterFlags)
{
	auto r = LuaEntityComponentIterator {l, componentId, filterFlags};
	if(filterTable) {
		filterTable.push(l);
		luabind::detail::stack_pop sp {l, 1};

		auto t = GetStackTop(l);
		CheckTable(l, t);
		auto numFilters = GetObjectLength(l, t);
		for(auto i = decltype(numFilters) {0u}; i < numFilters; ++i) {
			PushInt(l, i + 1u);
			GetTableValue(l, t);

			auto &filter = Lua::Check<LuaEntityIteratorFilterBase>(l, -1);
			r.AttachFilter(filter);

			Pop(l, 1);
		}
	}
	return r;
}

struct LuaEntityIteratorFilterFunction : public LuaEntityIteratorFilterBase {
	LuaEntityIteratorFilterFunction(Lua::func<bool> oFunc);
	virtual void Attach(pragma::ecs::EntityIterator &iterator) override;
  private:
	luabind::object m_function;
};

LuaEntityIteratorFilterFunction::LuaEntityIteratorFilterFunction(Lua::func<bool> oFunc) : m_function {oFunc} {}
void LuaEntityIteratorFilterFunction::Attach(pragma::ecs::EntityIterator &iterator)
{
	auto *data = iterator.GetIteratorData();
	auto *components = (data && data->entities && typeid(*data->entities) == typeid(ComponentContainer)) ? static_cast<ComponentContainer *>(data->entities.get()) : nullptr;
	auto *l = m_function.interpreter();
	iterator.AttachFilter<EntityIteratorFilterUser>([this, l, components](pragma::ecs::BaseEntity &ent, std::size_t index) -> bool {
		auto r = Lua::CallFunction(
		  l,
		  [this, &ent, index, components](lua::State *l) -> Lua::StatusCode {
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

static LuaEntityComponentIterator &citerator(lua::State *l, pragma::ComponentId componentId)
{
	s_centIterator = LuaEntityComponentIterator {l, componentId};
	return *s_centIterator;
}
static LuaEntityComponentIterator &citerator(lua::State *l, pragma::ComponentId componentId, const Lua::var<pragma::ecs::EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags)
{
	auto filterFlags = pragma::ecs::EntityIterator::FilterFlags::Default;
	luabind::object filterTable {};
	if(Lua::IsNumber(l, 2))
		filterFlags = static_cast<pragma::ecs::EntityIterator::FilterFlags>(luabind::object_cast<uint32_t>(oFilterOrFlags));
	else
		filterTable = oFilterOrFlags;
	s_centIterator = Lua::ents::create_lua_entity_component_iterator(l, componentId, filterTable, filterFlags);
	return *s_centIterator;
}
static LuaEntityComponentIterator &citerator(lua::State *l, pragma::ComponentId componentId, pragma::ecs::EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter)
{
	s_centIterator = Lua::ents::create_lua_entity_component_iterator(l, componentId, oFilter, filterFlags);
	return *s_centIterator;
}

void pragma::Game::RegisterLuaGameClasses(luabind::module_ &gameMod)
{
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents");
	RegisterLuaEntityComponents(modEnts);
	modEnts[luabind::def(
	  "iterator",
	  +[](lua::State *l) -> LuaEntityIterator & {
		  s_entIterator = LuaEntityIterator {l};
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "iterator",
	  +[](lua::State *l, const Lua::var<ecs::EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags) -> LuaEntityIterator & {
		  auto filterFlags = ecs::EntityIterator::FilterFlags::Default;
		  luabind::object filterTable {};
		  if(Lua::IsNumber(l, 1))
			  filterFlags = static_cast<ecs::EntityIterator::FilterFlags>(luabind::object_cast<uint32_t>(oFilterOrFlags));
		  else
			  filterTable = oFilterOrFlags;
		  s_entIterator = Lua::ents::create_lua_entity_iterator(l, filterTable, filterFlags);
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "iterator",
	  +[](lua::State *l, ecs::EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter) -> LuaEntityIterator & {
		  s_entIterator = Lua::ents::create_lua_entity_iterator(l, oFilter, filterFlags);
		  return *s_entIterator;
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", +[](lua::State *l, Lua::nil_type) -> LuaEntityComponentIterator & { return citerator(l, INVALID_COMPONENT_ID); }, luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua::State *, ComponentId)>(&citerator), luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua::State *, ComponentId, const Lua::var<ecs::EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &)>(&citerator), luabind::return_stl_iterator {})];
	modEnts[luabind::def("citerator", static_cast<LuaEntityComponentIterator &(*)(lua::State *, ComponentId, ecs::EntityIterator::FilterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &)>(&citerator), luabind::return_stl_iterator {})];

	modEnts[luabind::def(
	  "citerator",
	  +[](lua::State *l, Game &game, const std::string &componentName) -> LuaEntityComponentIterator & {
		  ComponentId componentId;
		  if(!game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId))
			  Lua::Error(l, "Unknown component type '" + componentName + "'!");
		  return citerator(l, componentId);
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "citerator",
	  +[](lua::State *l, Game &game, const std::string &componentName, const Lua::var<ecs::EntityIterator::FilterFlags, Lua::tb<LuaEntityIteratorFilterBase>> &oFilterOrFlags) -> LuaEntityComponentIterator & {
		  ComponentId componentId;
		  if(!game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId))
			  Lua::Error(l, "Unknown component type '" + componentName + "'!");
		  return citerator(l, componentId, oFilterOrFlags);
	  },
	  luabind::return_stl_iterator {})];
	modEnts[luabind::def(
	  "citerator",
	  +[](lua::State *l, Game &game, const std::string &componentName, ecs::EntityIterator::FilterFlags filterFlags, const Lua::tb<LuaEntityIteratorFilterBase> &oFilter) -> LuaEntityComponentIterator & {
		  ComponentId componentId;
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
	defItFilterComponent.def(luabind::constructor<ComponentId>());
	defItFilterComponent.def(luabind::constructor<lua::State *, const std::string &>());
	modEnts[defItFilterComponent];

	auto defItFilterFunction = luabind::class_<LuaEntityIteratorFilterFunction, LuaEntityIteratorFilterBase>("IteratorFilterFunction");
	defItFilterFunction.def(luabind::constructor<Lua::func<bool>>());
	modEnts[defItFilterFunction];

	Lua::RegisterLibraryEnums(GetLuaState(), "ents",
	  {
	    {"ITERATOR_FILTER_BIT_NONE", math::to_integral(ecs::EntityIterator::FilterFlags::None)},
	    {"ITERATOR_FILTER_BIT_SPAWNED", math::to_integral(ecs::EntityIterator::FilterFlags::Spawned)},
	    {"ITERATOR_FILTER_BIT_PENDING", math::to_integral(ecs::EntityIterator::FilterFlags::Pending)},
	    {"ITERATOR_FILTER_BIT_INCLUDE_SHARED", math::to_integral(ecs::EntityIterator::FilterFlags::IncludeShared)},
	    {"ITERATOR_FILTER_BIT_INCLUDE_NETWORK_LOCAL", math::to_integral(ecs::EntityIterator::FilterFlags::IncludeNetworkLocal)},

	    {"ITERATOR_FILTER_BIT_CHARACTER", math::to_integral(ecs::EntityIterator::FilterFlags::Character)},
	    {"ITERATOR_FILTER_BIT_PLAYER", math::to_integral(ecs::EntityIterator::FilterFlags::Player)},
	    {"ITERATOR_FILTER_BIT_WEAPON", math::to_integral(ecs::EntityIterator::FilterFlags::Weapon)},
	    {"ITERATOR_FILTER_BIT_VEHICLE", math::to_integral(ecs::EntityIterator::FilterFlags::Vehicle)},
	    {"ITERATOR_FILTER_BIT_NPC", math::to_integral(ecs::EntityIterator::FilterFlags::NPC)},
	    {"ITERATOR_FILTER_BIT_PHYSICAL", math::to_integral(ecs::EntityIterator::FilterFlags::Physical)},
	    {"ITERATOR_FILTER_BIT_SCRIPTED", math::to_integral(ecs::EntityIterator::FilterFlags::Scripted)},
	    {"ITERATOR_FILTER_BIT_MAP_ENTITY", math::to_integral(ecs::EntityIterator::FilterFlags::MapEntity)},

	    {"ITERATOR_FILTER_BIT_HAS_TRANSFORM", math::to_integral(ecs::EntityIterator::FilterFlags::HasTransform)},
	    {"ITERATOR_FILTER_BIT_HAS_MODEL", math::to_integral(ecs::EntityIterator::FilterFlags::HasModel)},

	    {"ITERATOR_FILTER_ANY_TYPE", math::to_integral(ecs::EntityIterator::FilterFlags::AnyType)},
	    {"ITERATOR_FILTER_ANY", math::to_integral(ecs::EntityIterator::FilterFlags::Any)},
	    {"ITERATOR_FILTER_DEFAULT", math::to_integral(ecs::EntityIterator::FilterFlags::Default)},

	    {"TICK_POLICY_ALWAYS", math::to_integral(TickPolicy::Always)},
	    {"TICK_POLICY_NEVER", math::to_integral(TickPolicy::Never)},
	    {"TICK_POLICY_WHEN_VISIBLE", math::to_integral(TickPolicy::WhenVisible)},
	  });

	auto surfaceMatDef = pragma::LuaCore::register_class<physics::SurfaceMaterial>(GetLuaState(), "SurfaceMaterial");
	surfaceMatDef->def("GetName", &physics::SurfaceMaterial::GetIdentifier);
	surfaceMatDef->def("GetIndex", &physics::SurfaceMaterial::GetIndex);
	surfaceMatDef->def("SetFriction", &physics::SurfaceMaterial::SetFriction);
	surfaceMatDef->def("SetStaticFriction", &physics::SurfaceMaterial::SetStaticFriction);
	surfaceMatDef->def("SetDynamicFriction", &physics::SurfaceMaterial::SetDynamicFriction);
	surfaceMatDef->def("GetStaticFriction", &physics::SurfaceMaterial::GetStaticFriction);
	surfaceMatDef->def("GetDynamicFriction", &physics::SurfaceMaterial::GetDynamicFriction);
	surfaceMatDef->def("GetRestitution", &physics::SurfaceMaterial::GetRestitution);
	surfaceMatDef->def("SetRestitution", &physics::SurfaceMaterial::SetRestitution);
	surfaceMatDef->def("GetFootstepSound", &physics::SurfaceMaterial::GetFootstepType);
	surfaceMatDef->def("SetFootstepSound", &physics::SurfaceMaterial::SetFootstepType);
	surfaceMatDef->def("SetImpactParticleEffect", &physics::SurfaceMaterial::SetImpactParticleEffect);
	surfaceMatDef->def("GetImpactParticleEffect", &physics::SurfaceMaterial::GetImpactParticleEffect);
	surfaceMatDef->def("GetBulletImpactSound", &physics::SurfaceMaterial::GetBulletImpactSound);
	surfaceMatDef->def("SetBulletImpactSound", &physics::SurfaceMaterial::SetBulletImpactSound);
	surfaceMatDef->def("SetHardImpactSound", &physics::SurfaceMaterial::SetHardImpactSound);
	surfaceMatDef->def("GetHardImpactSound", &physics::SurfaceMaterial::GetHardImpactSound);
	surfaceMatDef->def("SetSoftImpactSound", &physics::SurfaceMaterial::SetSoftImpactSound);
	surfaceMatDef->def("GetSoftImpactSound", &physics::SurfaceMaterial::GetSoftImpactSound);
	surfaceMatDef->def("GetIOR", &physics::SurfaceMaterial::GetIOR);
	surfaceMatDef->def("SetIOR", &physics::SurfaceMaterial::SetIOR);
	surfaceMatDef->def("ClearIOR", &physics::SurfaceMaterial::ClearIOR);

	surfaceMatDef->def("SetAudioLowFrequencyAbsorption", &physics::SurfaceMaterial::SetAudioLowFrequencyAbsorption);
	surfaceMatDef->def("GetAudioLowFrequencyAbsorption", &physics::SurfaceMaterial::GetAudioLowFrequencyAbsorption);
	surfaceMatDef->def("SetAudioMidFrequencyAbsorption", &physics::SurfaceMaterial::SetAudioMidFrequencyAbsorption);
	surfaceMatDef->def("GetAudioMidFrequencyAbsorption", &physics::SurfaceMaterial::GetAudioMidFrequencyAbsorption);
	surfaceMatDef->def("SetAudioHighFrequencyAbsorption", &physics::SurfaceMaterial::SetAudioHighFrequencyAbsorption);
	surfaceMatDef->def("GetAudioHighFrequencyAbsorption", &physics::SurfaceMaterial::GetAudioHighFrequencyAbsorption);
	surfaceMatDef->def("SetAudioScattering", &physics::SurfaceMaterial::SetAudioScattering);
	surfaceMatDef->def("GetAudioScattering", &physics::SurfaceMaterial::GetAudioScattering);
	surfaceMatDef->def("SetAudioLowFrequencyTransmission", &physics::SurfaceMaterial::SetAudioLowFrequencyTransmission);
	surfaceMatDef->def("GetAudioLowFrequencyTransmission", &physics::SurfaceMaterial::GetAudioLowFrequencyTransmission);
	surfaceMatDef->def("SetAudioMidFrequencyTransmission", &physics::SurfaceMaterial::SetAudioMidFrequencyTransmission);
	surfaceMatDef->def("GetAudioMidFrequencyTransmission", &physics::SurfaceMaterial::GetAudioMidFrequencyTransmission);
	surfaceMatDef->def("SetAudioHighFrequencyTransmission", &physics::SurfaceMaterial::SetAudioHighFrequencyTransmission);
	surfaceMatDef->def("GetAudioHighFrequencyTransmission", &physics::SurfaceMaterial::GetAudioHighFrequencyTransmission);

	surfaceMatDef->def("GetNavigationFlags", &physics::SurfaceMaterial::GetNavigationFlags);
	surfaceMatDef->def("SetNavigationFlags", &physics::SurfaceMaterial::SetNavigationFlags);
	surfaceMatDef->def("SetDensity", &physics::SurfaceMaterial::SetDensity);
	surfaceMatDef->def("GetDensity", &physics::SurfaceMaterial::GetDensity);
	surfaceMatDef->def("SetLinearDragCoefficient", &physics::SurfaceMaterial::SetLinearDragCoefficient);
	surfaceMatDef->def("GetLinearDragCoefficient", &physics::SurfaceMaterial::GetLinearDragCoefficient);
	surfaceMatDef->def("SetTorqueDragCoefficient", &physics::SurfaceMaterial::SetTorqueDragCoefficient);
	surfaceMatDef->def("GetTorqueDragCoefficient", &physics::SurfaceMaterial::GetTorqueDragCoefficient);
	surfaceMatDef->def("SetWaveStiffness", &physics::SurfaceMaterial::SetWaveStiffness);
	surfaceMatDef->def("GetWaveStiffness", &physics::SurfaceMaterial::GetWaveStiffness);
	surfaceMatDef->def("SetWavePropagation", &physics::SurfaceMaterial::SetWavePropagation);
	surfaceMatDef->def("GetWavePropagation", &physics::SurfaceMaterial::GetWavePropagation);
	surfaceMatDef->def("GetPBRMetalness", static_cast<float (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().metalness; }));
	surfaceMatDef->def("GetPBRRoughness", static_cast<float (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().roughness; }));
	surfaceMatDef->def("GetSubsurfaceFactor", static_cast<float (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.factor; }));
	surfaceMatDef->def("SetSubsurfaceFactor", static_cast<void (*)(lua::State *, physics::SurfaceMaterial &, float)>([](lua::State *l, physics::SurfaceMaterial &surfMat, float factor) { surfMat.GetPBRInfo().subsurface.factor = factor; }));
	surfaceMatDef->def("GetSubsurfaceScatterColor", static_cast<Vector3 (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.scatterColor; }));
	surfaceMatDef->def("SetSubsurfaceScatterColor", static_cast<void (*)(lua::State *, physics::SurfaceMaterial &, const Vector3 &)>([](lua::State *l, physics::SurfaceMaterial &surfMat, const Vector3 &radiusRGB) { surfMat.GetPBRInfo().subsurface.scatterColor = radiusRGB; }));
	surfaceMatDef->def("GetSubsurfaceRadiusMM", static_cast<Vector3 (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.radiusMM; }));
	surfaceMatDef->def("SetSubsurfaceRadiusMM", static_cast<void (*)(lua::State *, physics::SurfaceMaterial &, const Vector3 &)>([](lua::State *l, physics::SurfaceMaterial &surfMat, const Vector3 &radiusMM) { surfMat.GetPBRInfo().subsurface.radiusMM = radiusMM; }));
	surfaceMatDef->def("GetSubsurfaceColor", static_cast<Color (*)(lua::State *, physics::SurfaceMaterial &)>([](lua::State *l, physics::SurfaceMaterial &surfMat) { return surfMat.GetPBRInfo().subsurface.color; }));
	surfaceMatDef->def("SetSubsurfaceColor", static_cast<void (*)(lua::State *, physics::SurfaceMaterial &, const Color &)>([](lua::State *l, physics::SurfaceMaterial &surfMat, const Color &color) { surfMat.GetPBRInfo().subsurface.color = color; }));
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
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_MODEL", math::to_integral(GibletCreateInfo::PhysShape::Model));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_NONE", math::to_integral(GibletCreateInfo::PhysShape::None));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_SPHERE", math::to_integral(GibletCreateInfo::PhysShape::Sphere));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_BOX", math::to_integral(GibletCreateInfo::PhysShape::Box));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_CYLINDER", math::to_integral(GibletCreateInfo::PhysShape::Cylinder));
	gameMod[gibletCreateInfo];

	Lua::register_bullet_info(gameMod);

	auto classDefDamageInfo = luabind::class_<game::DamageInfo>("DamageInfo");
	classDefDamageInfo.def(luabind::constructor<>());
	classDefDamageInfo.def(luabind::tostring(luabind::self));
	classDefDamageInfo.def("SetDamage", &game::DamageInfo::SetDamage);
	classDefDamageInfo.def("AddDamage", &game::DamageInfo::AddDamage);
	classDefDamageInfo.def("ScaleDamage", &game::DamageInfo::ScaleDamage);
	classDefDamageInfo.def("GetDamage", &game::DamageInfo::GetDamage);
	classDefDamageInfo.def("GetAttacker", &game::DamageInfo::GetAttacker);
	classDefDamageInfo.def("SetAttacker", static_cast<void (game::DamageInfo::*)(const ecs::BaseEntity *)>(&game::DamageInfo::SetAttacker));
	classDefDamageInfo.def("GetInflictor", &game::DamageInfo::GetInflictor);
	classDefDamageInfo.def("SetInflictor", static_cast<void (game::DamageInfo::*)(const ecs::BaseEntity *)>(&game::DamageInfo::SetInflictor));
	classDefDamageInfo.def("GetDamageTypes", &game::DamageInfo::GetDamageTypes);
	classDefDamageInfo.def("SetDamageType", &game::DamageInfo::SetDamageType);
	classDefDamageInfo.def("AddDamageType", &game::DamageInfo::AddDamageType);
	classDefDamageInfo.def("RemoveDamageType", &game::DamageInfo::RemoveDamageType);
	classDefDamageInfo.def("IsDamageType", &game::DamageInfo::IsDamageType);
	classDefDamageInfo.def("SetSource", &game::DamageInfo::SetSource);
	classDefDamageInfo.def("GetSource", &game::DamageInfo::GetSource, luabind::copy_policy<0> {});
	classDefDamageInfo.def("SetHitPosition", &game::DamageInfo::SetHitPosition);
	classDefDamageInfo.def("GetHitPosition", &game::DamageInfo::GetHitPosition, luabind::copy_policy<0> {});
	classDefDamageInfo.def("SetForce", &game::DamageInfo::SetForce);
	classDefDamageInfo.def("GetForce", &game::DamageInfo::GetForce, luabind::copy_policy<0> {});
	classDefDamageInfo.def("GetHitGroup", &game::DamageInfo::GetHitGroup);
	classDefDamageInfo.def("SetHitGroup", &game::DamageInfo::SetHitGroup);
	gameMod[classDefDamageInfo];
}

#define LUA_MATRIX_MEMBERS_CLASSDEF(defMat, type)                                                                                                                                                                                                                                                \
	defMat.def(luabind::constructor<>());                                                                                                                                                                                                                                                        \
	defMat.def(luabind::constructor<float>());                                                                                                                                                                                                                                                   \
	defMat.def(luabind::const_self / float());                                                                                                                                                                                                                                                   \
	defMat.def(luabind::const_self *float());                                                                                                                                                                                                                                                    \
	defMat.def(float() / luabind::const_self);                                                                                                                                                                                                                                                   \
	defMat.def(float() * luabind::const_self);                                                                                                                                                                                                                                                   \
	defMat.def("__tostring", &glm_type_to_string<Mat##type>);                                                                                                                                                                                                                                    \
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
	defMat.def("Set", static_cast<void (*)(lua::State *, Mat##type &, int, int, float)>(&Lua::Mat##type::Set));                                                                                                                                                                                  \
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
	defMat2.def("Set", static_cast<void (*)(lua::State *, Mat2 &, float, float, float, float)>(&Lua::Mat2::Set));
	defMat2.def("Set", static_cast<void (*)(lua::State *, Mat2 &, const Mat2 &)>(&Lua::Mat2::Set));
	modMath[defMat2];
	register_string_to_vector_type_constructor<Mat2>(lua.GetState());

	auto defMat2x3 = luabind::class_<Mat2x3>("Mat2x3");
	defMat2x3.def(luabind::constructor<float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x3, 2x3);
	defMat2x3.def(luabind::const_self == luabind::const_self);
	defMat2x3.def(luabind::const_self + luabind::const_self);
	defMat2x3.def(luabind::const_self - luabind::const_self);
	defMat2x3.def(luabind::const_self * Vector2());
	defMat2x3.def("Set", static_cast<void (*)(lua::State *, Mat2x3 &, float, float, float, float, float, float)>(&Lua::Mat2x3::Set));
	defMat2x3.def("Set", static_cast<void (*)(lua::State *, Mat2x3 &, const Mat2x3 &)>(&Lua::Mat2x3::Set));
	modMath[defMat2x3];
	register_string_to_vector_type_constructor<Mat2x3>(lua.GetState());

	auto defMat2x4 = luabind::class_<Mat2x4>("Mat2x4");
	defMat2x4.def(luabind::constructor<float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x4, 2x4);
	defMat2x4.def(luabind::const_self == luabind::const_self);
	defMat2x4.def(luabind::const_self + luabind::const_self);
	defMat2x4.def(luabind::const_self - luabind::const_self);
	defMat2x4.def(luabind::const_self * Vector2());
	defMat2x4.def("Set", static_cast<void (*)(lua::State *, Mat2x4 &, float, float, float, float, float, float, float, float)>(&Lua::Mat2x4::Set));
	defMat2x4.def("Set", static_cast<void (*)(lua::State *, Mat2x4 &, const Mat2x4 &)>(&Lua::Mat2x4::Set));
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
	defMat3.def("Set", static_cast<void (*)(lua::State *, Mat3 &, float, float, float, float, float, float, float, float, float)>(&Lua::Mat3::Set));
	defMat3.def("Set", static_cast<void (*)(lua::State *, Mat3 &, const Mat3 &)>(&Lua::Mat3::Set));
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
	defMat3x2.def("Set", static_cast<void (*)(lua::State *, Mat3x2 &, float, float, float, float, float, float)>(&Lua::Mat3x2::Set));
	defMat3x2.def("Set", static_cast<void (*)(lua::State *, Mat3x2 &, const Mat3x2 &)>(&Lua::Mat3x2::Set));
	modMath[defMat3x2];
	register_string_to_vector_type_constructor<Mat3x2>(lua.GetState());

	auto defMat3x4 = luabind::class_<Mat3x4>("Mat3x4");
	defMat3x4.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3x4, 3x4);
	defMat3x4.def(luabind::const_self == luabind::const_self);
	defMat3x4.def(luabind::const_self + luabind::const_self);
	defMat3x4.def(luabind::const_self - luabind::const_self);
	defMat3x4.def(luabind::const_self * Vector3());
	defMat3x4.def("Set", static_cast<void (*)(lua::State *, Mat3x4 &, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat3x4::Set));
	defMat3x4.def("Set", static_cast<void (*)(lua::State *, Mat3x4 &, const Mat3x4 &)>(&Lua::Mat3x4::Set));
	modMath[defMat3x4];
	register_string_to_vector_type_constructor<Mat3x4>(lua.GetState());

	auto defMat4 = luabind::class_<Mat4>("Mat4");
	defMat4.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4, 4);
	defMat4.def("Translate", &Lua::Mat4::Translate);
	defMat4.def("Rotate", static_cast<void (*)(lua::State *, Mat4 &, const EulerAngles &)>(&Lua::Mat4::Rotate));
	defMat4.def("Rotate", static_cast<void (*)(lua::State *, Mat4 &, const Vector3 &, float)>(&Lua::Mat4::Rotate));
	defMat4.def("Scale", &Lua::Mat4::Scale);
	defMat4.def("ToEulerAngles", &Lua::Mat4::ToEulerAngles);
	defMat4.def("ToQuaternion", &Lua::Mat4::ToQuaternion);
	defMat4.def("Decompose", &Lua::Mat4::Decompose);
	defMat4.def("MulRow", static_cast<void (*)(lua::State *, Mat4 &, uint32_t, float)>([](lua::State *l, Mat4 &m, uint32_t rowIndex, float factor) {
		for(uint8_t i = 0; i < 4; ++i)
			m[rowIndex][i] *= factor;
	}));
	defMat4.def("MulCol", static_cast<void (*)(lua::State *, Mat4 &, uint32_t, float)>([](lua::State *l, Mat4 &m, uint32_t colIndex, float factor) {
		for(uint8_t i = 0; i < 4; ++i)
			m[i][colIndex] *= factor;
	}));
	defMat4.def("SwapRows", static_cast<void (*)(lua::State *, Mat4 &, uint32_t, uint32_t)>([](lua::State *l, Mat4 &m, uint32_t rowSrc, uint32_t rowDst) {
		std::array<float, 4> tmpRow = {m[rowSrc][0], m[rowSrc][1], m[rowSrc][2], m[rowSrc][3]};
		for(uint8_t i = 0; i < 4; ++i) {
			m[rowSrc][i] = m[rowDst][i];
			m[rowDst][i] = tmpRow.at(i);
		}
	}));
	defMat4.def("SwapColumns", static_cast<void (*)(lua::State *, Mat4 &, uint32_t, uint32_t)>([](lua::State *l, Mat4 &m, uint32_t colSrc, uint32_t colDst) {
		std::array<float, 4> tmpCol = {m[0][colSrc], m[1][colSrc], m[2][colSrc], m[3][colSrc]};
		for(uint8_t i = 0; i < 4; ++i) {
			m[i][colSrc] = m[i][colDst];
			m[i][colDst] = tmpCol.at(i);
		}
	}));
	defMat4.def("ApplyProjectionDepthBiasOffset", static_cast<void (*)(lua::State *, Mat4 &, float, float, float, float)>([](lua::State *l, Mat4 &p, float nearZ, float farZ, float d, float delta) { umat::apply_projection_depth_bias_offset(p, nearZ, farZ, d, delta); }));
	defMat4.def(luabind::const_self == luabind::const_self);
	defMat4.def(luabind::const_self + luabind::const_self);
	defMat4.def(luabind::const_self - luabind::const_self);
	defMat4.def(luabind::const_self * luabind::const_self);
	defMat4.def(luabind::const_self * Vector4());
	defMat4.def("Inverse", &Lua::Mat4::Inverse);
	defMat4.def("GetInverse", &Lua::Mat4::GetInverse);
	defMat4.def("Set", static_cast<void (*)(lua::State *, Mat4 &, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat4::Set));
	defMat4.def("Set", static_cast<void (*)(lua::State *, Mat4 &, const Mat4 &)>(&Lua::Mat4::Set));
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
	defMat4x2.def("Set", static_cast<void (*)(lua::State *, Mat4x2 &, float, float, float, float, float, float, float, float)>(&Lua::Mat4x2::Set));
	defMat4x2.def("Set", static_cast<void (*)(lua::State *, Mat4x2 &, const Mat4x2 &)>(&Lua::Mat4x2::Set));
	modMath[defMat4x2];
	register_string_to_vector_type_constructor<Mat4x2>(lua.GetState());

	auto defMat4x3 = luabind::class_<Mat4x3>("Mat4x3");
	defMat4x3.def(luabind::constructor<float, float, float, float, float, float, float, float, float, float, float, float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4x3, 4x3);
	defMat4x3.def(luabind::const_self == luabind::const_self);
	defMat4x3.def(luabind::const_self + luabind::const_self);
	defMat4x3.def(luabind::const_self - luabind::const_self);
	defMat4x3.def(luabind::const_self * Vector4());
	defMat4x3.def("Set", static_cast<void (*)(lua::State *, Mat4x3 &, float, float, float, float, float, float, float, float, float, float, float, float)>(&Lua::Mat4x3::Set));
	defMat4x3.def("Set", static_cast<void (*)(lua::State *, Mat4x3 &, const Mat4x3 &)>(&Lua::Mat4x3::Set));
	modMath[defMat4x3];
	register_string_to_vector_type_constructor<Mat4x3>(lua.GetState());
}
