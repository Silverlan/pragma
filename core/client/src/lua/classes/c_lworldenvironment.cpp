#include "stdafx_client.h"
#include "pragma/lua/classes/c_lworldenvironment.hpp"
#include "luasystem.h"
#include <pragma/lua/classes/lproperty.hpp>

namespace Lua
{
	namespace WorldEnvironment
	{
		static void SetAmbientColor(lua_State *l,::WorldEnvironment &worldEnv,const Vector4 &ambientColor);
		static void GetAmbientColor(lua_State *l,::WorldEnvironment &worldEnv);

		static void SetShaderQuality(lua_State *l,::WorldEnvironment &worldEnv,int32_t shaderQuality);
		static void GetShaderQuality(lua_State *l,::WorldEnvironment &worldEnv);

		static void SetUnlit(lua_State *l,::WorldEnvironment &worldEnv,bool bUnlit);
		static void IsUnlit(lua_State *l,::WorldEnvironment &worldEnv);

		static void SetShadowResolution(lua_State *l,::WorldEnvironment &worldEnv,uint32_t shadowResolution);
		static void GetShadowResolution(lua_State *l,::WorldEnvironment &worldEnv);

		// Fog
		static void SetFogStart(lua_State *l,::WorldEnvironment &worldEnv,float start);
		static void SetFogEnd(lua_State *l,::WorldEnvironment &worldEnv,float end);
		static void GetFogStart(lua_State *l,::WorldEnvironment &worldEnv);
		static void GetFogEnd(lua_State *l,::WorldEnvironment &worldEnv);
		static void SetFogColor(lua_State *l,::WorldEnvironment &worldEnv,const Color &color);
		static void GetFogColor(lua_State *l,::WorldEnvironment &worldEnv);
		static void SetFogEnabled(lua_State *l,::WorldEnvironment &worldEnv,bool bEnabled);
		static void IsFogEnabled(lua_State *l,::WorldEnvironment &worldEnv);
		static void SetFogDensity(lua_State *l,::WorldEnvironment &worldEnv,float density);
		static void GetFogDensity(lua_State *l,::WorldEnvironment &worldEnv);
		static void SetFogType(lua_State *l,::WorldEnvironment &worldEnv,uint32_t type);
		static void GetFogType(lua_State *l,::WorldEnvironment &worldEnv);
		static void GetFogFarDistance(lua_State *l,::WorldEnvironment &worldEnv);
	};
};

void Lua::WorldEnvironment::register_class(luabind::class_<::WorldEnvironment> &classDef)
{
	classDef.def("SetAmbientColor",&SetAmbientColor);
	classDef.def("GetAmbientColor",&GetAmbientColor);
	classDef.def("SetShaderQuality",&SetShaderQuality);
	classDef.def("GetShaderQuality",&GetShaderQuality);
	classDef.def("SetUnlit",&SetUnlit);
	classDef.def("IsUnlit",&IsUnlit);
	classDef.def("SetShadowResolution",&SetShadowResolution);
	classDef.def("GetShadowResolution",&GetShadowResolution);

	classDef.def("SetFogStart",&SetFogStart);
	classDef.def("SetFogEnd",&SetFogEnd);
	classDef.def("GetFogStart",&GetFogStart);
	classDef.def("GetFogEnd",&GetFogEnd);
	classDef.def("SetFogColor",&SetFogColor);
	classDef.def("GetFogColor",&GetFogColor);
	classDef.def("SetFogEnabled",&SetFogEnabled);
	classDef.def("IsFogEnabled",&IsFogEnabled);
	classDef.def("SetFogDensity",&SetFogDensity);
	classDef.def("GetFogDensity",&GetFogDensity);
	classDef.def("SetFogType",&SetFogType);
	classDef.def("GetFogType",&GetFogType);
	classDef.def("GetFogFarDistance",&GetFogFarDistance);

	classDef.def("GetAmbientColorProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetAmbientColorProperty());
	}));
	classDef.def("GetShaderQualityProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetShaderQualityProperty());
	}));
	classDef.def("GetUnlitProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetUnlitProperty());
	}));
	classDef.def("GetShadowResolutionProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetShadowResolutionProperty());
	}));
	classDef.def("GetFogColorProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetColorProperty());
	}));
	classDef.def("GetFogStartProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetStartProperty());
	}));
	classDef.def("GetFogEndProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetEndProperty());
	}));
	classDef.def("GetFogMaxDensityProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetMaxDensityProperty());
	}));
	classDef.def("GetFogTypeProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetTypeProperty());
	}));
	classDef.def("GetFogEnabledProperty",static_cast<void(*)(lua_State*,std::shared_ptr<::WorldEnvironment>&)>([](lua_State *l,std::shared_ptr<::WorldEnvironment> &pEnv) {
		Lua::Property::push(l,*pEnv->GetFogSettings().GetEnabledProperty());
	}));

	classDef.add_static_constant("FOG_TYPE_LINEAR",umath::to_integral(::WorldEnvironment::Fog::Type::Linear));
	classDef.add_static_constant("FOG_TYPE_EXPONENTIAL",umath::to_integral(::WorldEnvironment::Fog::Type::Exponential));
	classDef.add_static_constant("FOG_TYPE_EXPONENTIAL2",umath::to_integral(::WorldEnvironment::Fog::Type::Exponential2));
}

void Lua::WorldEnvironment::SetAmbientColor(lua_State *l,::WorldEnvironment &worldEnv,const Vector4 &ambientColor)
{
	worldEnv.SetAmbientColor(ambientColor);
}
void Lua::WorldEnvironment::GetAmbientColor(lua_State *l,::WorldEnvironment &worldEnv)
{
	Lua::Push<Vector4>(l,worldEnv.GetAmbientColor());
}

void Lua::WorldEnvironment::SetShaderQuality(lua_State *l,::WorldEnvironment &worldEnv,int32_t shaderQuality)
{
	worldEnv.SetShaderQuality(shaderQuality);
}
void Lua::WorldEnvironment::GetShaderQuality(lua_State *l,::WorldEnvironment &worldEnv)
{
	Lua::PushInt(l,worldEnv.GetShaderQuality());
}

void Lua::WorldEnvironment::SetUnlit(lua_State *l,::WorldEnvironment &worldEnv,bool bUnlit)
{
	worldEnv.SetUnlit(bUnlit);
}
void Lua::WorldEnvironment::IsUnlit(lua_State *l,::WorldEnvironment &worldEnv)
{
	Lua::PushBool(l,worldEnv.IsUnlit());
}

void Lua::WorldEnvironment::SetShadowResolution(lua_State *l,::WorldEnvironment &worldEnv,uint32_t shadowResolution)
{
	worldEnv.SetShadowResolution(shadowResolution);
}
void Lua::WorldEnvironment::GetShadowResolution(lua_State *l,::WorldEnvironment &worldEnv)
{
	Lua::PushInt(l,worldEnv.GetShadowResolution());
}

////////////////////////////////

void Lua::WorldEnvironment::SetFogStart(lua_State *l,::WorldEnvironment &worldEnv,float start)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetStart(start);
}
void Lua::WorldEnvironment::SetFogEnd(lua_State *l,::WorldEnvironment &worldEnv,float end)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetEnd(end);
}
void Lua::WorldEnvironment::GetFogStart(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushNumber(l,fog.GetStart());
}
void Lua::WorldEnvironment::GetFogEnd(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushNumber(l,fog.GetEnd());
}
void Lua::WorldEnvironment::SetFogColor(lua_State *l,::WorldEnvironment &worldEnv,const Color &color)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetColor(color);
}
void Lua::WorldEnvironment::GetFogColor(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::Push<Color>(l,fog.GetColor());
}
void Lua::WorldEnvironment::SetFogEnabled(lua_State *l,::WorldEnvironment &worldEnv,bool bEnabled)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetEnabled(bEnabled);
}
void Lua::WorldEnvironment::IsFogEnabled(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushBool(l,fog.IsEnabled());
}
void Lua::WorldEnvironment::SetFogDensity(lua_State *l,::WorldEnvironment &worldEnv,float density)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetMaxDensity(density);
}
void Lua::WorldEnvironment::GetFogDensity(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushNumber(l,fog.GetMaxDensity());
}
void Lua::WorldEnvironment::SetFogType(lua_State *l,::WorldEnvironment &worldEnv,uint32_t type)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetType(static_cast<::WorldEnvironment::Fog::Type>(type));
}
void Lua::WorldEnvironment::GetFogType(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushInt(l,umath::to_integral(fog.GetType()));
}
void Lua::WorldEnvironment::GetFogFarDistance(lua_State *l,::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::PushNumber(l,fog.GetFarDistance());
}
