// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.world_environment;
namespace Lua {
	namespace WorldEnvironment {
		static void SetAmbientColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, const ::Vector4 &ambientColor);
		static void GetAmbientColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);

		static void SetShaderQuality(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, int32_t shaderQuality);
		static void GetShaderQuality(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);

		static void SetUnlit(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, bool bUnlit);
		static void IsUnlit(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);

		static void SetShadowResolution(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, uint32_t shadowResolution);
		static void GetShadowResolution(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);

		// Fog
		static void SetFogStart(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float start);
		static void SetFogEnd(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float end);
		static void GetFogStart(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void GetFogEnd(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void SetFogColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, const ::Color &color);
		static void GetFogColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void SetFogEnabled(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, bool bEnabled);
		static void IsFogEnabled(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void SetFogDensity(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float density);
		static void GetFogDensity(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void SetFogType(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, pragma::util::FogType type);
		static void GetFogType(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
		static void GetFogFarDistance(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv);
	};
};

void Lua::WorldEnvironment::register_class(luabind::class_<pragma::rendering::WorldEnvironment> &classDef)
{
	classDef.def("SetShaderQuality", &pragma::rendering::WorldEnvironment::SetShaderQuality);
	classDef.def("GetShaderQuality", &pragma::rendering::WorldEnvironment::GetShaderQuality);
	classDef.def("SetUnlit", &pragma::rendering::WorldEnvironment::SetUnlit);
	classDef.def("IsUnlit", &pragma::rendering::WorldEnvironment::IsUnlit);
	classDef.def("SetShadowResolution", &pragma::rendering::WorldEnvironment::SetShadowResolution);
	classDef.def("GetShadowResolution", &pragma::rendering::WorldEnvironment::GetShadowResolution);

	classDef.def("SetFogStart", &SetFogStart);
	classDef.def("SetFogEnd", &SetFogEnd);
	classDef.def("GetFogStart", &GetFogStart);
	classDef.def("GetFogEnd", &GetFogEnd);
	classDef.def("SetFogColor", &SetFogColor);
	classDef.def("GetFogColor", &GetFogColor);
	classDef.def("SetFogEnabled", &SetFogEnabled);
	classDef.def("IsFogEnabled", &IsFogEnabled);
	classDef.def("SetFogDensity", &SetFogDensity);
	classDef.def("GetFogDensity", &GetFogDensity);
	classDef.def("SetFogType", &SetFogType);
	classDef.def("GetFogType", &GetFogType);
	classDef.def("GetFogFarDistance", &GetFogFarDistance);

	classDef.def("GetShaderQualityProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetShaderQualityProperty()); }));
	classDef.def("GetUnlitProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetUnlitProperty()); }));
	classDef.def("GetShadowResolutionProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetShadowResolutionProperty()); }));
	classDef.def("GetFogColorProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetColorProperty()); }));
	classDef.def("GetFogStartProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetStartProperty()); }));
	classDef.def("GetFogEndProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetEndProperty()); }));
	classDef.def("GetFogMaxDensityProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetMaxDensityProperty()); }));
	classDef.def("GetFogTypeProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetTypeProperty()); }));
	classDef.def("GetFogEnabledProperty", static_cast<void (*)(lua::State *, std::shared_ptr<pragma::rendering::WorldEnvironment> &)>([](lua::State *l, std::shared_ptr<pragma::rendering::WorldEnvironment> &pEnv) { Property::push(l, *pEnv->GetFogSettings().GetEnabledProperty()); }));

	classDef.add_static_constant("FOG_TYPE_LINEAR", pragma::math::to_integral(pragma::util::FogType::Linear));
	classDef.add_static_constant("FOG_TYPE_EXPONENTIAL", pragma::math::to_integral(pragma::util::FogType::Exponential));
	classDef.add_static_constant("FOG_TYPE_EXPONENTIAL2", pragma::math::to_integral(pragma::util::FogType::Exponential2));
}

////////////////////////////////

void Lua::WorldEnvironment::SetFogStart(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float start)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetStart(start);
}
void Lua::WorldEnvironment::SetFogEnd(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float end)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetEnd(end);
}
void Lua::WorldEnvironment::GetFogStart(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushNumber(l, fog.GetStart());
}
void Lua::WorldEnvironment::GetFogEnd(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushNumber(l, fog.GetEnd());
}
void Lua::WorldEnvironment::SetFogColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, const ::Color &color)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetColor(color);
}
void Lua::WorldEnvironment::GetFogColor(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	Lua::Push<::Color>(l, fog.GetColor());
}
void Lua::WorldEnvironment::SetFogEnabled(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, bool bEnabled)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetEnabled(bEnabled);
}
void Lua::WorldEnvironment::IsFogEnabled(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushBool(l, fog.IsEnabled());
}
void Lua::WorldEnvironment::SetFogDensity(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, float density)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetMaxDensity(density);
}
void Lua::WorldEnvironment::GetFogDensity(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushNumber(l, fog.GetMaxDensity());
}
void Lua::WorldEnvironment::SetFogType(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv, pragma::util::FogType type)
{
	auto &fog = worldEnv.GetFogSettings();
	fog.SetType(type);
}
void Lua::WorldEnvironment::GetFogType(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushInt(l, pragma::math::to_integral(fog.GetType()));
}
void Lua::WorldEnvironment::GetFogFarDistance(lua::State *l, pragma::rendering::WorldEnvironment &worldEnv)
{
	auto &fog = worldEnv.GetFogSettings();
	PushNumber(l, fog.GetFarDistance());
}
