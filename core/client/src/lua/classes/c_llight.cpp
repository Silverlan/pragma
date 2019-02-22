#include "stdafx_client.h"
// TODO: Remove this file
#if 0
#include "c_llight.hpp"
#include "luasystem.h"
#include "pragma/lua/classes/c_ldef_light.hpp"

namespace Lua
{
	namespace Light
	{
		static void IsValid(lua_State *l,CLightObjectHandle &hLight);
		static void Remove(lua_State *l,CLightObjectHandle &hLight);

		static void TurnOn(lua_State *l,CLightObjectHandle &hLight);
		static void TurnOff(lua_State *l,CLightObjectHandle &hLight);
		static void IsTurnedOn(lua_State *l,CLightObjectHandle &hLight);
		static void Toggle(lua_State *l,CLightObjectHandle &hLight);

		static void SetPos(lua_State *l,CLightObjectHandle &hLight,const Vector3 &pos);
		static void GetPos(lua_State *l,CLightObjectHandle &hLight);

		static void SetColor(lua_State *l,CLightObjectHandle &hLight,const Color &col);
		static void GetColor(lua_State *l,CLightObjectHandle &hLight);

		static void SetShadowType(lua_State *l,CLightObjectHandle &hLight,uint32_t type);
		static void GetShadowType(lua_State *l,CLightObjectHandle &hLight);

		static void SetRadius(lua_State *l,CLightObjectHandle &hLight,float radius);
		static void GetRadius(lua_State *l,CLightObjectHandle &hLight);

		static void UpdateBuffers(lua_State *l,CLightObjectHandle &hLight);
	};
};

void Lua::Light::register_class(luabind::class_<CLightObjectHandle> &classDef)
{
	classDef.def("IsValid",&IsValid);
	classDef.def("Remove",&Remove);
	classDef.def("TurnOn",&TurnOn);
	classDef.def("TurnOff",&TurnOff);
	classDef.def("IsTurnedOn",&IsTurnedOn);
	classDef.def("Toggle",&Toggle);
	classDef.def("SetPos",&SetPos);
	classDef.def("GetPos",&GetPos);
	classDef.def("SetColor",&SetColor);
	classDef.def("GetColor",&GetColor);
	classDef.def("SetShadowType",&SetShadowType);
	classDef.def("GetShadowType",&GetShadowType);
	classDef.def("SetRadius",&SetRadius);
	classDef.def("GetRadius",&GetRadius);
	classDef.def("UpdateBuffers",&UpdateBuffers);

	classDef.add_static_constant("SHADOW_TYPE_NONE",umath::to_integral(ShadowType::None));
	classDef.add_static_constant("SHADOW_TYPE_STATIC_ONLY",umath::to_integral(ShadowType::StaticOnly));
	classDef.add_static_constant("SHADOW_TYPE_FULL",umath::to_integral(ShadowType::Full));
}

void Lua::Light::IsValid(lua_State *l,CLightObjectHandle &hLight)
{
	Lua::PushBool(l,hLight.IsValid());
}

void Lua::Light::Remove(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->Remove();
}
void Lua::Light::TurnOn(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->TurnOn();
}
void Lua::Light::TurnOff(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->TurnOff();
}
void Lua::Light::IsTurnedOn(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	Lua::PushBool(l,hLight->IsTurnedOn());
}
void Lua::Light::Toggle(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->Toggle();
}
void Lua::Light::SetPos(lua_State *l,CLightObjectHandle &hLight,const Vector3 &pos)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->SetPosition(pos);
}
void Lua::Light::GetPos(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	Lua::Push<Vector3>(l,hLight->GetPosition());
}
void Lua::Light::SetColor(lua_State *l,CLightObjectHandle &hLight,const Color &col)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->SetColor(col);
}
void Lua::Light::GetColor(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	Lua::Push<Color>(l,hLight->GetColor());
}
void Lua::Light::SetShadowType(lua_State *l,CLightObjectHandle &hLight,uint32_t type)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->SetShadowType(static_cast<ShadowType>(type));
}
void Lua::Light::GetShadowType(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	Lua::PushInt(l,umath::to_integral(hLight->GetShadowType()));
}
void Lua::Light::SetRadius(lua_State *l,CLightObjectHandle &hLight,float radius)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->SetDistance(radius);
}
void Lua::Light::GetRadius(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	Lua::PushNumber(l,hLight->GetDistance());
}
void Lua::Light::UpdateBuffers(lua_State *l,CLightObjectHandle &hLight)
{
	LUA_CHECK_LIGHT(l,hLight);
	hLight->UpdateBuffers();
}
#endif
