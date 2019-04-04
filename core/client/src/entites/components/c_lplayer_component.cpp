#include "stdafx_client.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_lentity_components.hpp"
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

namespace Lua
{
	namespace Player
	{
		namespace Client
		{
			static void GetViewOffset(lua_State *l,CPlayerHandle &hPl);
			static void SetViewOffset(lua_State *l,CPlayerHandle &hPl,const Vector3 &offset);
			static void IsInFirstPersonMode(lua_State *l,CPlayerHandle &hPl);
		};
	};
};
void Lua::register_cl_player_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<CPlayerHandle,BaseEntityComponentHandle>("PlayerComponent");
	Lua::register_base_player_component_methods<luabind::class_<CPlayerHandle,BaseEntityComponentHandle>,CPlayerHandle>(l,def);
	def.def("SetViewOffset",&Lua::Player::Client::SetViewOffset);
	def.def("GetViewOffset",&Lua::Player::Client::GetViewOffset);
	def.def("IsInFirstPersonMode",&Lua::Player::Client::IsInFirstPersonMode);
	module[def];
}
void Lua::Player::Client::GetViewOffset(lua_State *l,CPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hPl);
	Lua::Push<Vector3>(l,hPl->GetViewOffset());
}

void Lua::Player::Client::SetViewOffset(lua_State *l,CPlayerHandle &hPl,const Vector3 &offset)
{
	pragma::Lua::check_component(l,hPl);
	hPl->SetViewOffset(offset);
}

void Lua::Player::Client::IsInFirstPersonMode(lua_State *l,CPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hPl);
	Lua::PushBool(l,hPl->IsInFirstPersonMode());
}
