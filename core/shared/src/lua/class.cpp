/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
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
#include <pragma/util/matrices.h>
#include "pragma/physics/physobj.h"
#include "pragma/lua/classes/lphysobj.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/classes/ldamageinfo.h"
#include "pragma/game/damageinfo.h"
#include "pragma/lua/classes/lplane.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/model/model.h"
#include "luasystem.h"
#include "pragma/game/gamemode/gamemode.h"
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
#include "pragma/util/util_splash_damage_info.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/entities/entity_property.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/classes/lproperty_generic.hpp"
#include "pragma/lua/classes/lproperty_entity.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include <pragma/util/transform.h>
#include <sharedutils/datastream.h>
#include <sharedutils/util_parallel_job.hpp>
#include <sharedutils/util_path.hpp>
#include <util_image_buffer.hpp>
#include <mathutil/umath_lighting.hpp>
#include <luainterface.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/out_value_policy.hpp>

extern DLLENGINE Engine *engine;

std::ostream &operator<<(std::ostream &out,const ALSound &snd)
{
	auto state = snd.GetState();
	out<<"ALSound["<<snd.GetIndex()<<"][";
	switch(state)
	{
		case ALState::Initial:
			out<<"Initial";
			break;
		case ALState::Playing:
			out<<"Playing";
			break;
		case ALState::Paused:
			out<<"Paused";
			break;
		case ALState::Stopped:
			out<<"Stopped";
			break;
	}
	out<<"][";
	auto type = snd.GetType();
	auto values = umath::get_power_of_2_values(static_cast<uint64_t>(type));
	auto bStart = true;
	for(auto v : values)
	{
		if(bStart == false)
			out<<" | ";
		else
			bStart = true;
		if(v == static_cast<uint64_t>(ALSoundType::Effect))
			out<<"Effect";
		else if(v == static_cast<uint64_t>(ALSoundType::Music))
			out<<"Music";
		else if(v == static_cast<uint64_t>(ALSoundType::Voice))
			out<<"Voice";
		else if(v == static_cast<uint64_t>(ALSoundType::Weapon))
			out<<"Weapon";
		else if(v == static_cast<uint64_t>(ALSoundType::NPC))
			out<<"NPC";
		else if(v == static_cast<uint64_t>(ALSoundType::Player))
			out<<"Player";
		else if(v == static_cast<uint64_t>(ALSoundType::Vehicle))
			out<<"Vehicle";
		else if(v == static_cast<uint64_t>(ALSoundType::Physics))
			out<<"Physics";
		else if(v == static_cast<uint64_t>(ALSoundType::Environment))
			out<<"Environment";
	}
	out<<"]";
	return out;
}

static void RegisterLuaMatrices(Lua::Interface &lua);

static Quat QuaternionConstruct() {return uquat::identity();}
static Quat QuaternionConstruct(float w,float x,float y,float z) {return Quat(w,x,y,z);}
static Quat QuaternionConstruct(const Vector3 &v,float f) {return uquat::create(v,f);}
static Quat QuaternionConstruct(const Vector3 &a,const Vector3 &b,const Vector3 &c)
{
	auto m = umat::create_from_axes(a,b,c);
	return Quat(m);
}
static Quat QuaternionConstruct(const Quat &q) {return Quat(q);}
static Quat QuaternionConstruct(const Vector3 &forward,const Vector3 &up) {return uquat::create_look_rotation(forward,up);}

void NetworkState::RegisterSharedLuaClasses(Lua::Interface &lua)
{
	auto modString = luabind::module_(lua.GetState(),"string");
	modString[
		luabind::def("calc_levenshtein_distance",Lua::string::calc_levenshtein_distance),
		luabind::def("calc_levenshtein_similarity",Lua::string::calc_levenshtein_similarity),
		luabind::def("find_longest_common_substring",Lua::string::find_longest_common_substring,luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>,luabind::pure_out_value<5>>::type{}),
		luabind::def("find_similar_elements",Lua::string::find_similar_elements,luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("is_integer",ustring::is_integer),
		luabind::def("is_number",ustring::is_number),
		luabind::def("split",Lua::string::split,luabind::return_stl_iterator{}),
		luabind::def("join",static_cast<std::string(*)(lua_State*,luabind::table<>,const std::string&)>(Lua::string::join)),
		luabind::def("join",static_cast<std::string(*)(lua_State*,luabind::table<>)>([](lua_State *l,luabind::table<> values) {return Lua::string::join(l,values);})),
		luabind::def("remove_whitespace",Lua::string::remove_whitespace),
		luabind::def("remove_quotes",Lua::string::remove_quotes),
		luabind::def("replace",static_cast<std::string(*)(const std::string&,const std::string&,const std::string&)>([](const std::string &subject,const std::string &from,const std::string &to) -> std::string {
			auto tmp = subject;
			ustring::replace(tmp,from,to);
			return tmp;
		})),
		luabind::def("fill_zeroes",ustring::fill_zeroes),
		luabind::def("compare",static_cast<bool(*)(const char*,const char*,bool,size_t)>(ustring::compare)),
		luabind::def("compare",static_cast<bool(*)(const std::string&,const std::string&,bool)>(ustring::compare)),
		luabind::def("compare",static_cast<bool(*)(const std::string&,const std::string&)>([](const std::string &a,const std::string &b) -> bool {return ustring::compare(a,b,true);}))
	];

	auto modLight = luabind::module_(lua.GetState(),"light");
	modLight[
		luabind::def("get_color_temperature",static_cast<void(*)(ulighting::NaturalLightType,Kelvin&,Kelvin&)>([](ulighting::NaturalLightType type,Kelvin &outMin,Kelvin &outMax) {
			auto colTemp = ulighting::get_color_temperature(type);
			outMin = colTemp.first;
			outMax = colTemp.second;
		}),luabind::meta::join<luabind::pure_out_value<2>,luabind::pure_out_value<3>>::type{}),
		luabind::def("get_average_color_temperature",ulighting::get_average_color_temperature),
		luabind::def("color_temperature_to_color",ulighting::color_temperature_to_color),
		luabind::def("wavelength_to_color",ulighting::wavelength_to_color),
		luabind::def("get_luminous_efficacy",ulighting::get_luminous_efficacy),
		luabind::def("lumens_to_watts",static_cast<Watt(*)(Lumen,LuminousEfficacy)>(ulighting::lumens_to_watts)),
		luabind::def("lumens_to_watts",static_cast<Watt(*)(Lumen)>([](Lumen lumen) -> Watt {return ulighting::lumens_to_watts(lumen);})),
		luabind::def("watts_to_lumens",static_cast<Watt(*)(Lumen,LuminousEfficacy)>(ulighting::watts_to_lumens)),
		luabind::def("watts_to_lumens",static_cast<Watt(*)(Lumen)>([](Watt watt) -> Lumen {return ulighting::watts_to_lumens(watt);})),
		luabind::def("irradiance_to_lux",ulighting::irradiance_to_lux),
		luabind::def("lux_to_irradiance",ulighting::lux_to_irradiance)
	];

	Lua::RegisterLibraryEnums(lua.GetState(),"light",{
		{"NATURAL_LIGHT_TYPE_MATCH_FLAME",umath::to_integral(ulighting::NaturalLightType::MatchFlame)},
		{"NATURAL_LIGHT_TYPE_CANDLE",umath::to_integral(ulighting::NaturalLightType::Candle)},
		{"NATURAL_LIGHT_TYPE_FLAME",umath::to_integral(ulighting::NaturalLightType::Flame)},
		{"NATURAL_LIGHT_TYPE_SUNSET",umath::to_integral(ulighting::NaturalLightType::Sunset)},
		{"NATURAL_LIGHT_TYPE_SUNRISE",umath::to_integral(ulighting::NaturalLightType::Sunrise)},
		{"NATURAL_LIGHT_TYPE_HOUSEHOLD_TUNGSTEN_BULB",umath::to_integral(ulighting::NaturalLightType::HouseholdTungstenBulb)},
		{"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_500W_TO_1K",umath::to_integral(ulighting::NaturalLightType::TungstenLamp500WTo1K)},
		{"NATURAL_LIGHT_TYPE_INCANDESCENT_LAMP",umath::to_integral(ulighting::NaturalLightType::IncandescentLamp)},
		{"NATURAL_LIGHT_TYPE_WARM_FLUORESCENT_LAMP",umath::to_integral(ulighting::NaturalLightType::WarmFluorescentLamp)},
		{"NATURAL_LIGHT_TYPE_LED_LAMP",umath::to_integral(ulighting::NaturalLightType::LEDLamp)},
		{"NATURAL_LIGHT_TYPE_QUARTZ_LIGHT",umath::to_integral(ulighting::NaturalLightType::QuartzLight)},
		{"NATURAL_LIGHT_TYPE_STUDIO_LAMP",umath::to_integral(ulighting::NaturalLightType::StudioLamp)},
		{"NATURAL_LIGHT_TYPE_FLOODLIGHT",umath::to_integral(ulighting::NaturalLightType::Floodlight)},
		{"NATURAL_LIGHT_TYPE_FLUORESCENT_LIGHT",umath::to_integral(ulighting::NaturalLightType::FluorescentLight)},
		{"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_2K",umath::to_integral(ulighting::NaturalLightType::TungstenLamp2K)},
		{"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_5K",umath::to_integral(ulighting::NaturalLightType::TungstenLamp5K)},
		{"NATURAL_LIGHT_TYPE_TUNGSTEN_LAMP_10K",umath::to_integral(ulighting::NaturalLightType::TungstenLamp10K)},
		{"NATURAL_LIGHT_TYPE_MOONLIGHT",umath::to_integral(ulighting::NaturalLightType::Moonlight)},
		{"NATURAL_LIGHT_TYPE_HORIZON_DAYLIGHT",umath::to_integral(ulighting::NaturalLightType::HorizonDaylight)},
		{"NATURAL_LIGHT_TYPE_TUBULAR_FLUORESCENT_LAMP",umath::to_integral(ulighting::NaturalLightType::TubularFluorescentLamp)},
		{"NATURAL_LIGHT_TYPE_VERTICAL_DAYLIGHT",umath::to_integral(ulighting::NaturalLightType::VerticalDaylight)},
		{"NATURAL_LIGHT_TYPE_SUN_AT_NOON",umath::to_integral(ulighting::NaturalLightType::SunAtNoon)},
		{"NATURAL_LIGHT_TYPE_DAYLIGHT",umath::to_integral(ulighting::NaturalLightType::Daylight)},
		{"NATURAL_LIGHT_TYPE_SUN_THROUGH_CLOUDS",umath::to_integral(ulighting::NaturalLightType::SunThroughClouds)},
		{"NATURAL_LIGHT_TYPE_OVERCAST",umath::to_integral(ulighting::NaturalLightType::Overcast)},
		{"NATURAL_LIGHT_TYPE_RGB_MONITOR_WHITE_POINT",umath::to_integral(ulighting::NaturalLightType::RGBMonitorWhitePoint)},
		{"NATURAL_LIGHT_TYPE_OUTDOOR_SHADE",umath::to_integral(ulighting::NaturalLightType::OutdoorShade)},
		{"NATURAL_LIGHT_TYPE_PARTLY_CLOUDY",umath::to_integral(ulighting::NaturalLightType::PartlyCloudy)},
		{"NATURAL_LIGHT_TYPE_CLEAR_BLUESKY",umath::to_integral(ulighting::NaturalLightType::ClearBlueSky)},
		{"NATURAL_LIGHT_TYPE_CLEAR_COUNT",umath::to_integral(ulighting::NaturalLightType::Count)},

		{"LIGHT_SOURCE_TYPE_TUNGSTEN_INCANDESCENT_LIGHT_BULB",umath::to_integral(ulighting::LightSourceType::TungstenIncandescentLightBulb)},
		{"LIGHT_SOURCE_TYPE_HALOGEN_LAMP",umath::to_integral(ulighting::LightSourceType::HalogenLamp)},
		{"LIGHT_SOURCE_TYPE_FLUORESCENT_LAMP",umath::to_integral(ulighting::LightSourceType::FluorescentLamp)},
		{"LIGHT_SOURCE_TYPE_LED_LAMP",umath::to_integral(ulighting::LightSourceType::LEDLamp)},
		{"LIGHT_SOURCE_TYPE_METAL_HALIDE_LAMP",umath::to_integral(ulighting::LightSourceType::MetalHalideLamp)},
		{"LIGHT_SOURCE_TYPE_HIGH_PRESSURE_SODIUM_VAPOR_LAMP",umath::to_integral(ulighting::LightSourceType::HighPressureSodiumVaporLamp)},
		{"LIGHT_SOURCE_TYPE_LOW_PRESSURE_SODIUM_VAPOR_LAMP",umath::to_integral(ulighting::LightSourceType::LowPressureSodiumVaporLamp)},
		{"LIGHT_SOURCE_TYPE_MERCURY_VAPOR_LAMP",umath::to_integral(ulighting::LightSourceType::MercuryVaporLamp)},
		{"LIGHT_SOURCE_TYPE_D65_STANDARD_ILLUMINANT",umath::to_integral(ulighting::LightSourceType::D65StandardIlluminant)}
	});

	auto &modUtil = lua.RegisterLibrary("util");

	auto defParallelJob = luabind::class_<util::BaseParallelJob>("ParallelJob");
	defParallelJob.add_static_constant("JOB_STATUS_FAILED",umath::to_integral(util::JobStatus::Failed));
	defParallelJob.add_static_constant("JOB_STATUS_SUCCESSFUL",umath::to_integral(util::JobStatus::Successful));
	defParallelJob.add_static_constant("JOB_STATUS_INITIAL",umath::to_integral(util::JobStatus::Initial));
	defParallelJob.add_static_constant("JOB_STATUS_CANCELLED",umath::to_integral(util::JobStatus::Cancelled));
	defParallelJob.add_static_constant("JOB_STATUS_PENDING",umath::to_integral(util::JobStatus::Pending));
	defParallelJob.add_static_constant("JOB_STATUS_INVALID",umath::to_integral(util::JobStatus::Invalid));
	defParallelJob.def("Cancel",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		job.Cancel();
	}));
	defParallelJob.def("Wait",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		job.Wait();
	}));
	defParallelJob.def("Start",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		job.Start();
	}));
	defParallelJob.def("IsComplete",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsComplete());
	}));
	defParallelJob.def("IsPending",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsPending());
	}));
	defParallelJob.def("IsCancelled",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsCancelled());
	}));
	defParallelJob.def("IsSuccessful",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsSuccessful());
	}));
	defParallelJob.def("IsThreadActive",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsThreadActive());
	}));
	defParallelJob.def("GetProgress",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushNumber(l,job.GetProgress());
	}));
	defParallelJob.def("GetStatus",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushInt(l,umath::to_integral(job.GetStatus()));
	}));
	defParallelJob.def("GetResultMessage",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushString(l,job.GetResultMessage());
	}));
	defParallelJob.def("IsValid",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.IsValid());
	}));
	defParallelJob.def("Poll",static_cast<void(*)(lua_State*,util::BaseParallelJob&)>([](lua_State *l,util::BaseParallelJob &job) {
		Lua::PushBool(l,job.Poll());
	}));
	modUtil[defParallelJob];

	auto defImageBuffer = luabind::class_<uimg::ImageBuffer>("ImageBuffer");
	defImageBuffer.add_static_constant("FORMAT_NONE",umath::to_integral(uimg::ImageBuffer::Format::None));
	defImageBuffer.add_static_constant("FORMAT_RGB8",umath::to_integral(uimg::ImageBuffer::Format::RGB8));
	defImageBuffer.add_static_constant("FORMAT_RGBA8",umath::to_integral(uimg::ImageBuffer::Format::RGBA8));
	defImageBuffer.add_static_constant("FORMAT_RGB16",umath::to_integral(uimg::ImageBuffer::Format::RGB16));
	defImageBuffer.add_static_constant("FORMAT_RGBA16",umath::to_integral(uimg::ImageBuffer::Format::RGBA16));
	defImageBuffer.add_static_constant("FORMAT_RGB32",umath::to_integral(uimg::ImageBuffer::Format::RGB32));
	defImageBuffer.add_static_constant("FORMAT_RGBA32",umath::to_integral(uimg::ImageBuffer::Format::RGBA32));
	defImageBuffer.add_static_constant("FORMAT_COUNT",umath::to_integral(uimg::ImageBuffer::Format::Count));

	defImageBuffer.add_static_constant("FORMAT_RGB_LDR",umath::to_integral(uimg::ImageBuffer::Format::RGB_LDR));
	defImageBuffer.add_static_constant("FORMAT_RGBA_LDR",umath::to_integral(uimg::ImageBuffer::Format::RGBA_LDR));
	defImageBuffer.add_static_constant("FORMAT_RGB_HDR",umath::to_integral(uimg::ImageBuffer::Format::RGB_HDR));
	defImageBuffer.add_static_constant("FORMAT_RGBA_HDR",umath::to_integral(uimg::ImageBuffer::Format::RGBA_HDR));
	defImageBuffer.add_static_constant("FORMAT_RGB_FLOAT",umath::to_integral(uimg::ImageBuffer::Format::RGB_FLOAT));
	defImageBuffer.add_static_constant("FORMAT_RGBA_FLOAT",umath::to_integral(uimg::ImageBuffer::Format::RGBA_FLOAT));

	defImageBuffer.add_static_constant("CHANNEL_RED",umath::to_integral(uimg::ImageBuffer::Channel::Red));
	defImageBuffer.add_static_constant("CHANNEL_GREEN",umath::to_integral(uimg::ImageBuffer::Channel::Green));
	defImageBuffer.add_static_constant("CHANNEL_BLUE",umath::to_integral(uimg::ImageBuffer::Channel::Blue));
	defImageBuffer.add_static_constant("CHANNEL_ALPHA",umath::to_integral(uimg::ImageBuffer::Channel::Alpha));
	defImageBuffer.add_static_constant("CHANNEL_R",umath::to_integral(uimg::ImageBuffer::Channel::R));
	defImageBuffer.add_static_constant("CHANNEL_G",umath::to_integral(uimg::ImageBuffer::Channel::G));
	defImageBuffer.add_static_constant("CHANNEL_B",umath::to_integral(uimg::ImageBuffer::Channel::B));
	defImageBuffer.add_static_constant("CHANNEL_A",umath::to_integral(uimg::ImageBuffer::Channel::A));

	defImageBuffer.add_static_constant("TONE_MAPPING_GAMMA_CORRECTION",umath::to_integral(uimg::ImageBuffer::ToneMapping::GammaCorrection));
	defImageBuffer.add_static_constant("TONE_MAPPING_REINHARD",umath::to_integral(uimg::ImageBuffer::ToneMapping::Reinhard));
	defImageBuffer.add_static_constant("TONE_MAPPING_HEJIL_RICHARD",umath::to_integral(uimg::ImageBuffer::ToneMapping::HejilRichard));
	defImageBuffer.add_static_constant("TONE_MAPPING_UNCHARTED",umath::to_integral(uimg::ImageBuffer::ToneMapping::Uncharted));
	defImageBuffer.add_static_constant("TONE_MAPPING_ACES",umath::to_integral(uimg::ImageBuffer::ToneMapping::Aces));
	defImageBuffer.add_static_constant("TONE_MAPPING_GRAN_TURISMO",umath::to_integral(uimg::ImageBuffer::ToneMapping::GranTurismo));

	defImageBuffer.scope[luabind::def("Create",static_cast<void(*)(lua_State*,uint32_t,uint32_t,uint32_t,DataStream&)>([](lua_State *l,uint32_t width,uint32_t height,uint32_t format,DataStream &ds) {
		auto imgBuffer = uimg::ImageBuffer::Create(ds->GetData(),width,height,static_cast<uimg::ImageBuffer::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l,imgBuffer);
	}))];
	defImageBuffer.scope[luabind::def("Create",static_cast<void(*)(lua_State*,uint32_t,uint32_t,uint32_t)>([](lua_State *l,uint32_t width,uint32_t height,uint32_t format) {
		auto imgBuffer = uimg::ImageBuffer::Create(width,height,static_cast<uimg::ImageBuffer::Format>(format));
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l,imgBuffer);
	}))];
	defImageBuffer.scope[luabind::def("Create",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &parent,uint32_t x,uint32_t y,uint32_t w,uint32_t h) {
		auto imgBuffer = uimg::ImageBuffer::Create(parent,x,y,w,h);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l,imgBuffer);
	}))];
	defImageBuffer.scope[luabind::def("CreateCubemap",static_cast<void(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l,t);
		std::array<std::shared_ptr<uimg::ImageBuffer>,6> cubemapSides {};
		for(uint8_t i=0;i<6;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			auto &img = Lua::Check<uimg::ImageBuffer>(l,-1);
			cubemapSides.at(i) = img.shared_from_this();
			Lua::Pop(l,1);
		}
		auto imgBuffer = uimg::ImageBuffer::CreateCubemap(cubemapSides);
		if(imgBuffer == nullptr)
			return;
		Lua::Push(l,imgBuffer);
	}))];
	defImageBuffer.def("GetData",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		auto *data = imgBuffer.GetData();
		auto dataSize = imgBuffer.GetSize();
		DataStream ds {data,static_cast<uint32_t>(dataSize)};
		ds->SetOffset(0);
		Lua::Push(l,ds);
	}));
	defImageBuffer.def("GetFormat",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,umath::to_integral(imgBuffer.GetFormat()));
	}));
	defImageBuffer.def("GetWidth",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetWidth());
	}));
	defImageBuffer.def("GetHeight",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetHeight());
	}));
	defImageBuffer.def("GetChannelCount",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetChannelCount());
	}));
	defImageBuffer.def("GetChannelSize",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetChannelSize());
	}));
	defImageBuffer.def("GetPixelSize",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetPixelSize());
	}));
	defImageBuffer.def("GetPixelCount",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetPixelCount());
	}));
	defImageBuffer.def("HasAlphaChannel",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushBool(l,imgBuffer.HasAlphaChannel());
	}));
	defImageBuffer.def("IsLDRFormat",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushBool(l,imgBuffer.IsLDRFormat());
	}));
	defImageBuffer.def("IsHDRFormat",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushBool(l,imgBuffer.IsHDRFormat());
	}));
	defImageBuffer.def("IsFloatFormat",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushBool(l,imgBuffer.IsFloatFormat());
	}));
	defImageBuffer.def("Copy",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::Push(l,imgBuffer.Copy());
	}));
	defImageBuffer.def("Copy",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t format) {
		Lua::Push(l,imgBuffer.Copy(static_cast<uimg::ImageBuffer::Format>(format)));
	}));
	defImageBuffer.def("Copy",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uimg::ImageBuffer &dst,uint32_t xSrc,uint32_t ySrc,uint32_t xDst,uint32_t yDst,uint32_t w,uint32_t h) {
		imgBuffer.Copy(dst,xSrc,ySrc,xDst,yDst,w,h);
	}));
	defImageBuffer.def("Convert",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t format) {
		imgBuffer.Convert(static_cast<uimg::ImageBuffer::Format>(format));
	}));
	defImageBuffer.def("ToLDR",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		imgBuffer.ToLDR();
	}));
	defImageBuffer.def("ToHDR",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		imgBuffer.ToHDR();
	}));
	defImageBuffer.def("ToFloat",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		imgBuffer.ToFloat();
	}));
	defImageBuffer.def("GetSize",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		Lua::PushInt(l,imgBuffer.GetSize());
	}));
	defImageBuffer.def("Clear",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,const Color&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,const Color &color) {
		imgBuffer.Clear(color);
	}));
	defImageBuffer.def("Clear",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,const Vector4&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,const Vector4 &color) {
		imgBuffer.Clear(color);
	}));
	defImageBuffer.def("ClearAlpha",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,float)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,float a) {
		a = umath::clamp(a,0.f,1.f);
		imgBuffer.ClearAlpha(a *std::numeric_limits<uint8_t>::max());
	}));
	defImageBuffer.def("GetPixelIndex",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y) {
		Lua::PushInt(l,imgBuffer.GetPixelIndex(x,y));
	}));
	defImageBuffer.def("GetPixelOffset",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y) {
		Lua::PushInt(l,imgBuffer.GetPixelOffset(x,y));
	}));
	defImageBuffer.def("Resize",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t w,uint32_t h) {
		imgBuffer.Resize(w,h);
	}));
	defImageBuffer.def("FlipHorizontally",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		imgBuffer.FlipHorizontally();
	}));
	defImageBuffer.def("FlipVertically",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		imgBuffer.FlipVertically();
	}));
	defImageBuffer.def("ApplyToneMapping",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t toneMapping) {
		auto tonemappedImg = imgBuffer.ApplyToneMapping(static_cast<uimg::ImageBuffer::ToneMapping>(toneMapping));
		if(tonemappedImg == nullptr)
			return;
		Lua::Push(l,tonemappedImg);
	}));

	defImageBuffer.def("GetPixelOffset",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y) {
		Lua::PushInt(l,imgBuffer.GetPixelOffset(x,y));
	}));
	defImageBuffer.def("GetPixelIndex",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y) {
		Lua::PushInt(l,imgBuffer.GetPixelIndex(x,y));
	}));
	defImageBuffer.def("GetPixelValue",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y,uint32_t channel) {
		Lua::PushNumber(l,imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x,y)).GetFloatValue(static_cast<uimg::ImageBuffer::Channel>(channel)));
	}));
	defImageBuffer.def("SetPixelValue",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t,float)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y,uint32_t channel,float value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x,y)).SetValue(static_cast<uimg::ImageBuffer::Channel>(channel),value);
	}));
	defImageBuffer.def("SetPixelValueLDR",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t,uint8_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y,uint32_t channel,uint8_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x,y)).SetValue(static_cast<uimg::ImageBuffer::Channel>(channel),value);
	}));
	defImageBuffer.def("SetPixelValueHDR",static_cast<void(*)(lua_State*,uimg::ImageBuffer&,uint32_t,uint32_t,uint32_t,uint16_t)>([](lua_State *l,uimg::ImageBuffer &imgBuffer,uint32_t x,uint32_t y,uint32_t channel,uint16_t value) {
		imgBuffer.GetPixelView(imgBuffer.GetPixelOffset(x,y)).SetValue(static_cast<uimg::ImageBuffer::Channel>(channel),value);
	}));
	defImageBuffer.def("CalcLuminance",static_cast<void(*)(lua_State*,uimg::ImageBuffer&)>([](lua_State *l,uimg::ImageBuffer &imgBuffer) {
		float avgLuminance,minLuminance,maxLuminance,logAvgLuminance;
		Vector3 avgIntensity;
		imgBuffer.CalcLuminance(avgLuminance,minLuminance,maxLuminance,avgIntensity,&logAvgLuminance);
		Lua::PushNumber(l,avgLuminance);
		Lua::PushNumber(l,minLuminance);
		Lua::PushNumber(l,maxLuminance);
		Lua::Push<Vector3>(l,avgIntensity);
		Lua::PushNumber(l,logAvgLuminance);
	}));
	modUtil[defImageBuffer];

	auto defImgParallelJob = luabind::class_<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>,util::BaseParallelJob>("ParallelJobImage");
	defImgParallelJob.def("GetResult",static_cast<void(*)(lua_State*,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>&)>([](lua_State *l,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &job) {
		Lua::Push(l,job.GetResult());
	}));
	modUtil[defImgParallelJob];

	auto defDataBlock = luabind::class_<ds::Block>("DataBlock");
	defDataBlock.scope[luabind::def("load",static_cast<void(*)(lua_State*,const std::string&)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("load",static_cast<void(*)(lua_State*,VFilePtr)>(Lua::DataBlock::load))];
	defDataBlock.scope[luabind::def("create",static_cast<void(*)(lua_State*)>(Lua::DataBlock::create))];

	defDataBlock.def("GetInt",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector4",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::GetVector4));

	defDataBlock.def("GetInt",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,int32_t)>(&Lua::DataBlock::GetInt));
	defDataBlock.def("GetFloat",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,float)>(&Lua::DataBlock::GetFloat));
	defDataBlock.def("GetBool",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,bool)>(&Lua::DataBlock::GetBool));
	defDataBlock.def("GetString",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,const std::string&)>(&Lua::DataBlock::GetString));
	defDataBlock.def("GetColor",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,const Color&)>(&Lua::DataBlock::GetColor));
	defDataBlock.def("GetVector",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,const Vector3&)>(&Lua::DataBlock::GetVector));
	defDataBlock.def("GetVector4",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,const Vector4&)>(&Lua::DataBlock::GetVector4));

	defDataBlock.def("GetData",&Lua::DataBlock::GetData);
	defDataBlock.def("GetChildBlocks",&Lua::DataBlock::GetChildBlocks);
	defDataBlock.def("SetValue",&Lua::DataBlock::SetValue);
	defDataBlock.def("Merge",&Lua::DataBlock::Merge);
	defDataBlock.def("GetValueType",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>([](lua_State *l,ds::Block &dataBlock,const std::string &key) {
		auto val = dataBlock.GetDataValue(key);
		if(val == nullptr)
			return;
		Lua::PushString(l,val->GetTypeString());
	}));
	defDataBlock.def("GetKeys",static_cast<void(*)(lua_State*,ds::Block&)>([](lua_State *l,ds::Block &dataBlock) {
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &pair : *dataBlock.GetData())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}));

	defDataBlock.def("RemoveValue",&Lua::DataBlock::RemoveValue);
	defDataBlock.def("IsEmpty",&Lua::DataBlock::IsEmpty);
	defDataBlock.def("HasValue",&Lua::DataBlock::HasValue);
	defDataBlock.def("AddBlock",&Lua::DataBlock::AddBlock);
	defDataBlock.def("IsString",&Lua::DataBlock::IsString);
	defDataBlock.def("IsInt",&Lua::DataBlock::IsInt);
	defDataBlock.def("IsFloat",&Lua::DataBlock::IsFloat);
	defDataBlock.def("IsBool",&Lua::DataBlock::IsBool);
	defDataBlock.def("IsColor",&Lua::DataBlock::IsColor);
	defDataBlock.def("IsVector",&Lua::DataBlock::IsVector);
	defDataBlock.def("IsVector4",&Lua::DataBlock::IsVector4);
	defDataBlock.def("FindBlock",static_cast<void(*)(lua_State*,ds::Block&,const std::string&)>(&Lua::DataBlock::FindBlock));
	defDataBlock.def("FindBlock",static_cast<void(*)(lua_State*,ds::Block&,const std::string&,uint32_t)>(&Lua::DataBlock::FindBlock));
	modUtil[defDataBlock];

	// Path
	auto defPath = luabind::class_<util::Path>("Path");
	defPath.scope[luabind::def("CreateFromComponents",static_cast<void(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l,t);
		std::vector<std::string> components {};
		auto n = Lua::GetObjectLength(l,t);
		components.reserve(n);
		for(auto i=decltype(n){0u};i<n;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			components.push_back(Lua::CheckString(l,-1));

			Lua::Pop(l,1);
		}
		Lua::Push<util::Path>(l,{components});
	}))];
	defPath.def(luabind::constructor<>());
	defPath.def(luabind::constructor<const util::Path&>());
	defPath.def(luabind::constructor<const std::string&>());

	defPath.def(luabind::tostring(luabind::self));
	defPath.def(luabind::self +luabind::const_self);
	defPath.def(luabind::self +std::string{});

	defPath.def(luabind::const_self ==luabind::const_self);
	defPath.def(luabind::const_self ==std::string{});

	defPath.def("Copy",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::Push<util::Path>(l,p);
	}));
	defPath.def("ToComponents",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		auto components = p.ToComponents();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &c : components)
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,c);
			Lua::SetTableValue(l,t);
		}
	}));
	defPath.def("GetString",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushString(l,p.GetString());
	}));
	defPath.def("GetPath",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushString(l,p.GetPath());
	}));
	defPath.def("GetFileName",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushString(l,p.GetFileName());
	}));
	defPath.def("GetFront",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushString(l,p.GetFront());
	}));
	defPath.def("GetBack",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushString(l,p.GetBack());
	}));
	defPath.def("MoveUp",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		p.MoveUp();
	}));
	defPath.def("PopFront",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		p.PopFront();
	}));
	defPath.def("PopBack",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		p.PopBack();
	}));
	defPath.def("Canonicalize",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		p.Canonicalize();
	}));
	defPath.def("IsFile",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushBool(l,p.IsFile());
	}));
	defPath.def("IsPath",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushBool(l,!p.IsFile());
	}));
	defPath.def("GetFileExtension",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		auto ext = p.GetFileExtension();
		if(ext.has_value() == false)
			return;
		Lua::PushString(l,*ext);
	}));
	defPath.def("RemoveFileExtension",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		p.RemoveFileExtension();
	}));
	defPath.def("MakeRelative",static_cast<void(*)(lua_State*,util::Path&,util::Path&)>([](lua_State *l,util::Path &p,util::Path &pOther) {
		p.MakeRelative(pOther);
	}));
	defPath.def("MakeRelative",static_cast<void(*)(lua_State*,util::Path&,const std::string&)>([](lua_State *l,util::Path &p,const std::string &other) {
		p.MakeRelative(other);
	}));
	defPath.def("GetComponentCount",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushInt(l,p.GetComponentCount());
	}));
	defPath.def("IsEmpty",static_cast<void(*)(lua_State*,util::Path&)>([](lua_State *l,util::Path &p) {
		Lua::PushBool(l,p.IsEmpty());
	}));
	modUtil[defPath];

	// Properties
	Lua::Property::register_classes(lua);

	auto &modMath = lua.RegisterLibrary("math");

	// Transform
	auto classDefTransform = luabind::class_<Transform>("Transform");
	classDefTransform.def(luabind::constructor<>());
	classDefTransform.def(luabind::constructor<const Vector3&>());
	classDefTransform.def(luabind::constructor<const Quat&>());
	classDefTransform.def(luabind::constructor<const Vector3&,const Quat&>());
	classDefTransform.def("GetScale",static_cast<void(*)(lua_State*,Transform&)>([](lua_State *l,Transform &t) {
		auto scale = t.GetScale();
		Lua::Push<Vector3>(l,scale);
	}));
	classDefTransform.def("GetPosition",static_cast<void(*)(lua_State*,Transform&)>([](lua_State *l,Transform &t) {
		auto pos = t.GetPosition();
		Lua::Push<Vector3>(l,pos);
	}));
	classDefTransform.def("GetRotation",static_cast<void(*)(lua_State*,Transform&)>([](lua_State *l,Transform &t) {
		auto rot = t.GetOrientation();
		Lua::Push<Quat>(l,rot);
	}));
	classDefTransform.def("GetTransformationMatrix",static_cast<void(*)(lua_State*,Transform&)>([](lua_State *l,Transform &t) {
		auto m = t.GetTransformationMatrix();
		Lua::Push<Mat4>(l,m);
	}));
	classDefTransform.def("SetScale",static_cast<void(*)(lua_State*,Transform&,const Vector3&)>([](lua_State *l,Transform &t,const Vector3 &scale) {
		t.SetScale(scale);
	}));
	classDefTransform.def("SetPosition",static_cast<void(*)(lua_State*,Transform&,const Vector3&)>([](lua_State *l,Transform &t,const Vector3 &pos) {
		t.SetPosition(pos);
	}));
	classDefTransform.def("SetRotation",static_cast<void(*)(lua_State*,Transform&,const Quat&)>([](lua_State *l,Transform &t,const Quat &rot) {
		t.SetOrientation(rot);
	}));
	classDefTransform.def("UpdateMatrix",static_cast<void(*)(lua_State*,Transform&)>([](lua_State *l,Transform &t) {
		t.UpdateMatrix();
	}));
	modMath[classDefTransform];

	// PID Controller
	auto defPIDController = luabind::class_<util::PIDController>("PIDController");
	defPIDController.def(luabind::constructor<>());
	defPIDController.def(luabind::constructor<float,float,float>());
	defPIDController.def(luabind::constructor<float,float,float,float,float>());
	defPIDController.def("SetProportionalTerm",&util::PIDController::SetProportionalTerm);
	defPIDController.def("SetIntegralTerm",&util::PIDController::SetIntegralTerm);
	defPIDController.def("SetDerivativeTerm",&util::PIDController::SetDerivativeTerm);
	defPIDController.def("SetTerms",&util::PIDController::SetTerms);
	defPIDController.def("GetProportionalTerm",&util::PIDController::GetProportionalTerm);
	defPIDController.def("GetIntegralTerm",&util::PIDController::GetIntegralTerm);
	defPIDController.def("GetDerivativeTerm",&util::PIDController::GetDerivativeTerm);
	defPIDController.def("GetTerms",static_cast<void(*)(lua_State*,const util::PIDController&)>([](lua_State *l,const util::PIDController &pidController) {
		auto p = 0.f;
		auto i = 0.f;
		auto d = 0.f;
		pidController.GetTerms(p,i,d);
		Lua::PushNumber(l,p);
		Lua::PushNumber(l,i);
		Lua::PushNumber(l,d);
	}));
	defPIDController.def("SetRange",&util::PIDController::SetRange);
	defPIDController.def("GetRange",static_cast<void(*)(lua_State*,const util::PIDController&)>([](lua_State *l,const util::PIDController &pidController) {
		auto range = pidController.GetRange();
		Lua::PushNumber(l,range.first);
		Lua::PushNumber(l,range.second);
	}));
	defPIDController.def("Calculate",&util::PIDController::Calculate);
	defPIDController.def("Reset",&util::PIDController::Reset);
	defPIDController.def("ClearRange",&util::PIDController::ClearRange);
	defPIDController.def("SetMin",&util::PIDController::SetMin);
	defPIDController.def("SetMax",&util::PIDController::SetMax);
	modMath[defPIDController];

	// Noise
	auto defNoiseModule = luabind::class_<NoiseBaseModule>("NoiseModule");
	defNoiseModule.def("GetValue",&Lua_NoiseModule_GetValue);
	defNoiseModule.def("SetScale",&Lua_NoiseModule_SetScale);
	
	auto defNoiseAbs = luabind::class_<NoiseAbs,NoiseBaseModule>("Abs");
	defNoiseModule.scope[defNoiseAbs];

	auto defNoiseBillow = luabind::class_<NoiseBillow,NoiseBaseModule>("Billow");
	defNoiseBillow.def("GetFrequency",&Lua_BillowNoise_GetFrequency);
	defNoiseBillow.def("GetLacunarity",&Lua_BillowNoise_GetLacunarity);
	defNoiseBillow.def("GetNoiseQuality",&Lua_BillowNoise_GetNoiseQuality);
	defNoiseBillow.def("GetOctaveCount",&Lua_BillowNoise_GetOctaveCount);
	defNoiseBillow.def("GetPersistence",&Lua_BillowNoise_GetPersistence);
	defNoiseBillow.def("GetSeed",&Lua_BillowNoise_GetSeed);
	defNoiseBillow.def("SetFrequency",&Lua_BillowNoise_SetFrequency);
	defNoiseBillow.def("SetLacunarity",&Lua_BillowNoise_SetLacunarity);
	defNoiseBillow.def("SetNoiseQuality",&Lua_BillowNoise_SetNoiseQuality);
	defNoiseBillow.def("SetOctaveCount",&Lua_BillowNoise_SetOctaveCount);
	defNoiseBillow.def("SetPersistence",&Lua_BillowNoise_SetPersistence);
	defNoiseBillow.def("SetSeed",&Lua_BillowNoise_SetSeed);
	defNoiseModule.scope[defNoiseBillow];

	auto defNoiseBlend = luabind::class_<NoiseBlend,NoiseBaseModule>("Blend");
	defNoiseModule.scope[defNoiseBlend];

	auto defNoiseCache = luabind::class_<NoiseCache,NoiseBaseModule>("Cache");
	defNoiseModule.scope[defNoiseCache];

	auto defNoiseCheckerboard = luabind::class_<NoiseCheckerboard,NoiseBaseModule>("Checkerboard");
	defNoiseModule.scope[defNoiseCheckerboard];

	auto defNoiseClamp = luabind::class_<NoiseClamp,NoiseBaseModule>("Clamp");
	defNoiseModule.scope[defNoiseClamp];

	auto defNoiseConst = luabind::class_<NoiseConst,NoiseBaseModule>("Const");
	defNoiseModule.scope[defNoiseConst];

	auto defNoiseCurve = luabind::class_<NoiseCurve,NoiseBaseModule>("Curve");
	defNoiseModule.scope[defNoiseCurve];

	auto defNoiseCylinders = luabind::class_<NoiseCylinders,NoiseBaseModule>("Cylinders");
	defNoiseModule.scope[defNoiseCylinders];

	auto defNoiseDisplace = luabind::class_<NoiseDisplace,NoiseBaseModule>("Displace");
	defNoiseModule.scope[defNoiseDisplace];

	auto defNoiseExponent = luabind::class_<NoiseExponent,NoiseBaseModule>("Exponent");
	defNoiseModule.scope[defNoiseExponent];

	auto defNoiseInvert = luabind::class_<NoiseInvert,NoiseBaseModule>("Invert");
	defNoiseModule.scope[defNoiseInvert];

	auto defNoiseMax = luabind::class_<NoiseMax,NoiseBaseModule>("Max");
	defNoiseModule.scope[defNoiseMax];

	auto defNoiseMin = luabind::class_<NoiseMin,NoiseBaseModule>("Min");
	defNoiseModule.scope[defNoiseMin];

	auto defNoiseMultiply = luabind::class_<NoiseMultiply,NoiseBaseModule>("Multiply");
	defNoiseModule.scope[defNoiseMultiply];

	auto defNoisePerlin = luabind::class_<NoisePerlin,NoiseBaseModule>("Perlin");
	defNoisePerlin.def("GetFrequency",&Lua_PerlinNoise_GetFrequency);
	defNoisePerlin.def("GetLacunarity",&Lua_PerlinNoise_GetLacunarity);
	defNoisePerlin.def("GetNoiseQuality",&Lua_PerlinNoise_GetNoiseQuality);
	defNoisePerlin.def("GetOctaveCount",&Lua_PerlinNoise_GetOctaveCount);
	defNoisePerlin.def("GetPersistence",&Lua_PerlinNoise_GetPersistence);
	defNoisePerlin.def("GetSeed",&Lua_PerlinNoise_GetSeed);
	defNoisePerlin.def("SetFrequency",&Lua_PerlinNoise_SetFrequency);
	defNoisePerlin.def("SetLacunarity",&Lua_PerlinNoise_SetLacunarity);
	defNoisePerlin.def("SetNoiseQuality",&Lua_PerlinNoise_SetNoiseQuality);
	defNoisePerlin.def("SetOctaveCount",&Lua_PerlinNoise_SetOctaveCount);
	defNoisePerlin.def("SetPersistence",&Lua_PerlinNoise_SetPersistence);
	defNoisePerlin.def("SetSeed",&Lua_PerlinNoise_SetSeed);
	defNoiseModule.scope[defNoisePerlin];

	auto defNoisePower = luabind::class_<NoisePower,NoiseBaseModule>("Power");
	defNoiseModule.scope[defNoisePower];

	auto defNoiseRidgedMulti = luabind::class_<NoiseRidgedMulti,NoiseBaseModule>("RidgedMulti");
	defNoiseRidgedMulti.def("GetFrequency",&Lua_RidgedMultiNoise_GetFrequency);
	defNoiseRidgedMulti.def("GetLacunarity",&Lua_RidgedMultiNoise_GetLacunarity);
	defNoiseRidgedMulti.def("GetNoiseQuality",&Lua_RidgedMultiNoise_GetNoiseQuality);
	defNoiseRidgedMulti.def("GetOctaveCount",&Lua_RidgedMultiNoise_GetOctaveCount);
	defNoiseRidgedMulti.def("GetSeed",&Lua_RidgedMultiNoise_GetSeed);
	defNoiseRidgedMulti.def("SetFrequency",&Lua_RidgedMultiNoise_SetFrequency);
	defNoiseRidgedMulti.def("SetLacunarity",&Lua_RidgedMultiNoise_SetLacunarity);
	defNoiseRidgedMulti.def("SetNoiseQuality",&Lua_RidgedMultiNoise_SetNoiseQuality);
	defNoiseRidgedMulti.def("SetOctaveCount",&Lua_RidgedMultiNoise_SetOctaveCount);
	defNoiseRidgedMulti.def("SetSeed",&Lua_RidgedMultiNoise_SetSeed);
	defNoiseModule.scope[defNoiseRidgedMulti];

	auto defNoiseRotatePoint = luabind::class_<NoiseRotatePoint,NoiseBaseModule>("RotatePoint");
	defNoiseModule.scope[defNoiseRotatePoint];

	auto defNoiseScaleBias = luabind::class_<NoiseScaleBias,NoiseBaseModule>("ScaleBias");
	defNoiseModule.scope[defNoiseScaleBias];

	auto noiseScalePoint = luabind::class_<NoiseScalePoint,NoiseBaseModule>("ScalePoint");
	defNoiseModule.scope[noiseScalePoint];

	auto noiseSelect = luabind::class_<NoiseSelect,NoiseBaseModule>("Select");
	defNoiseModule.scope[noiseSelect];

	auto noiseSpheres = luabind::class_<NoiseSpheres,NoiseBaseModule>("Spheres");
	defNoiseModule.scope[noiseSpheres];

	auto noiseTerrace = luabind::class_<NoiseTerrace,NoiseBaseModule>("Terrace");
	defNoiseModule.scope[noiseTerrace];

	auto noiseTransplatePoint = luabind::class_<NoiseTranslatePoint,NoiseBaseModule>("TranslatePoint");
	defNoiseModule.scope[noiseTransplatePoint];

	auto noiseTurbulance = luabind::class_<NoiseTurbulance,NoiseBaseModule>("Turbulance");
	defNoiseModule.scope[noiseTurbulance];

	auto noiseVoroni = luabind::class_<NoiseVoronoi,NoiseBaseModule>("Voronoi");
	noiseVoroni.def("GetDisplacement",&Lua_VoronoiNoise_GetDisplacement);
	noiseVoroni.def("GetFrequency",&Lua_VoronoiNoise_GetFrequency);
	noiseVoroni.def("GetSeed",&Lua_VoronoiNoise_GetSeed);
	noiseVoroni.def("SetDisplacement",&Lua_VoronoiNoise_SetDisplacement);
	noiseVoroni.def("SetFrequency",&Lua_VoronoiNoise_SetFrequency);
	noiseVoroni.def("SetSeed",&Lua_VoronoiNoise_SetSeed);
	defNoiseModule.scope[noiseVoroni];

	modMath[defNoiseModule];

	auto noiseMap = luabind::class_<noise::utils::NoiseMap>("NoiseMap");
	noiseMap.def("GetValue",&Lua_NoiseMap_GetValue);
	noiseMap.def("GetHeight",&Lua_NoiseMap_GetHeight);
	noiseMap.def("GetWidth",&Lua_NoiseMap_GetWidth);
	modMath[noiseMap];
	//

	auto defVectori = luabind::class_<Vector3i>("Vectori");
	defVectori.def(luabind::constructor<>());
	defVectori.def(luabind::constructor<int32_t,int32_t,int32_t>());
	defVectori.def(luabind::tostring(luabind::self));
	defVectori.def(-luabind::const_self);
	defVectori.def_readwrite("x",&Vector3i::x);
	defVectori.def_readwrite("y",&Vector3i::y);
	defVectori.def_readwrite("z",&Vector3i::z);
	defVectori.def(luabind::const_self /int32_t());
	defVectori.def(luabind::const_self *int32_t());
	defVectori.def(luabind::const_self +luabind::const_self);
	defVectori.def(luabind::const_self -luabind::const_self);
	defVectori.def(luabind::const_self ==luabind::const_self);
	defVectori.def(int32_t() /luabind::const_self);
	defVectori.def(int32_t() *luabind::const_self);
	defVectori.def("Copy",&Lua::Vectori::Copy);
	defVectori.def("Get",static_cast<void(*)(lua_State*,const Vector3i&,uint32_t)>([](lua_State *l,const Vector3i &v,uint32_t idx) {
		Lua::PushInt(l,v[idx]);
	}));
	modMath[defVectori];
	
	auto defVector2i = luabind::class_<Vector2i>("Vector2i");
	defVector2i.def(luabind::constructor<>());
	defVector2i.def(luabind::constructor<int32_t,int32_t>());
	defVector2i.def(luabind::tostring(luabind::self));
	defVector2i.def(-luabind::const_self);
	defVector2i.def_readwrite("x",&Vector2i::x);
	defVector2i.def_readwrite("y",&Vector2i::y);
	defVector2i.def(luabind::const_self /int32_t());
	defVector2i.def(luabind::const_self *int32_t());
	defVector2i.def(luabind::const_self +luabind::const_self);
	defVector2i.def(luabind::const_self -luabind::const_self);
	defVector2i.def(luabind::const_self ==luabind::const_self);
	defVector2i.def(int32_t() /luabind::const_self);
	defVector2i.def(int32_t() *luabind::const_self);
	defVector2i.def("Copy",&Lua::Vector2i::Copy);
	defVector2i.def("Get",static_cast<void(*)(lua_State*,const Vector2i&,uint32_t)>([](lua_State *l,const Vector2i &v,uint32_t idx) {
		Lua::PushInt(l,v[idx]);
	}));
	modMath[defVector2i];

	auto defVector4i = luabind::class_<Vector4i>("Vector4i");
	defVector4i.def(luabind::constructor<>());
	defVector4i.def(luabind::constructor<int32_t,int32_t,int32_t,int32_t>());
	defVector4i.def(luabind::tostring(luabind::self));
	defVector4i.def(-luabind::const_self);
	defVector4i.def_readwrite("w",&Vector4i::w);
	defVector4i.def_readwrite("x",&Vector4i::x);
	defVector4i.def_readwrite("y",&Vector4i::y);
	defVector4i.def_readwrite("z",&Vector4i::z);
	defVector4i.def(luabind::const_self /int32_t());
	defVector4i.def(luabind::const_self *int32_t());
	defVector4i.def(luabind::const_self +luabind::const_self);
	defVector4i.def(luabind::const_self -luabind::const_self);
	defVector4i.def(luabind::const_self ==luabind::const_self);
	defVector4i.def(luabind::const_self *luabind::const_self);
	defVector4i.def(int32_t() /luabind::const_self);
	defVector4i.def(int32_t() *luabind::const_self);
	defVector4i.def("Copy",&Lua::Vector4i::Copy);
	defVector4i.def("Get",static_cast<void(*)(lua_State*,const Vector4i&,uint32_t)>([](lua_State *l,const Vector4i &v,uint32_t idx) {
		Lua::PushInt(l,v[idx]);
	}));
	modMath[defVector4i];

	auto defVector = luabind::class_<Vector3>("Vector");
	defVector.def(luabind::constructor<>());
	defVector.def(luabind::constructor<float,float,float>());
	defVector.def(luabind::constructor<const Vector2&,float>());
	defVector.def(luabind::tostring(luabind::self));
	defVector.def(-luabind::const_self);
	defVector.def_readwrite("x",&Vector3::x);
	defVector.def_readwrite("y",&Vector3::y);
	defVector.def_readwrite("z",&Vector3::z);
	defVector.def_readwrite("r",&Vector3::r);
	defVector.def_readwrite("g",&Vector3::g);
	defVector.def_readwrite("b",&Vector3::b);
	defVector.def(luabind::const_self /float());
	defVector.def(luabind::const_self *float());
	defVector.def(luabind::const_self *Vector3());
	defVector.def(luabind::const_self +luabind::const_self);
	defVector.def(luabind::const_self -luabind::const_self);
	defVector.def(luabind::const_self ==luabind::const_self);
	defVector.def(luabind::const_self *Quat());
	//defVector.def(luabind::const_self *umath::Transform());
	//defVector.def(luabind::const_self *umath::ScaledTransform());
	defVector.def("Mul",static_cast<void(*)(lua_State*,const Vector3&,const umath::Transform&)>([](lua_State *l,const Vector3 &a,const umath::Transform &b) {
		Lua::Push<Vector3>(l,a *b);
	}));
	defVector.def("Mul",static_cast<void(*)(lua_State*,const Vector3&,const umath::ScaledTransform&)>([](lua_State *l,const Vector3 &a,const umath::ScaledTransform &b) {
		Lua::Push<Vector3>(l,a *b);
	}));
	defVector.def(float() /luabind::const_self);
	defVector.def(float() *luabind::const_self);
	defVector.def(Quat() *luabind::const_self);
	defVector.def("GetNormal",uvec::get_normal);
	defVector.def("Normalize",&Lua::Vector::Normalize);
	defVector.def("ToEulerAngles",static_cast<EulerAngles(*)(const Vector3&)>(uvec::to_angle));
	defVector.def("Length",uvec::length);
	defVector.def("LengthSqr",uvec::length_sqr);
	defVector.def("Distance",uvec::distance);
	defVector.def("DistanceSqr",uvec::distance_sqr);
	defVector.def("PlanarDistance",uvec::planar_distance);
	defVector.def("PlanarDistanceSqr",uvec::planar_distance_sqr);
	defVector.def("Cross",uvec::cross);
	defVector.def("DotProduct",uvec::dot);
	defVector.def("GetRotation",uvec::get_rotation);
	defVector.def("Rotate",static_cast<void(*)(lua_State*,Vector3&,const EulerAngles&)>(&Lua::Vector::Rotate));
	defVector.def("Rotate",static_cast<void(*)(lua_State*,Vector3&,const Vector3&,float)>(&Lua::Vector::Rotate));
	defVector.def("Rotate",static_cast<void(*)(lua_State*,Vector3&,const Quat&)>(&Lua::Vector::Rotate));
	defVector.def("RotateAround",&Lua::Vector::RotateAround);
	defVector.def("Lerp",&Lua::Vector::Lerp);
	defVector.def("Equals",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,float)>([](lua_State *l,const Vector3 &a,const Vector3 &b,float epsilon) {
		Lua::PushBool(l,umath::abs(a.x -b.x) <= epsilon && umath::abs(a.y -b.y) <= epsilon && umath::abs(a.z -b.z) <= epsilon);
	}));
	defVector.def("Equals",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&)>([](lua_State *l,const Vector3 &a,const Vector3 &b) {
		float epsilon = 0.001f;
		Lua::PushBool(l,umath::abs(a.x -b.x) <= epsilon && umath::abs(a.y -b.y) <= epsilon && umath::abs(a.z -b.z) <= epsilon);
	}));
	defVector.def("Slerp",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,float)>([](lua_State *l,const Vector3 &a,const Vector3 &b,float factor) {
		auto result = glm::slerp(a,b,factor);
		Lua::Push<Vector3>(l,result);
	}));
	defVector.def("Copy",&Lua::Vector::Copy);
	defVector.def("Set",static_cast<void(*)(lua_State*,Vector3&,const Vector3&)>(&Lua::Vector::Set));
	defVector.def("Set",static_cast<void(*)(lua_State*,Vector3&,float,float,float)>(&Lua::Vector::Set));
	defVector.def("Set",static_cast<void(*)(lua_State*,Vector3&,uint32_t,float)>([](lua_State *l,Vector3 &v,uint32_t idx,float val) {
		v[idx] = val;
	}));
	defVector.def("Get",static_cast<void(*)(lua_State*,const Vector3&,uint32_t)>([](lua_State *l,const Vector3 &v,uint32_t idx) {
		Lua::PushNumber(l,v[idx]);
	}));
	defVector.def("ToMatrix",&Lua::Vector::ToMatrix);
	defVector.def("SnapToGrid",static_cast<void(*)(lua_State*,Vector3&)>(&Lua::Vector::SnapToGrid));
	defVector.def("SnapToGrid",static_cast<void(*)(lua_State*,Vector3&,UInt32)>(&Lua::Vector::SnapToGrid));
	defVector.def("Project",uvec::project);
	defVector.def("ProjectToPlane",uvec::project_to_plane);
	defVector.def("GetPerpendicular",uvec::get_perpendicular);
	defVector.def("OuterProduct",&uvec::calc_outer_product);
	modMath[defVector];

	auto defVector2 = luabind::class_<Vector2>("Vector2");
	defVector2.def(luabind::constructor<>());
	defVector2.def(luabind::constructor<float,float>());
	defVector2.def(luabind::tostring(luabind::self));
	defVector2.def(-luabind::const_self);
	defVector2.def_readwrite("x",&Vector2::x);
	defVector2.def_readwrite("y",&Vector2::y);
	defVector2.def(luabind::const_self /float());
	defVector2.def(luabind::const_self *float());
	defVector2.def(luabind::const_self *Vector2());
	defVector2.def(luabind::const_self +luabind::const_self);
	defVector2.def(luabind::const_self -luabind::const_self);
	defVector2.def(luabind::const_self ==luabind::const_self);
	defVector2.def(float() /luabind::const_self);
	defVector2.def(float() *luabind::const_self);
	defVector2.def("GetNormal",&Lua::Vector2::GetNormal);
	defVector2.def("Normalize",&Lua::Vector2::Normalize);
	defVector2.def("Length",&Lua::Vector2::Length);
	defVector2.def("LengthSqr",&Lua::Vector2::LengthSqr);
	defVector2.def("Distance",&Lua::Vector2::Distance);
	defVector2.def("DistanceSqr",&Lua::Vector2::DistanceSqr);
	defVector2.def("Cross",&Lua::Vector2::Cross);
	defVector2.def("DotProduct",&Lua::Vector2::DotProduct);
	defVector2.def("Rotate",&Lua::Vector2::Rotate);
	defVector2.def("RotateAround",&Lua::Vector2::RotateAround);
	defVector2.def("Lerp",&Lua::Vector2::Lerp);
	defVector2.def("Copy",&Lua::Vector2::Copy);
	defVector2.def("Set",static_cast<void(*)(lua_State*,Vector2&,const Vector2&)>(&Lua::Vector2::Set));
	defVector2.def("Set",static_cast<void(*)(lua_State*,Vector2&,float,float)>(&Lua::Vector2::Set));
	defVector2.def("Get",static_cast<void(*)(lua_State*,const Vector2&,uint32_t)>([](lua_State *l,const Vector2 &v,uint32_t idx) {
		Lua::PushNumber(l,v[idx]);
	}));
	defVector2.def("Project",&Lua::Vector2::Project);
	modMath[defVector2];

	auto defVector4 = luabind::class_<Vector4>("Vector4");
	defVector4.def(luabind::constructor<>());
	defVector4.def(luabind::constructor<float,float,float,float>());
	defVector4.def(luabind::constructor<const Vector3&,float>());
	defVector4.def(luabind::tostring(luabind::self));
	defVector4.def(-luabind::const_self);
	defVector4.def_readwrite("w",&Vector4::w);
	defVector4.def_readwrite("x",&Vector4::x);
	defVector4.def_readwrite("y",&Vector4::y);
	defVector4.def_readwrite("z",&Vector4::z);
	defVector4.def_readwrite("r",&Vector4::r);
	defVector4.def_readwrite("g",&Vector4::g);
	defVector4.def_readwrite("b",&Vector4::b);
	defVector4.def_readwrite("a",&Vector4::a);
	defVector4.def(luabind::const_self /float());
	defVector4.def(luabind::const_self *float());
	defVector4.def(luabind::const_self *Vector4());
	defVector4.def(luabind::const_self +luabind::const_self);
	defVector4.def(luabind::const_self -luabind::const_self);
	defVector4.def(luabind::const_self ==luabind::const_self);
	defVector4.def(luabind::const_self *Mat4());
	defVector4.def(float() /luabind::const_self);
	defVector4.def(float() *luabind::const_self);
	defVector4.def("GetNormal",&Lua::Vector4::GetNormal);
	defVector4.def("Normalize",&Lua::Vector4::Normalize);
	defVector4.def("Length",&Lua::Vector4::Length);
	defVector4.def("LengthSqr",&Lua::Vector4::LengthSqr);
	defVector4.def("Distance",&Lua::Vector4::Distance);
	defVector4.def("DistanceSqr",&Lua::Vector4::DistanceSqr);
	defVector4.def("DotProduct",&Lua::Vector4::DotProduct);
	defVector4.def("Lerp",&Lua::Vector4::Lerp);
	defVector4.def("Copy",&Lua::Vector4::Copy);
	defVector4.def("Set",static_cast<void(*)(lua_State*,Vector4&,const Vector4&)>(&Lua::Vector4::Set));
	defVector4.def("Set",static_cast<void(*)(lua_State*,Vector4&,float,float,float,float)>(&Lua::Vector4::Set));
	defVector4.def("Get",static_cast<void(*)(lua_State*,const Vector4&,uint32_t)>([](lua_State *l,const Vector4 &v,uint32_t idx) {
		Lua::PushNumber(l,v[idx]);
	}));
	defVector4.def("Project",&Lua::Vector4::Project);
	modMath[defVector4];

	auto defEulerAngles = luabind::class_<EulerAngles>("EulerAngles");
	defEulerAngles.def(luabind::constructor<>());
	defEulerAngles.def(luabind::constructor<float,float,float>());
	defEulerAngles.def(luabind::constructor<const EulerAngles&>());
	defEulerAngles.def(luabind::constructor<const Mat4&>());
	defEulerAngles.def(luabind::constructor<const Vector3&>());
	defEulerAngles.def(luabind::constructor<const Vector3&,const Vector3&>());
	defEulerAngles.def(luabind::constructor<const Quat&>());
	defEulerAngles.def(luabind::tostring(luabind::self));
	defEulerAngles.def(-luabind::const_self);
	defEulerAngles.def_readwrite("p",&EulerAngles::p);
	defEulerAngles.def_readwrite("y",&EulerAngles::y);
	defEulerAngles.def_readwrite("r",&EulerAngles::r);
	defEulerAngles.def(luabind::const_self /float());
	defEulerAngles.def(luabind::const_self *float());
	defEulerAngles.def(luabind::const_self +luabind::const_self);
	defEulerAngles.def(luabind::const_self -luabind::const_self);
	defEulerAngles.def(luabind::const_self ==luabind::const_self);
	defEulerAngles.def(float() *luabind::const_self);
	defEulerAngles.def("GetForward",&EulerAngles::Forward);
	defEulerAngles.def("GetRight",&EulerAngles::Right);
	defEulerAngles.def("GetUp",&Lua::Angle::Up);
	defEulerAngles.def("GetOrientation",&Lua::Angle::Orientation);
	defEulerAngles.def("Normalize",static_cast<void(EulerAngles::*)()>(&EulerAngles::Normalize));
	defEulerAngles.def("Normalize",static_cast<void(EulerAngles::*)(float)>(&EulerAngles::Normalize));
	defEulerAngles.def("ToMatrix",&Lua::Angle::ToMatrix);
	defEulerAngles.def("Copy",&Lua::Angle::Copy);
	defEulerAngles.def("Equals",static_cast<void(*)(lua_State*,const EulerAngles&,const EulerAngles&,float)>([](lua_State *l,const EulerAngles &a,const EulerAngles &b,float epsilon) {
		Lua::PushBool(l,umath::abs(a.p -b.p) <= epsilon && umath::abs(a.y -b.y) <= epsilon && umath::abs(a.r -b.r) <= epsilon);
		}));
	defEulerAngles.def("Equals",static_cast<void(*)(lua_State*,const EulerAngles&,const EulerAngles&)>([](lua_State *l,const EulerAngles &a,const EulerAngles &b) {
		float epsilon = 0.001f;
		Lua::PushBool(l,umath::abs(a.p -b.p) <= epsilon && umath::abs(a.y -b.y) <= epsilon && umath::abs(a.r -b.r) <= epsilon);
	}));
	defEulerAngles.def("ToQuaternion",Lua::Angle::ToQuaternion);
	defEulerAngles.def("ToQuaternion",static_cast<void(*)(lua_State*,EulerAngles*)>([](lua_State *l,EulerAngles *ang) {
		Lua::Angle::ToQuaternion(l,ang,umath::to_integral(pragma::RotationOrder::YXZ));
	}));
	defEulerAngles.def("Set",static_cast<void(EulerAngles::*)(const EulerAngles&)>(&EulerAngles::Set));
	defEulerAngles.def("Set",&Lua::Angle::Set);
	defEulerAngles.def("Set",static_cast<void(*)(lua_State*,EulerAngles&,uint32_t,float value)>([](lua_State *l,EulerAngles &ang,uint32_t idx,float value) {
		ang[idx] = value;
	}));
	defEulerAngles.def("Get",static_cast<void(*)(lua_State*,const EulerAngles&,uint32_t)>([](lua_State *l,const EulerAngles &ang,uint32_t idx) {
		Lua::PushNumber(l,ang[idx]);
	}));
	modMath[defEulerAngles];

	modMath[
		luabind::def("Quaternion",static_cast<Quat(*)()>(&QuaternionConstruct)) COMMA
		luabind::def("Quaternion",static_cast<Quat(*)(float,float,float,float)>(&QuaternionConstruct)) COMMA
		luabind::def("Quaternion",static_cast<Quat(*)(const Vector3&,float)>(&QuaternionConstruct)) COMMA
		luabind::def("Quaternion",static_cast<Quat(*)(const Vector3&,const Vector3&,const Vector3&)>(&QuaternionConstruct)) COMMA
		luabind::def("Quaternion",static_cast<Quat(*)(const Quat&)>(&QuaternionConstruct)) COMMA
		luabind::def("Quaternion",static_cast<Quat(*)(const Vector3&,const Vector3&)>(&QuaternionConstruct))
	];
	auto defQuat = luabind::class_<Quat>("QuaternionInternal");
	defQuat.def(luabind::tostring(luabind::self));
	defQuat.def_readwrite("w",&Quat::w);
	defQuat.def_readwrite("x",&Quat::x);
	defQuat.def_readwrite("y",&Quat::y);
	defQuat.def_readwrite("z",&Quat::z);
	defQuat.def(-luabind::const_self);
	defQuat.def(luabind::const_self /float());
	defQuat.def(luabind::const_self *float());
	defQuat.def(luabind::const_self *luabind::const_self);
	defQuat.def(luabind::const_self ==luabind::const_self);
	//defQuat.def(luabind::const_self *umath::Transform());
	//defQuat.def(luabind::const_self *umath::ScaledTransform());
	defQuat.def("Mul",static_cast<void(*)(lua_State*,const Quat&,const umath::Transform&)>([](lua_State *l,const Quat &a,const umath::Transform &b) {
		Lua::Push<Quat>(l,a *b);
		}));
	defQuat.def("Mul",static_cast<void(*)(lua_State*,const Quat&,const umath::ScaledTransform&)>([](lua_State *l,const Quat &a,const umath::ScaledTransform &b) {
		Lua::Push<Quat>(l,a *b);
	}));
	defQuat.def(float() *luabind::const_self);
	defQuat.def("GetForward",&Lua::Quaternion::GetForward);
	defQuat.def("GetRight",&Lua::Quaternion::GetRight);
	defQuat.def("GetUp",&Lua::Quaternion::GetUp);
	defQuat.def("GetOrientation",&Lua::Quaternion::GetOrientation);
	defQuat.def("DotProduct",&Lua::Quaternion::DotProduct);
	defQuat.def("Inverse",&Lua::Quaternion::Inverse);
	defQuat.def("GetInverse",&Lua::Quaternion::GetInverse);
	defQuat.def("Length",&Lua::Quaternion::Length);
	defQuat.def("Normalize",&Lua::Quaternion::Normalize);
	defQuat.def("GetNormal",&Lua::Quaternion::GetNormal);
	defQuat.def("Copy",&Lua::Quaternion::Copy);
	defQuat.def("ToMatrix",&Lua::Quaternion::ToMatrix);
	defQuat.def("Lerp",&Lua::Quaternion::Lerp);
	defQuat.def("Slerp",&Lua::Quaternion::Slerp);
	defQuat.def("ToEulerAngles",Lua::Quaternion::ToEulerAngles);
	defQuat.def("ToEulerAngles",static_cast<void(*)(lua_State*,Quat&)>([](lua_State *l,Quat &rot) {
		Lua::Quaternion::ToEulerAngles(l,rot,umath::to_integral(pragma::RotationOrder::YXZ));
	}));
	defQuat.def("ToAxisAngle",&Lua::Quaternion::ToAxisAngle);
	defQuat.def("Set",&Lua::Quaternion::Set);
	defQuat.def("Set",static_cast<void(*)(Quat&,const Quat&)>([](Quat &rot,const Quat &rotNew) {
		rot = rotNew;
	}));
	defQuat.def("Set",static_cast<void(*)(Quat&,uint32_t,float)>([](Quat &rot,uint32_t idx,float value) {
		constexpr std::array<uint8_t,4> quatIndices = {3,0,1,2};
		rot[quatIndices.at(idx)] = value;
	}));
	defQuat.def("Get",static_cast<void(*)(lua_State*,Quat&,uint32_t)>([](lua_State *l,Quat &rot,uint32_t idx) {
		constexpr std::array<uint8_t,4> quatIndices = {3,0,1,2};
		Lua::PushNumber(l,rot[quatIndices.at(idx)]);
	}));
	defQuat.def("RotateX",static_cast<void(*)(Quat&,float)>(&uquat::rotate_x));
	defQuat.def("RotateY",static_cast<void(*)(Quat&,float)>(&uquat::rotate_y));
	defQuat.def("RotateZ",static_cast<void(*)(Quat&,float)>(&uquat::rotate_z));
	defQuat.def("Rotate",static_cast<void(*)(Quat&,const Vector3&,float)>(&uquat::rotate));
	defQuat.def("Rotate",static_cast<void(*)(Quat&,const EulerAngles&)>(&uquat::rotate));
	defQuat.def("ApproachDirection",static_cast<void(*)(lua_State*,const Quat&,const Vector3&,const Vector3&,const ::Vector2&,const ::Vector2*,const ::Vector2*,const Quat*,const EulerAngles*)>(&Lua::Quaternion::ApproachDirection));
	defQuat.def("ApproachDirection",static_cast<void(*)(lua_State*,const Quat&,const Vector3&,const Vector3&,const ::Vector2&,const ::Vector2*,const ::Vector2*,const Quat*)>(&Lua::Quaternion::ApproachDirection));
	defQuat.def("ApproachDirection",static_cast<void(*)(lua_State*,const Quat&,const Vector3&,const Vector3&,const ::Vector2&,const ::Vector2*,const ::Vector2*)>(&Lua::Quaternion::ApproachDirection));
	defQuat.def("ApproachDirection",static_cast<void(*)(lua_State*,const Quat&,const Vector3&,const Vector3&,const ::Vector2&,const ::Vector2*)>(&Lua::Quaternion::ApproachDirection));
	defQuat.def("ApproachDirection",static_cast<void(*)(lua_State*,const Quat&,const Vector3&,const Vector3&,const ::Vector2&)>(&Lua::Quaternion::ApproachDirection));
	defQuat.def("GetConjugate",&Lua::Quaternion::GetConjugate);
	modMath[defQuat];
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

static bool operator==(const EntityHandle &v,const LEntityProperty &prop) {return **prop == v;}
static std::ostream& operator<<(std::ostream &str,const LEntityProperty &v)
{
	if((*v)->IsValid())
		(*v)->get()->print(str);
	else
		str<<"NULL";
	return str;
}
void Lua::Property::push(lua_State *l,pragma::EntityProperty &prop) {Lua::Property::push_property<LEntityPropertyWrapper>(l,prop);}

void Game::RegisterLuaClasses()
{
	NetworkState::RegisterSharedLuaClasses(GetLuaInterface());

	// Entity
	auto &modUtil = GetLuaInterface().RegisterLibrary("util");
	auto entDef = luabind::class_<LEntityProperty,LBasePropertyWrapper>("EntityProperty");
	Lua::Property::add_generic_methods<LEntityProperty,EntityHandle,luabind::class_<LEntityProperty,LBasePropertyWrapper>>(entDef);
	entDef.def(luabind::constructor<>());
	entDef.def(luabind::constructor<EntityHandle>());
	entDef.def(luabind::tostring(luabind::const_self));
	entDef.def("Link",static_cast<void(*)(lua_State*,LEntityProperty&,LEntityProperty&)>(&Lua::Property::link<LEntityProperty,EntityHandle>));
	modUtil[entDef];

	auto defSplashDamageInfo = luabind::class_<util::SplashDamageInfo>("SplashDamageInfo");
	defSplashDamageInfo.def(luabind::constructor<>());
	defSplashDamageInfo.def_readwrite("origin",&util::SplashDamageInfo::origin);
	defSplashDamageInfo.def_readwrite("radius",&util::SplashDamageInfo::radius);
	defSplashDamageInfo.def_readwrite("damageInfo",&util::SplashDamageInfo::damageInfo);
	defSplashDamageInfo.def("SetCone",static_cast<void(*)(lua_State*,util::SplashDamageInfo&,const Vector3&,float)>([](lua_State *l,util::SplashDamageInfo &splashDamageInfo,const Vector3 &coneDirection,float coneAngle) {
		splashDamageInfo.cone = {{coneDirection,coneAngle}};
	}));
	defSplashDamageInfo.def("SetCallback",static_cast<void(*)(lua_State*,util::SplashDamageInfo&,luabind::object)>([](lua_State *l,util::SplashDamageInfo &splashDamageInfo,luabind::object oCallback) {
		Lua::CheckFunction(l,2);
		splashDamageInfo.callback = [l,oCallback](BaseEntity *ent,DamageInfo &dmgInfo) -> bool {
			auto r = Lua::CallFunction(l,[ent,&dmgInfo,&oCallback](lua_State *l) -> Lua::StatusCode {
				oCallback.push(l);
				if(ent != nullptr)
					ent->GetLuaObject()->push(l);
				else
					Lua::PushNil(l);
				Lua::Push<DamageInfo*>(l,&dmgInfo);
				return Lua::StatusCode::Ok;
			},1);
			if(r == Lua::StatusCode::Ok && Lua::IsSet(l,-1))
				return Lua::CheckBool(l,-1);
			return false;
		};
	}));
	modUtil[defSplashDamageInfo];

	auto &modGame = GetLuaInterface().RegisterLibrary("game");
	auto defGmBase = luabind::class_<GameMode,luabind::no_bases,luabind::default_holder,GameModeWrapper>("Base");
	defGmBase.def(luabind::constructor<>());
	defGmBase.def("GetName",&Lua::GameMode::GetName);
	defGmBase.def("GetIdentifier",&Lua::GameMode::GetIdentifier);
	defGmBase.def("GetClassName",&Lua::GameMode::GetClassName);
	defGmBase.def("GetAuthor",&Lua::GameMode::GetAuthor);
	defGmBase.def("GetVersion",&Lua::GameMode::GetVersion);
	defGmBase.def("Think",&GameModeWrapper::LThink,&GameModeWrapper::default_Think);
	defGmBase.def("Tick",&GameModeWrapper::LTick,&GameModeWrapper::default_Tick);
	defGmBase.def("OnEntityTakeDamage",&GameModeWrapper::LOnEntityTakeDamage,&GameModeWrapper::default_OnEntityTakeDamage);
	defGmBase.def("OnEntityTakenDamage",&GameModeWrapper::LOnEntityTakenDamage,&GameModeWrapper::default_OnEntityTakenDamage);
	defGmBase.def("OnEntityHealthChanged",&GameModeWrapper::LOnEntityHealthChanged,&GameModeWrapper::default_OnEntityHealthChanged);
	defGmBase.def("OnPlayerDeath",&GameModeWrapper::LOnPlayerDeath,&GameModeWrapper::default_OnPlayerDeath);
	defGmBase.def("OnPlayerSpawned",&GameModeWrapper::LOnPlayerSpawned,&GameModeWrapper::default_OnPlayerSpawned);
	defGmBase.def("OnActionInput",&GameModeWrapper::LOnActionInput,&GameModeWrapper::default_OnActionInput);
	defGmBase.def("OnPlayerDropped",&GameModeWrapper::LOnPlayerDropped,&GameModeWrapper::default_OnPlayerDropped);
	defGmBase.def("OnPlayerReady",&GameModeWrapper::LOnPlayerReady,&GameModeWrapper::default_OnPlayerReady);
	defGmBase.def("OnPlayerJoined",&GameModeWrapper::LOnPlayerJoined,&GameModeWrapper::default_OnPlayerJoined);
	defGmBase.def("OnGameReady",&GameModeWrapper::LOnGameReady,&GameModeWrapper::default_OnGameReady);
	defGmBase.def("OnGameInitialized",&GameModeWrapper::LOnGameInitialized,&GameModeWrapper::default_OnGameInitialized);
	defGmBase.def("OnMapInitialized",&GameModeWrapper::LOnMapInitialized,&GameModeWrapper::default_OnMapInitialized);
	modGame[defGmBase];
	auto _G = luabind::globals(GetLuaState());
	_G["GMBase"] = _G["game"]["Base"];

	auto &modMath = m_lua->RegisterLibrary("math");
	auto defPlane = luabind::class_<Plane>("Plane");
	defPlane.def(luabind::constructor<Vector3,Vector3,Vector3>());
	defPlane.def(luabind::constructor<Vector3,Vector3>());
	defPlane.def(luabind::constructor<Vector3,double>());
	defPlane.def("Copy",static_cast<void(*)(lua_State*,Plane&)>([](lua_State *l,Plane &plane) {
		Lua::Push<Plane>(l,Plane{plane});
	}));
	defPlane.def("GetNormal",&Lua_Plane_GetNormal);
	defPlane.def("GetPos",&Lua_Plane_GetPos);
	defPlane.def("GetDistance",static_cast<void(*)(lua_State*,Plane&)>(&Lua_Plane_GetDistance));
	defPlane.def("GetDistance",static_cast<void(*)(lua_State*,Plane&,const Vector3&)>(&Lua_Plane_GetDistance));
	defPlane.def("MoveToPos",&Lua_Plane_MoveToPos);
	defPlane.def("Rotate",&Lua_Plane_Rotate);
	defPlane.def("GetCenterPos",&Lua_Plane_GetCenterPos);
	defPlane.def("Transform",static_cast<void(*)(lua_State*,Plane&,const Mat4&)>([](lua_State *l,Plane &plane,const Mat4 &transform) {
		const auto &n = plane.GetNormal();
		auto p = n *static_cast<float>(plane.GetDistance());
		auto n0 = uvec::get_perpendicular(n);
		uvec::normalize(&n0);
		auto n1 = uvec::cross(n,n0);
		uvec::normalize(&n1);
		auto p04 = transform *Vector4{p.x,p.y,p.z,1.f};
		auto p1 = p +n0 *10.f;
		auto p14 = transform *Vector4{p1.x,p1.y,p1.z,1.f};
		auto p2 = p +n1 *10.f;
		auto p24 = transform *Vector4{p2.x,p2.y,p2.z,1.f};
		plane = Plane{Vector3{p04.x,p04.y,p04.z},Vector3{p14.x,p14.y,p14.z},Vector3{p24.x,p24.y,p24.z}};
	}));
	modMath[defPlane];
}

LuaEntityIterator Lua::ents::create_lua_entity_iterator(lua_State *l,luabind::object oFilter,uint32_t idxFilter,EntityIterator::FilterFlags filterFlags)
{
	auto r = LuaEntityIterator{l,filterFlags};
	if(idxFilter != std::numeric_limits<uint32_t>::max())
	{
		auto t = idxFilter;
		Lua::CheckTable(l,t);
		auto numFilters = Lua::GetObjectLength(l,t);
		for(auto i=decltype(numFilters){0u};i<numFilters;++i)
		{
			Lua::PushInt(l,i +1u);
			Lua::GetTableValue(l,t);

			auto *filter = Lua::CheckEntityIteratorFilter(l,-1);
			r.AttachFilter(*filter);

			Lua::Pop(l,1);
		}
	}
	return r;
}

static std::optional<LuaEntityIterator> s_entIterator {}; // HACK: This is a workaround for a bug in luabind, which causes errors when compiled with gcc.
void Game::RegisterLuaGameClasses(luabind::module_ &gameMod)
{
	auto &modEnts = GetLuaInterface().RegisterLibrary("ents");
	RegisterLuaEntityComponents(modEnts);
	modEnts[
		luabind::def("iterator",static_cast<LuaEntityIterator&(*)(lua_State*)>([](lua_State *l) -> LuaEntityIterator& {
			s_entIterator = LuaEntityIterator{l};
			return *s_entIterator;
		}),luabind::return_stl_iterator{})
	];
	modEnts[
		luabind::def("iterator",static_cast<LuaEntityIterator&(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object oFilterOrFlags) -> LuaEntityIterator& {
			auto filterFlags = EntityIterator::FilterFlags::Default;
			auto filterIdx = 1u;
			if(Lua::IsNumber(l,1))
			{
				filterFlags = static_cast<EntityIterator::FilterFlags>(Lua::CheckInt(l,1));
				filterIdx = std::numeric_limits<uint32_t>::max();
			}
			s_entIterator = Lua::ents::create_lua_entity_iterator(l,oFilterOrFlags,filterIdx,filterFlags);
			return *s_entIterator;
		}),luabind::return_stl_iterator{})
	];
	modEnts[
		luabind::def("iterator",static_cast<LuaEntityIterator&(*)(lua_State*,luabind::object,luabind::object)>([](lua_State *l,luabind::object oFilterFlags,luabind::object oFilter) -> LuaEntityIterator& {
			Lua::CheckInt(l,1);
			auto filterFlags = static_cast<EntityIterator::FilterFlags>(Lua::CheckInt(l,1));
			s_entIterator = Lua::ents::create_lua_entity_iterator(l,oFilter,2u,filterFlags);
			return *s_entIterator;
		}),luabind::return_stl_iterator{})
	];

	auto defItFilter = luabind::class_<LuaEntityIteratorFilterBase>("IteratorFilter");
	modEnts[defItFilter];

	auto defItFilterClass = luabind::class_<LuaEntityIteratorFilterClass,LuaEntityIteratorFilterBase>("IteratorFilterClass");
	defItFilterClass.def(luabind::constructor<const std::string&>());
	defItFilterClass.def(luabind::constructor<const std::string&,bool>());
	defItFilterClass.def(luabind::constructor<const std::string&,bool,bool>());
	modEnts[defItFilterClass];

	auto defItFilterName = luabind::class_<LuaEntityIteratorFilterName,LuaEntityIteratorFilterBase>("IteratorFilterName");
	defItFilterName.def(luabind::constructor<const std::string&>());
	defItFilterName.def(luabind::constructor<const std::string&,bool>());
	defItFilterName.def(luabind::constructor<const std::string&,bool,bool>());
	modEnts[defItFilterName];

	auto defItFilterNameOrClass = luabind::class_<LuaEntityIteratorFilterNameOrClass,LuaEntityIteratorFilterBase>("IteratorFilterNameOrClass");
	defItFilterNameOrClass.def(luabind::constructor<const std::string&>());
	defItFilterNameOrClass.def(luabind::constructor<const std::string&,bool>());
	defItFilterNameOrClass.def(luabind::constructor<const std::string&,bool,bool>());
	modEnts[defItFilterNameOrClass];

	auto defItFilterEntity = luabind::class_<LuaEntityIteratorFilterEntity,LuaEntityIteratorFilterBase>("IteratorFilterEntity");
	defItFilterEntity.def(luabind::constructor<const std::string&>());
	modEnts[defItFilterEntity];

	auto defItFilterSphere = luabind::class_<LuaEntityIteratorFilterSphere,LuaEntityIteratorFilterBase>("IteratorFilterSphere");
	defItFilterSphere.def(luabind::constructor<const Vector3&,float>());
	modEnts[defItFilterSphere];

	auto defItFilterBox = luabind::class_<LuaEntityIteratorFilterBox,LuaEntityIteratorFilterBase>("IteratorFilterBox");
	defItFilterBox.def(luabind::constructor<const Vector3&,const Vector3&>());
	modEnts[defItFilterBox];

	auto defItFilterCone = luabind::class_<LuaEntityIteratorFilterCone,LuaEntityIteratorFilterBase>("IteratorFilterCone");
	defItFilterCone.def(luabind::constructor<const Vector3&,const Vector3&,float,float>());
	modEnts[defItFilterCone];

	auto defItFilterComponent = luabind::class_<LuaEntityIteratorFilterComponent,LuaEntityIteratorFilterBase>("IteratorFilterComponent");
	defItFilterComponent.def(luabind::constructor<pragma::ComponentId>());
	defItFilterComponent.def(luabind::constructor<lua_State*,const std::string&>());
	modEnts[defItFilterComponent];
	
	Lua::RegisterLibraryEnums(GetLuaState(),"ents",{
		{"ITERATOR_FILTER_BIT_NONE",umath::to_integral(EntityIterator::FilterFlags::None)},
		{"ITERATOR_FILTER_BIT_SPAWNED",umath::to_integral(EntityIterator::FilterFlags::Spawned)},
		{"ITERATOR_FILTER_BIT_PENDING",umath::to_integral(EntityIterator::FilterFlags::Pending)},
		{"ITERATOR_FILTER_BIT_INCLUDE_SHARED",umath::to_integral(EntityIterator::FilterFlags::IncludeShared)},
		{"ITERATOR_FILTER_BIT_INCLUDE_NETWORK_LOCAL",umath::to_integral(EntityIterator::FilterFlags::IncludeNetworkLocal)},

		{"ITERATOR_FILTER_BIT_CHARACTER",umath::to_integral(EntityIterator::FilterFlags::Character)},
		{"ITERATOR_FILTER_BIT_PLAYER",umath::to_integral(EntityIterator::FilterFlags::Player)},
		{"ITERATOR_FILTER_BIT_WEAPON",umath::to_integral(EntityIterator::FilterFlags::Weapon)},
		{"ITERATOR_FILTER_BIT_VEHICLE",umath::to_integral(EntityIterator::FilterFlags::Vehicle)},
		{"ITERATOR_FILTER_BIT_NPC",umath::to_integral(EntityIterator::FilterFlags::NPC)},
		{"ITERATOR_FILTER_BIT_PHYSICAL",umath::to_integral(EntityIterator::FilterFlags::Physical)},
		{"ITERATOR_FILTER_BIT_SCRIPTED",umath::to_integral(EntityIterator::FilterFlags::Scripted)},
		{"ITERATOR_FILTER_BIT_MAP_ENTITY",umath::to_integral(EntityIterator::FilterFlags::MapEntity)},

		{"ITERATOR_FILTER_BIT_HAS_TRANSFORM",umath::to_integral(EntityIterator::FilterFlags::HasTransform)},
		{"ITERATOR_FILTER_BIT_HAS_MODEL",umath::to_integral(EntityIterator::FilterFlags::HasModel)},

		{"ITERATOR_FILTER_ANY_TYPE",umath::to_integral(EntityIterator::FilterFlags::AnyType)},
		{"ITERATOR_FILTER_ANY",umath::to_integral(EntityIterator::FilterFlags::Any)},
		{"ITERATOR_FILTER_DEFAULT",umath::to_integral(EntityIterator::FilterFlags::Default)}
	});

	auto surfaceMatDef = luabind::class_<SurfaceMaterial>("SurfaceMaterial");
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_CUBIC",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::Cubic));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_GAUSSIAN",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::Gaussian));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_PRINCIPLED",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::Principled));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_BURLEY",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::Burley));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_RANDOM_WALK",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::RandomWalk));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_PRINCIPLED_RANDOM_WALK",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::PrincipledRandomWalk));
	surfaceMatDef.add_static_constant("SUBSURFACE_SCATTERING_METHOD_COUNT",umath::to_integral(SurfaceMaterial::PBRInfo::SubsurfaceMethod::Count));
	surfaceMatDef.def(luabind::tostring(luabind::self));
	surfaceMatDef.def("GetName",&Lua::SurfaceMaterial::GetName);
	surfaceMatDef.def("GetIndex",&Lua::SurfaceMaterial::GetIndex);
	surfaceMatDef.def("SetFriction",&Lua::SurfaceMaterial::SetFriction);
	surfaceMatDef.def("SetStaticFriction",&Lua::SurfaceMaterial::SetStaticFriction);
	surfaceMatDef.def("SetDynamicFriction",&Lua::SurfaceMaterial::SetDynamicFriction);
	surfaceMatDef.def("GetStaticFriction",&Lua::SurfaceMaterial::GetStaticFriction);
	surfaceMatDef.def("GetDynamicFriction",&Lua::SurfaceMaterial::GetDynamicFriction);
	surfaceMatDef.def("GetRestitution",&Lua::SurfaceMaterial::GetRestitution);
	surfaceMatDef.def("SetRestitution",&Lua::SurfaceMaterial::SetRestitution);
	surfaceMatDef.def("GetFootstepSound",&Lua::SurfaceMaterial::GetFootstepType);
	surfaceMatDef.def("SetFootstepSound",&Lua::SurfaceMaterial::SetFootstepType);
	surfaceMatDef.def("SetImpactParticleEffect",&Lua::SurfaceMaterial::SetImpactParticleEffect);
	surfaceMatDef.def("GetImpactParticleEffect",&Lua::SurfaceMaterial::GetImpactParticleEffect);
	surfaceMatDef.def("GetBulletImpactSound",&Lua::SurfaceMaterial::GetBulletImpactSound);
	surfaceMatDef.def("SetBulletImpactSound",&Lua::SurfaceMaterial::SetBulletImpactSound);
	surfaceMatDef.def("SetHardImpactSound",&Lua::SurfaceMaterial::SetHardImpactSound);
	surfaceMatDef.def("GetHardImpactSound",&Lua::SurfaceMaterial::GetHardImpactSound);
	surfaceMatDef.def("SetSoftImpactSound",&Lua::SurfaceMaterial::SetSoftImpactSound);
	surfaceMatDef.def("GetSoftImpactSound",&Lua::SurfaceMaterial::GetSoftImpactSound);

	surfaceMatDef.def("SetAudioLowFrequencyAbsorption",&Lua::SurfaceMaterial::SetAudioLowFrequencyAbsorption);
	surfaceMatDef.def("GetAudioLowFrequencyAbsorption",&Lua::SurfaceMaterial::GetAudioLowFrequencyAbsorption);
	surfaceMatDef.def("SetAudioMidFrequencyAbsorption",&Lua::SurfaceMaterial::SetAudioMidFrequencyAbsorption);
	surfaceMatDef.def("GetAudioMidFrequencyAbsorption",&Lua::SurfaceMaterial::GetAudioMidFrequencyAbsorption);
	surfaceMatDef.def("SetAudioHighFrequencyAbsorption",&Lua::SurfaceMaterial::SetAudioHighFrequencyAbsorption);
	surfaceMatDef.def("GetAudioHighFrequencyAbsorption",&Lua::SurfaceMaterial::GetAudioHighFrequencyAbsorption);
	surfaceMatDef.def("SetAudioScattering",&Lua::SurfaceMaterial::SetAudioScattering);
	surfaceMatDef.def("GetAudioScattering",&Lua::SurfaceMaterial::GetAudioScattering);
	surfaceMatDef.def("SetAudioLowFrequencyTransmission",&Lua::SurfaceMaterial::SetAudioLowFrequencyTransmission);
	surfaceMatDef.def("GetAudioLowFrequencyTransmission",&Lua::SurfaceMaterial::GetAudioLowFrequencyTransmission);
	surfaceMatDef.def("SetAudioMidFrequencyTransmission",&Lua::SurfaceMaterial::SetAudioMidFrequencyTransmission);
	surfaceMatDef.def("GetAudioMidFrequencyTransmission",&Lua::SurfaceMaterial::GetAudioMidFrequencyTransmission);
	surfaceMatDef.def("SetAudioHighFrequencyTransmission",&Lua::SurfaceMaterial::SetAudioHighFrequencyTransmission);
	surfaceMatDef.def("GetAudioHighFrequencyTransmission",&Lua::SurfaceMaterial::GetAudioHighFrequencyTransmission);

	surfaceMatDef.def("GetNavigationFlags",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushInt(l,umath::to_integral(surfMat.GetNavigationFlags()));
	}));
	surfaceMatDef.def("SetNavigationFlags",static_cast<void(*)(lua_State*,SurfaceMaterial&,uint32_t)>([](lua_State *l,SurfaceMaterial &surfMat,uint32_t navFlags) {
		surfMat.SetNavigationFlags(static_cast<pragma::nav::PolyFlags>(navFlags));
	}));
	surfaceMatDef.def("SetDensity",static_cast<void(*)(lua_State*,SurfaceMaterial&,float)>([](lua_State *l,SurfaceMaterial &surfMat,float density) {
		surfMat.SetDensity(density);
	}));
	surfaceMatDef.def("GetDensity",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetDensity());
	}));
	surfaceMatDef.def("SetLinearDragCoefficient",static_cast<void(*)(lua_State*,SurfaceMaterial&,float)>([](lua_State *l,SurfaceMaterial &surfMat,float coefficient) {
		surfMat.SetLinearDragCoefficient(coefficient);
	}));
	surfaceMatDef.def("GetLinearDragCoefficient",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetLinearDragCoefficient());
	}));
	surfaceMatDef.def("SetTorqueDragCoefficient",static_cast<void(*)(lua_State*,SurfaceMaterial&,float)>([](lua_State *l,SurfaceMaterial &surfMat,float coefficient) {
		surfMat.SetTorqueDragCoefficient(coefficient);
	}));
	surfaceMatDef.def("GetTorqueDragCoefficient",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetTorqueDragCoefficient());
	}));
	surfaceMatDef.def("SetWaveStiffness",static_cast<void(*)(lua_State*,SurfaceMaterial&,float)>([](lua_State *l,SurfaceMaterial &surfMat,float stiffness) {
		surfMat.SetWaveStiffness(stiffness);
	}));
	surfaceMatDef.def("GetWaveStiffness",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetWaveStiffness());
	}));
	surfaceMatDef.def("SetWavePropagation",static_cast<void(*)(lua_State*,SurfaceMaterial&,float)>([](lua_State *l,SurfaceMaterial &surfMat,float propagation) {
		surfMat.SetWavePropagation(propagation);
	}));
	surfaceMatDef.def("GetWavePropagation",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetWavePropagation());
	}));
	surfaceMatDef.def("GetPBRMetalness",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetPBRInfo().metalness);
	}));
	surfaceMatDef.def("GetPBRRoughness",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetPBRInfo().roughness);
	}));
	surfaceMatDef.def("GetSubsurfaceMultiplier",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushNumber(l,surfMat.GetPBRInfo().subsurfaceMultiplier);
	}));
	surfaceMatDef.def("GetSubsurfaceColor",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::Push<Color>(l,surfMat.GetPBRInfo().subsurfaceColor);
	}));
	surfaceMatDef.def("GetSubsurfaceMethod",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::PushInt(l,umath::to_integral(surfMat.GetPBRInfo().subsurfaceMethod));
	}));
	surfaceMatDef.def("GetSubsurfaceRadius",static_cast<void(*)(lua_State*,SurfaceMaterial&)>([](lua_State *l,SurfaceMaterial &surfMat) {
		Lua::Push<Vector3>(l,surfMat.GetPBRInfo().subsurfaceRadius);
	}));
	gameMod[surfaceMatDef];

	auto gibletCreateInfo = luabind::class_<GibletCreateInfo>("GibletCreateInfo");
	gibletCreateInfo.def(luabind::constructor<>());
	gibletCreateInfo.def_readwrite("model",&GibletCreateInfo::model);
	gibletCreateInfo.def_readwrite("skin",&GibletCreateInfo::skin);
	gibletCreateInfo.def_readwrite("scale",&GibletCreateInfo::scale);
	gibletCreateInfo.def_readwrite("mass",&GibletCreateInfo::mass);
	gibletCreateInfo.def_readwrite("lifetime",&GibletCreateInfo::lifetime);
	gibletCreateInfo.def_readwrite("position",&GibletCreateInfo::position);
	gibletCreateInfo.def_readwrite("rotation",&GibletCreateInfo::rotation);
	gibletCreateInfo.def_readwrite("velocity",&GibletCreateInfo::velocity);
	gibletCreateInfo.def_readwrite("angularVelocity",&GibletCreateInfo::angularVelocity);

	gibletCreateInfo.def_readwrite("physTranslationOffset",&GibletCreateInfo::physTranslationOffset);
	gibletCreateInfo.def_readwrite("physRotationOffset",&GibletCreateInfo::physRotationOffset);
	gibletCreateInfo.def_readwrite("physRadius",&GibletCreateInfo::physRadius);
	gibletCreateInfo.def_readwrite("physHeight",&GibletCreateInfo::physHeight);
	gibletCreateInfo.def_readwrite("physShape",reinterpret_cast<std::underlying_type_t<decltype(GibletCreateInfo::physShape)> GibletCreateInfo::*>(&GibletCreateInfo::physShape));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_MODEL",umath::to_integral(GibletCreateInfo::PhysShape::Model));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_NONE",umath::to_integral(GibletCreateInfo::PhysShape::None));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_SPHERE",umath::to_integral(GibletCreateInfo::PhysShape::Sphere));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_BOX",umath::to_integral(GibletCreateInfo::PhysShape::Box));
	gibletCreateInfo.add_static_constant("PHYS_SHAPE_CYLINDER",umath::to_integral(GibletCreateInfo::PhysShape::Cylinder));
	gameMod[gibletCreateInfo];

	auto bulletInfo = luabind::class_<BulletInfo>("BulletInfo");
	bulletInfo.def(luabind::constructor<>());
	bulletInfo.def_readwrite("spread",&BulletInfo::spread);
	bulletInfo.def_readwrite("force",&BulletInfo::force);
	bulletInfo.def_readwrite("distance",&BulletInfo::distance);
	bulletInfo.def_readwrite("damageType",reinterpret_cast<std::underlying_type_t<decltype(BulletInfo::damageType)> BulletInfo::*>(&BulletInfo::damageType));
	bulletInfo.def_readwrite("bulletCount",&BulletInfo::bulletCount);
	bulletInfo.def_readwrite("attacker",&BulletInfo::hAttacker);
	bulletInfo.def_readwrite("inflictor",&BulletInfo::hInflictor);
	bulletInfo.def_readwrite("tracerCount",&BulletInfo::tracerCount);
	bulletInfo.def_readwrite("tracerRadius",&BulletInfo::tracerRadius);
	bulletInfo.def_readwrite("tracerColor",&BulletInfo::tracerColor);
	bulletInfo.def_readwrite("tracerLength",&BulletInfo::tracerLength);
	bulletInfo.def_readwrite("tracerSpeed",&BulletInfo::tracerSpeed);
	bulletInfo.def_readwrite("tracerMaterial",&BulletInfo::tracerMaterial);
	bulletInfo.def_readwrite("tracerBloom",&BulletInfo::tracerBloom);
	bulletInfo.def_readwrite("ammoType",&BulletInfo::ammoType);
	bulletInfo.def_readwrite("direction",&BulletInfo::direction);
	bulletInfo.def_readwrite("effectOrigin",&BulletInfo::effectOrigin);
	bulletInfo.def_readwrite("damage",&BulletInfo::damage);
	gameMod[bulletInfo];

	auto classDefDamageInfo = luabind::class_<DamageInfo>("DamageInfo");
	classDefDamageInfo.def(luabind::constructor<>());
	classDefDamageInfo.def("SetDamage",&Lua::DamageInfo::SetDamage);
	classDefDamageInfo.def("AddDamage",&Lua::DamageInfo::AddDamage);
	classDefDamageInfo.def("ScaleDamage",&Lua::DamageInfo::ScaleDamage);
	classDefDamageInfo.def("GetDamage",&Lua::DamageInfo::GetDamage);
	classDefDamageInfo.def("GetAttacker",&Lua::DamageInfo::GetAttacker);
	classDefDamageInfo.def("SetAttacker",&Lua::DamageInfo::SetAttacker);
	classDefDamageInfo.def("GetInflictor",&Lua::DamageInfo::GetInflictor);
	classDefDamageInfo.def("SetInflictor",&Lua::DamageInfo::SetInflictor);
	classDefDamageInfo.def("GetDamageTypes",&Lua::DamageInfo::GetDamageTypes);
	classDefDamageInfo.def("SetDamageType",&Lua::DamageInfo::SetDamageType);
	classDefDamageInfo.def("AddDamageType",&Lua::DamageInfo::AddDamageType);
	classDefDamageInfo.def("RemoveDamageType",&Lua::DamageInfo::RemoveDamageType);
	classDefDamageInfo.def("IsDamageType",&Lua::DamageInfo::IsDamageType);
	classDefDamageInfo.def("SetSource",&Lua::DamageInfo::SetSource);
	classDefDamageInfo.def("GetSource",&Lua::DamageInfo::GetSource);
	classDefDamageInfo.def("SetHitPosition",&Lua::DamageInfo::SetHitPosition);
	classDefDamageInfo.def("GetHitPosition",&Lua::DamageInfo::GetHitPosition);
	classDefDamageInfo.def("SetForce",&Lua::DamageInfo::SetForce);
	classDefDamageInfo.def("GetForce",&Lua::DamageInfo::GetForce);
	classDefDamageInfo.def("GetHitGroup",&Lua::DamageInfo::GetHitGroup);
	classDefDamageInfo.def("SetHitGroup",&Lua::DamageInfo::SetHitGroup);
	gameMod[classDefDamageInfo];
}

#define LUA_MATRIX_MEMBERS_CLASSDEF(defMat,type) \
	defMat.def(luabind::constructor<>()); \
	defMat.def(luabind::constructor<float>()); \
	defMat.def(luabind::const_self /float()); \
	defMat.def(luabind::const_self *float()); \
	defMat.def(float() /luabind::const_self); \
	defMat.def(float() *luabind::const_self); \
	defMat.def(luabind::tostring(luabind::self)); \
	defMat.def(luabind::constructor<Mat2>()); \
	defMat.def(luabind::constructor<Mat2x3>()); \
	defMat.def(luabind::constructor<Mat2x4>()); \
	defMat.def(luabind::constructor<Mat3>()); \
	defMat.def(luabind::constructor<Mat3x2>()); \
	defMat.def(luabind::constructor<Mat3x4>()); \
	defMat.def(luabind::constructor<Mat4>()); \
	defMat.def(luabind::constructor<Mat4x2>()); \
	defMat.def(luabind::constructor<Mat4x3>()); \
	defMat.def("Copy",&Lua::Mat##type::Copy); \
	defMat.def("Get",&Lua::Mat##type::Get); \
	defMat.def("Set",static_cast<void(*)(lua_State*,Mat##type&,int,int,float)>(&Lua::Mat##type::Set)); \
	defMat.def("Transpose",&Lua::Mat##type::Transpose); \
	defMat.def("GetTranspose",&Lua::Mat##type::GetTransposition);

static void RegisterLuaMatrices(Lua::Interface &lua)
{
	auto &modMath = lua.RegisterLibrary("math");
	auto defMat2 = luabind::class_<Mat2>("Mat2");
	defMat2.def(luabind::constructor<float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2,2);
	defMat2.def(luabind::const_self ==luabind::const_self);
	defMat2.def(luabind::const_self +luabind::const_self);
	defMat2.def(luabind::const_self -luabind::const_self);
	defMat2.def(luabind::const_self *luabind::const_self);
	defMat2.def(luabind::const_self *Vector2());
	defMat2.def("Inverse",&Lua::Mat2::Inverse);
	defMat2.def("GetInverse",&Lua::Mat2::GetInverse);
	defMat2.def("Set",static_cast<void(*)(lua_State*,::Mat2&,float,float,float,float)>(&Lua::Mat2::Set));
	defMat2.def("Set",static_cast<void(*)(lua_State*,::Mat2&,const ::Mat2&)>(&Lua::Mat2::Set));
	modMath[defMat2];
	
	auto defMat2x3 = luabind::class_<Mat2x3>("Mat2x3");
	defMat2x3.def(luabind::constructor<float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x3,2x3);
	defMat2x3.def(luabind::const_self ==luabind::const_self);
	defMat2x3.def(luabind::const_self +luabind::const_self);
	defMat2x3.def(luabind::const_self -luabind::const_self);
	defMat2x3.def(luabind::const_self *Vector2());
	defMat2x3.def("Set",static_cast<void(*)(lua_State*,::Mat2x3&,float,float,float,float,float,float)>(&Lua::Mat2x3::Set));
	defMat2x3.def("Set",static_cast<void(*)(lua_State*,::Mat2x3&,const ::Mat2x3&)>(&Lua::Mat2x3::Set));
	modMath[defMat2x3];

	auto defMat2x4 = luabind::class_<Mat2x4>("Mat2x4");
	defMat2x4.def(luabind::constructor<float,float,float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat2x4,2x4);
	defMat2x4.def(luabind::const_self ==luabind::const_self);
	defMat2x4.def(luabind::const_self +luabind::const_self);
	defMat2x4.def(luabind::const_self -luabind::const_self);
	defMat2x4.def(luabind::const_self *Vector2());
	defMat2x4.def("Set",static_cast<void(*)(lua_State*,::Mat2x4&,float,float,float,float,float,float,float,float)>(&Lua::Mat2x4::Set));
	defMat2x4.def("Set",static_cast<void(*)(lua_State*,::Mat2x4&,const ::Mat2x4&)>(&Lua::Mat2x4::Set));
	modMath[defMat2x4];

	auto defMat3 = luabind::class_<Mat3>("Mat3");
	defMat3.def(luabind::constructor<float,float,float,float,float,float,float,float,float>());
	defMat3.def(luabind::constructor<Quat>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3,3);
	defMat3.def(luabind::const_self ==luabind::const_self);
	defMat3.def(luabind::const_self +luabind::const_self);
	defMat3.def(luabind::const_self -luabind::const_self);
	defMat3.def(luabind::const_self *luabind::const_self);
	defMat3.def(luabind::const_self *Vector3());
	defMat3.def("Inverse",&Lua::Mat3::Inverse);
	defMat3.def("GetInverse",&Lua::Mat3::GetInverse);
	defMat3.def("Set",static_cast<void(*)(lua_State*,::Mat3&,float,float,float,float,float,float,float,float,float)>(&Lua::Mat3::Set));
	defMat3.def("Set",static_cast<void(*)(lua_State*,::Mat3&,const ::Mat3&)>(&Lua::Mat3::Set));
	defMat3.def("CalcEigenValues",&Lua::Mat3::CalcEigenValues);
	modMath[defMat3];

	auto defMat3x2 = luabind::class_<Mat3x2>("Mat3x2");
	defMat3x2.def(luabind::constructor<float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3x2,3x2);
	defMat3x2.def(luabind::const_self ==luabind::const_self);
	defMat3x2.def(luabind::const_self +luabind::const_self);
	defMat3x2.def(luabind::const_self -luabind::const_self);
	defMat3x2.def(luabind::const_self *Vector3());
	defMat3x2.def("Set",static_cast<void(*)(lua_State*,::Mat3x2&,float,float,float,float,float,float)>(&Lua::Mat3x2::Set));
	defMat3x2.def("Set",static_cast<void(*)(lua_State*,::Mat3x2&,const ::Mat3x2&)>(&Lua::Mat3x2::Set));
	modMath[defMat3x2];

	auto defMat3x4 = luabind::class_<Mat3x4>("Mat3x4");
	defMat3x4.def(luabind::constructor<float,float,float,float,float,float,float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat3x4,3x4);
	defMat3x4.def(luabind::const_self ==luabind::const_self);
	defMat3x4.def(luabind::const_self +luabind::const_self);
	defMat3x4.def(luabind::const_self -luabind::const_self);
	defMat3x4.def(luabind::const_self *Vector3());
	defMat3x4.def("Set",static_cast<void(*)(lua_State*,::Mat3x4&,float,float,float,float,float,float,float,float,float,float,float,float)>(&Lua::Mat3x4::Set));
	defMat3x4.def("Set",static_cast<void(*)(lua_State*,::Mat3x4&,const ::Mat3x4&)>(&Lua::Mat3x4::Set));
	modMath[defMat3x4];

	auto defMat4 = luabind::class_<Mat4>("Mat4");
	defMat4.def(luabind::constructor<float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4,4);
	defMat4.def("Translate",&Lua::Mat4::Translate);
	defMat4.def("Rotate",static_cast<void(*)(lua_State*,::Mat4&,const EulerAngles&)>(&Lua::Mat4::Rotate));
	defMat4.def("Rotate",static_cast<void(*)(lua_State*,::Mat4&,const Vector3&,float)>(&Lua::Mat4::Rotate));
	defMat4.def("Scale",&Lua::Mat4::Scale);
	defMat4.def("ToEulerAngles",&Lua::Mat4::ToEulerAngles);
	defMat4.def("ToQuaternion",&Lua::Mat4::ToQuaternion);
	defMat4.def("Decompose",&Lua::Mat4::Decompose);
	defMat4.def("MulRow",static_cast<void(*)(lua_State*,Mat4&,uint32_t,float)>([](lua_State *l,Mat4 &m,uint32_t rowIndex,float factor) {
		for(uint8_t i=0;i<4;++i)
			m[rowIndex][i] *= factor;
	}));
	defMat4.def("MulCol",static_cast<void(*)(lua_State*,Mat4&,uint32_t,float)>([](lua_State *l,Mat4 &m,uint32_t colIndex,float factor) {
		for(uint8_t i=0;i<4;++i)
			m[i][colIndex] *= factor;
	}));
	defMat4.def("SwapRows",static_cast<void(*)(lua_State*,Mat4&,uint32_t,uint32_t)>([](lua_State *l,Mat4 &m,uint32_t rowSrc,uint32_t rowDst) {
		std::array<float,4> tmpRow = {m[rowSrc][0],m[rowSrc][1],m[rowSrc][2],m[rowSrc][3]};
		for(uint8_t i=0;i<4;++i)
		{
			m[rowSrc][i] = m[rowDst][i];
			m[rowDst][i] = tmpRow.at(i);
		}
	}));
	defMat4.def("SwapColumns",static_cast<void(*)(lua_State*,Mat4&,uint32_t,uint32_t)>([](lua_State *l,Mat4 &m,uint32_t colSrc,uint32_t colDst) {
		std::array<float,4> tmpCol = {m[0][colSrc],m[1][colSrc],m[2][colSrc],m[3][colSrc]};
		for(uint8_t i=0;i<4;++i)
		{
			m[i][colSrc] = m[i][colDst];
			m[i][colDst] = tmpCol.at(i);
		}
	}));
	defMat4.def(luabind::const_self ==luabind::const_self);
	defMat4.def(luabind::const_self +luabind::const_self);
	defMat4.def(luabind::const_self -luabind::const_self);
	defMat4.def(luabind::const_self *luabind::const_self);
	defMat4.def(luabind::const_self *Vector4());
	defMat4.def("Inverse",&Lua::Mat4::Inverse);
	defMat4.def("GetInverse",&Lua::Mat4::GetInverse);
	defMat4.def("Set",static_cast<void(*)(lua_State*,::Mat4&,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float)>(&Lua::Mat4::Set));
	defMat4.def("Set",static_cast<void(*)(lua_State*,::Mat4&,const ::Mat4&)>(&Lua::Mat4::Set));
	modMath[defMat4];
	auto _G = luabind::globals(lua.GetState());
	_G["Mat4"] = _G["math"]["Mat4"];

	auto defMat4x2 = luabind::class_<Mat4x2>("Mat4x2");
	defMat4x2.def(luabind::constructor<float,float,float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4x2,4x2);
	defMat4x2.def(luabind::const_self ==luabind::const_self);
	defMat4x2.def(luabind::const_self +luabind::const_self);
	defMat4x2.def(luabind::const_self -luabind::const_self);
	defMat4x2.def(luabind::const_self *Vector4());
	defMat4x2.def("Set",static_cast<void(*)(lua_State*,::Mat4x2&,float,float,float,float,float,float,float,float)>(&Lua::Mat4x2::Set));
	defMat4x2.def("Set",static_cast<void(*)(lua_State*,::Mat4x2&,const ::Mat4x2&)>(&Lua::Mat4x2::Set));
	modMath[defMat4x2];

	auto defMat4x3 = luabind::class_<Mat4x3>("Mat4x3");
	defMat4x3.def(luabind::constructor<float,float,float,float,float,float,float,float,float,float,float,float>());
	LUA_MATRIX_MEMBERS_CLASSDEF(defMat4x3,4x3);
	defMat4x3.def(luabind::const_self ==luabind::const_self);
	defMat4x3.def(luabind::const_self +luabind::const_self);
	defMat4x3.def(luabind::const_self -luabind::const_self);
	defMat4x3.def(luabind::const_self *Vector4());
	defMat4x3.def("Set",static_cast<void(*)(lua_State*,::Mat4x3&,float,float,float,float,float,float,float,float,float,float,float,float)>(&Lua::Mat4x3::Set));
	defMat4x3.def("Set",static_cast<void(*)(lua_State*,::Mat4x3&,const ::Mat4x3&)>(&Lua::Mat4x3::Set));
	modMath[defMat4x3];
}
