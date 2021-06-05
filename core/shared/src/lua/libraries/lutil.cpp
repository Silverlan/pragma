/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lutil.hpp"
#include "pragma/lua/util.hpp"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_damageinfo.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/class_manager.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/game/damageinfo.h"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/environment/env_quake.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/util/util_game.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/util/util_splash_damage_info.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/util/util_rgbcsv.hpp"
#include "pragma/util/util_variable_type.hpp"
#include <sharedutils/netpacket.hpp>
#include <sharedutils/util_file.h>
#include <luainterface.hpp>
#include <se_scene.hpp>
#include <pragma/math/intersection.h>
#include <luabind/class_info.hpp>
#include <util_zip.h>

extern DLLNETWORK Engine *engine;

static auto s_bIgnoreIncludeCache = false;
void Lua::set_ignore_include_cache(bool b) {s_bIgnoreIncludeCache = b;}

luabind::detail::class_rep *Lua::get_crep(luabind::object o)
{
	auto *L = o.interpreter();
	luabind::detail::class_rep *crep = nullptr;

	o.push(L);
	if(luabind::detail::is_class_rep(L,-1))
	{
		crep = static_cast<luabind::detail::class_rep *>(lua_touserdata(L, -1));
		lua_pop(L, 1);
	}
	else
	{
		auto *obj = luabind::detail::get_instance(L,-1);
		if(!obj)
			lua_pop(L,1);
		else
		{
			lua_pop(L,1);
			// OK, we were given an object - gotta get the crep.
			crep = obj->crep();
		}
	}
	return crep;
}

void Lua::util::register_shared_generic(luabind::module_ &mod)
{
	mod[
		luabind::def("is_valid",static_cast<bool(*)(lua_State*)>(Lua::util::is_valid)),
		luabind::def("is_valid",static_cast<bool(*)(lua_State*,const luabind::object&)>(Lua::util::is_valid)),
		luabind::def("remove",static_cast<void(*)(lua_State*,const luabind::object&)>(remove)),
		luabind::def("remove",static_cast<void(*)(lua_State*,const luabind::object&,bool)>(remove)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&,const luabind::object&,const luabind::object&,const luabind::object&,const luabind::object&,const luabind::object&)>(Lua::util::register_class)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&,const luabind::object&,const luabind::object&,const luabind::object&,const luabind::object&)>(Lua::util::register_class)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&,const luabind::object&,const luabind::object&,const luabind::object&)>(Lua::util::register_class)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&,const luabind::object&,const luabind::object&)>(Lua::util::register_class)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&,const luabind::object&)>(Lua::util::register_class)),
		luabind::def("register_class",static_cast<luabind::object(*)(lua_State*,const std::string&)>(Lua::util::register_class)),

		luabind::def("local_to_world",static_cast<Quat(*)(lua_State*,const Quat&,const Quat&)>(local_to_world)),
		luabind::def("local_to_world",static_cast<Vector3(*)(lua_State*,const Vector3&,const Quat&,const Vector3&)>(local_to_world)),
		luabind::def("local_to_world",static_cast<void(*)(lua_State*,const Vector3&,const Quat&,const Vector3&,const Quat&)>(local_to_world)),

		luabind::def("world_to_local",static_cast<Quat(*)(lua_State*,const Quat&,const Quat&)>(world_to_local)),
		luabind::def("world_to_local",static_cast<Vector3(*)(lua_State*,const Vector3&,const Quat&,const Vector3&)>(world_to_local)),
		luabind::def("world_to_local",static_cast<void(*)(lua_State*,const Vector3&,const Quat&,const Vector3&,const Quat&)>(world_to_local)),

		luabind::def("get_pretty_duration",static_cast<std::string(*)(lua_State*,uint32_t,uint32_t,bool)>(Lua::util::get_pretty_duration)),
		luabind::def("get_pretty_duration",static_cast<std::string(*)(lua_State*,uint32_t,uint32_t)>(Lua::util::get_pretty_duration)),
		luabind::def("get_pretty_duration",static_cast<std::string(*)(lua_State*,uint32_t)>(Lua::util::get_pretty_duration)),
		luabind::def("get_pretty_time",Lua::util::get_pretty_time),

		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LColorProperty&,const Color&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LVector2iProperty&,const Vector2i&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LVector3Property&,const Vector3&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LVector3iProperty&,const Vector3i&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LVector4Property&,const Vector4&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LVector4iProperty&,const Vector4i&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LQuatProperty&,const Quat&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LEulerAnglesProperty&,const EulerAngles&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LGenericIntPropertyWrapper&,const int64_t&,float)>(Lua::util::fade_property)),
		luabind::def("fade_property",static_cast<luabind::object(*)(lua_State*,LGenericFloatPropertyWrapper&,const double&,float)>(Lua::util::fade_property)),

		luabind::def("round_string",static_cast<std::string(*)(lua_State*,float,uint32_t)>(Lua::util::round_string)),
		luabind::def("round_string",static_cast<std::string(*)(lua_State*,float)>(Lua::util::round_string)),

		luabind::def("get_type_name",Lua::util::get_type_name),
		luabind::def("is_same_object",Lua::util::is_same_object),
		luabind::def("clamp_resolution_to_aspect_ratio",Lua::util::clamp_resolution_to_aspect_ratio),
		luabind::def("get_class_value",Lua::util::get_class_value),
		luabind::def("pack_zip_archive",Lua::util::pack_zip_archive),
		luabind::def("world_space_point_to_screen_space_uv",static_cast<void(*)(lua_State*,const Vector3&,const Mat4&,float,float)>(Lua::util::world_space_point_to_screen_space_uv)),
		luabind::def("world_space_direction_to_screen_space",Lua::util::world_space_direction_to_screen_space),
		luabind::def("calc_screen_space_distance_to_world_space_position",Lua::util::calc_screenspace_distance_to_worldspace_position),
		luabind::def("depth_to_distance",Lua::util::depth_to_distance)
	];
}
void Lua::util::register_shared(luabind::module_ &mod)
{
	register_shared_generic(mod);
	mod[
		luabind::def("is_valid_entity",static_cast<bool(*)(lua_State*)>(Lua::util::is_valid_entity)),
		luabind::def("is_valid_entity",static_cast<bool(*)(lua_State*,const luabind::object&)>(Lua::util::is_valid_entity)),
		luabind::def("shake_screen",static_cast<void(*)(lua_State*,const Vector3&,float,float,float,float,float,float)>(Lua::util::shake_screen)),
		luabind::def("shake_screen",static_cast<void(*)(lua_State*,float,float,float,float,float)>(Lua::util::shake_screen)),
		luabind::def("read_scene_file",Lua::util::read_scene_file)
	];
}

void Lua::util::register_library(lua_State *l)
{
	auto utilMod = luabind::module(l,"util");
	utilMod[
		luabind::def("splash_damage",splash_damage),
		luabind::def("get_date_time",static_cast<std::string(*)(const std::string&)>(Lua::util::date_time)),
		luabind::def("get_date_time",static_cast<std::string(*)()>(Lua::util::date_time)),
		luabind::def("is_table",static_cast<bool(*)(luabind::argument)>(Lua::util::is_table)),
		luabind::def("is_table",static_cast<bool(*)()>(Lua::util::is_table)),
		luabind::def("get_faded_time_factor",static_cast<float(*)(float,float,float,float)>(Lua::util::get_faded_time_factor)),
		luabind::def("get_faded_time_factor",static_cast<float(*)(float,float,float)>(Lua::util::get_faded_time_factor)),
		luabind::def("get_scale_factor",static_cast<float(*)(float,float,float)>(Lua::util::get_scale_factor)),
		luabind::def("get_scale_factor",static_cast<float(*)(float,float)>(Lua::util::get_scale_factor)),
		luabind::def("open_path_in_explorer",static_cast<void(*)(const std::string&,const std::string&)>(Lua::util::open_path_in_explorer)),
		luabind::def("open_path_in_explorer",static_cast<void(*)(const std::string&)>(Lua::util::open_path_in_explorer)),
		luabind::def("get_pretty_bytes",Lua::util::get_pretty_bytes),
		luabind::def("units_to_metres",Lua::util::units_to_metres),
		luabind::def("metres_to_units",Lua::util::metres_to_units),
		luabind::def("variable_type_to_string",Lua::util::variable_type_to_string),
		luabind::def("open_url_in_browser",Lua::util::open_url_in_browser),
		luabind::def("get_addon_path",static_cast<std::string(*)(lua_State*)>(Lua::util::get_addon_path)),
		luabind::def("get_string_hash",Lua::util::get_string_hash),
		luabind::def("generate_uuid_v4",static_cast<std::string(*)()>([]() -> std::string {
			return ::util::uuid_to_string(::util::generate_uuid_v4());
		}))
	];
}

luabind::object Lua::global::include(lua_State *l,const std::string &f) {return include(l,f,s_bIgnoreIncludeCache);}
luabind::object Lua::global::include(lua_State *l,const std::string &f,bool ignoreCache) {return include(l,f,ignoreCache,false);}

luabind::object Lua::global::include(lua_State *l,const std::string &f,bool ignoreCache,bool reload)
{
	auto *lInterface = engine->GetLuaInterface(l);
	std::vector<std::string> *includeCache = (lInterface != nullptr) ? &lInterface->GetIncludeCache() : nullptr;
	auto fShouldInclude = [includeCache,ignoreCache,reload](std::string fpath) -> bool {
		if(includeCache == nullptr)
			return true;
		if(fpath.empty() == false)
		{
			if(fpath.front() == '/' || fpath.front() == '\\')
				fpath.erase(fpath.begin());
			else
				fpath = Lua::GetIncludePath(fpath);
		}
		fpath = FileManager::GetCanonicalizedPath(fpath);
		auto it = std::find_if(includeCache->begin(),includeCache->end(),[fpath](const std::string &other) {
			return ustring::compare(fpath,other,false);
		});
		if(!reload && it != includeCache->end())
			return ignoreCache;
		if(it == includeCache->end())
			includeCache->push_back(fpath);
		return true;
	};
	auto *nw = engine->GetNetworkState(l);
	auto *game = (nw != nullptr) ? nw->GetGameState() : nullptr;
	std::string ext;
	if(ufile::get_extension(f,&ext) == false) // Assume it's a directory
	{
		std::string relPath = f;
		if(relPath.back() != '\\' && relPath.back() != '/')
			relPath += "/";

		auto incPath = relPath;
		if(incPath.empty() == false)
		{
			if(incPath.front() == '/' || incPath.front() == '\\')
				incPath.erase(incPath.begin());
			else
				incPath = Lua::GetIncludePath(incPath);
		}
		incPath = "lua/" +incPath;
		auto incPathLua = incPath +"*.lua";
		std::vector<std::string> files;
		FileManager::FindFiles(incPathLua.c_str(),&files,nullptr);

		auto incPathCLua = incPath +"*.clua";
		std::vector<std::string> cfiles;
		FileManager::FindFiles(incPathCLua.c_str(),&cfiles,nullptr);
		files.reserve(files.size() +cfiles.size());

		// Add pre-compiled Lua-files to list, but make sure there are no duplicates!
		for(auto &cf : cfiles)
		{
			auto it = std::find_if(files.begin(),files.end(),[&cf](const std::string &fother) {
				return ustring::compare(cf.c_str(),fother.c_str(),false,cf.length() -5);
			});
			if(it != files.end())
			{
				*it = cf; // Prefer pre-compiled files over regular files
				continue;
			}
			files.push_back(cf);
		}

		for(auto &fName : files)
		{
			auto fpath = relPath +fName;
			if(fShouldInclude(fpath) == false)
				continue;
			auto r = Lua::IncludeFile(l,fpath,Lua::HandleTracebackError);
			switch(r)
			{
				case Lua::StatusCode::ErrorFile:
					lua_error(l);
					/* unreachable */
					break;
				case Lua::StatusCode::ErrorSyntax:
					Lua::HandleSyntaxError(l,r,fpath);
					break;
			}
		}
		return {};
	}
	if(fShouldInclude(f) == true)
	{
		//auto r = Lua::Execute(l,[&f,l](int(*traceback)(lua_State*)) {
		//	return Lua::IncludeFile(l,f,traceback);
		//});
		auto n = Lua::GetStackTop(l);
		auto fileName = f;
		auto r = Lua::IncludeFile(l,fileName,Lua::HandleTracebackError,LUA_MULTRET);
		switch(r)
		{
			case Lua::StatusCode::ErrorFile:
				lua_error(l);
				/* unreachable */
				break;
			case Lua::StatusCode::ErrorSyntax:
				Lua::HandleSyntaxError(l,r,fileName);
				break;
			case Lua::StatusCode::Ok:
				return luabind::object{luabind::from_stack{l,Lua::GetStackTop(l) -n}};
		}
	}
	return {};
}

luabind::object Lua::global::exec(lua_State *l,const std::string &f)
{
	auto n = Lua::GetStackTop(l);
	std::string fileName = f;
	auto r = Lua::ExecuteFile(l,fileName,Lua::HandleTracebackError,LUA_MULTRET);
	switch(r)
	{
		case Lua::StatusCode::ErrorFile:
			lua_error(l);
			/* unreachable */
			break;
		case Lua::StatusCode::ErrorSyntax:
			Lua::HandleSyntaxError(l,r,fileName);
			break;
		case Lua::StatusCode::Ok:
			return luabind::object{luabind::from_stack{l,Lua::GetStackTop(l) -n}};
	}
	return {};
}

std::string Lua::global::get_script_path() {return Lua::GetIncludePath();}
EulerAngles Lua::global::angle_rand() {return EulerAngles(umath::random(-180.f,180.f),umath::random(-180.f,180.f),umath::random(-180.f,180.f));}
EulerAngles Lua::global::create_from_string(const std::string &str) {return EulerAngles{str};}

static bool check_valid_lua_object(lua_State *l,const luabind::object &o)
{
	auto *pEnt = luabind::object_cast_nothrow<EntityHandle*>(o,static_cast<EntityHandle*>(nullptr));
	if(pEnt != nullptr)
		return pEnt->IsValid(); // Used frequently, and is faster than looking up "IsValid"
	auto bValid = true;
	try
	{
		auto oIsValid = o["IsValid"];
		if(!oIsValid)
			return bValid;
		bValid = luabind::call_member<bool>(o,"IsValid");
	}
	catch(std::exception&) // No "IsValid" method exists
	{}
	return bValid;
}

bool Lua::util::is_valid(lua_State *l) {return false;}
bool Lua::util::is_valid(lua_State *l,const luabind::object &o)
{
	if(!o)
		return false;
	auto type = luabind::type(o);
	switch(type)
	{
	case LUA_TUSERDATA:
		return check_valid_lua_object(l,o);
	case LUA_TBOOLEAN:
		return luabind::object_cast<bool>(o);
	case LUA_TTABLE:
	{
		for(luabind::iterator i{o}, e; i != e; ++i)
		{
			auto child = luabind::object{*i};
			if(check_valid_lua_object(l,child) == false)
				return false;
		}
		return true;
	}
	}
	return true;
}

bool Lua::util::is_valid_entity(lua_State *l)
{
	if(!Lua::IsSet(l,1) || !Lua::IsEntity(l,1))
		return false;
	return is_valid(l);
}

bool Lua::util::is_valid_entity(lua_State *l,const luabind::object &o)
{
	if(!o || !is_entity(o))
		return false;
	return is_valid(l,o);
}

static void safely_remove(const luabind::object &o,const char *removeFunction,bool useSafeMethod)
{
	auto *pEnt = luabind::object_cast_nothrow<EntityHandle*>(o,static_cast<EntityHandle*>(nullptr));
	if(pEnt != nullptr) // Used frequently, and is faster than looking up "IsValid"
	{
		if(pEnt->IsValid())
		{
			if(useSafeMethod)
				(*pEnt)->RemoveSafely();
			else
			{
				try
				{
					(*pEnt)->Remove();
				}
				catch(const std::exception &e)
				{
					Lua::Error(o.interpreter(),e.what());
				}
			}
		}
		return;
	}
	try
	{
		auto oRemove = o[removeFunction];
		if(!oRemove)
			return;
		luabind::call_member<void>(o,removeFunction);
	}
	catch(std::exception&) // No "IsValid" method exists
	{}
}

void Lua::util::remove(lua_State *l,const luabind::object &o,bool removeSafely)
{
	if(is_valid(l,o) == false)
		return;
	auto *removeFunction = removeSafely ? "RemoveSafely" : "Remove";
	if(luabind::type(o) == LUA_TTABLE)
	{
		for(luabind::iterator i(o), e; i != e; ++i)
		{
			auto o = luabind::object{*i};
			safely_remove(o,removeFunction,removeSafely);
		}
		return;
	}
	safely_remove(o,removeFunction,removeSafely);
}
void Lua::util::remove(lua_State *l,const luabind::object &o) {return remove(l,o,false);}

bool Lua::util::is_table(luabind::argument arg) {return luabind::type(arg) == LUA_TTABLE;}
bool Lua::util::is_table() {return false;}

std::string Lua::util::date_time(const std::string &format) {return engine->GetDate(format);}
std::string Lua::util::date_time() {return engine->GetDate();}

luabind::object Lua::util::fire_bullets(lua_State *l,BulletInfo &bulletInfo,bool hitReport,const std::function<void(DamageInfo&,::TraceData&,TraceResult&,uint32_t&)> &f)
{
	DamageInfo dmg;
	dmg.SetDamage(umath::min(CUInt16(bulletInfo.damage),CUInt16(std::numeric_limits<UInt16>::max())));
	dmg.SetDamageType(bulletInfo.damageType);
	dmg.SetSource(bulletInfo.effectOrigin);
	dmg.SetAttacker(bulletInfo.hAttacker.get());
	dmg.SetInflictor(bulletInfo.hInflictor.get());

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	auto &src = dmg.GetSource();
	luabind::object t {};
	if(hitReport == true)
		t = luabind::newtable(l);
	int32_t tIdx = 1;
	for(Int32 i=0;i<bulletInfo.bulletCount;i++)
	{
		auto randSpread = EulerAngles(umath::random(-bulletInfo.spread.p,bulletInfo.spread.p),umath::random(-bulletInfo.spread.y,bulletInfo.spread.y),0);
		auto bulletDir = bulletInfo.direction;
		uvec::rotate(&bulletDir,randSpread);
		::TraceData data;
		data.SetSource(src);
		data.SetTarget(src +bulletDir *bulletInfo.distance);
		data.SetCollisionFilterMask(CollisionMask::AllHitbox &~CollisionMask::Trigger); // Let everything pass (Except specific filters below)
		auto *attacker = dmg.GetAttacker();
		data.SetFilter([attacker](pragma::physics::IShape &shape,pragma::physics::IRigidBody &body) -> RayCastHitType {
			auto *phys = body.GetPhysObj();
			auto *ent = phys ? phys->GetOwner() : nullptr;
			if(ent == nullptr || &ent->GetEntity() == attacker) // Attacker can't shoot themselves
				return RayCastHitType::None;
			auto filterGroup = phys->GetCollisionFilter();
			auto mdlComponent = ent->GetEntity().GetModelComponent();
			if(mdlComponent && mdlComponent->GetHitboxCount() > 0 && (filterGroup &CollisionMask::NPC) != CollisionMask::None || (filterGroup &CollisionMask::Player) != CollisionMask::None) // Filter out player and NPC collision objects, since we only want to check their hitboxes
				return RayCastHitType::None;
			return RayCastHitType::Block;
		});
		auto filterGroup = CollisionMask::None;
		if(attacker != nullptr)
		{
			auto pPhysComponent = attacker->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				filterGroup = pPhysComponent->GetCollisionFilter();
			filterGroup |= CollisionMask::Water | CollisionMask::WaterSurface | CollisionMask::PlayerHitbox | CollisionMask::NPCHitbox;
		}
		else
			filterGroup = CollisionMask::AllHitbox;
		data.SetCollisionFilterGroup(filterGroup);
		std::vector<TraceResult> results {};
		if(game->RayCast(data,&results) && results.front().entity.IsValid())
		{
			auto &result = results.front();
			auto pDamageableComponent = result.entity->GetComponent<pragma::DamageableComponent>();
			if(pDamageableComponent.valid())
			{
				auto hitGroup = HitGroup::Generic;
				if(result.collisionObj.IsValid())
				{
					auto charComponent = result.entity->GetCharacterComponent();
					if(charComponent.valid())
						charComponent->FindHitgroup(*result.collisionObj,hitGroup);
				}
				dmg.SetHitGroup(hitGroup);
				dmg.SetForce(bulletDir *bulletInfo.force);
				dmg.SetHitPosition(result.position);
				pDamageableComponent->TakeDamage(dmg);
			}
		}
		if(hitReport == true)
		{
			if(results.empty())
				t[tIdx++] = TraceResult{data};
			else
			{
				for(auto &result : results)
					t[tIdx++] = result;
			}
		}
		if(f != nullptr)
		{
			if(results.empty())
			{
				TraceResult result {data};
				f(dmg,data,result,bulletInfo.tracerCount);
			}
			else
			{
				for(auto &result : results)
					f(dmg,data,result,bulletInfo.tracerCount);
			}
		}
	}
	return t;
}
luabind::object Lua::util::fire_bullets(lua_State *l,BulletInfo &bulletInfo,bool hitReport) {return fire_bullets(l,bulletInfo,hitReport,nullptr);}
luabind::object Lua::util::fire_bullets(lua_State *l,BulletInfo &bulletInfo) {return fire_bullets(l,bulletInfo,false,nullptr);}

static luabind::object register_class(lua_State *l,const std::string &pclassName,uint32_t idxBaseClassStart)
{
	auto className = pclassName;
	auto fullClassName = className;

	auto nParentClasses = Lua::GetStackTop(l) -1;
	auto fRegisterBaseClasses = [l,nParentClasses,idxBaseClassStart]() {
		for(auto i=idxBaseClassStart;i<=(nParentClasses +1);++i)
		{
			Lua::PushValue(l,-1); /* 2 */
			Lua::PushValue(l,i); /* 3 */
			if(Lua::ProtectedCall(l,1,0) != Lua::StatusCode::Ok) /* 1 */
				Lua::HandleLuaError(l);
		}
	};

	auto d = className.rfind('.');
	std::string slibs;
	if(d != std::string::npos)
	{
		slibs = className;
		className = ustring::substr(className,d +1);

		std::vector<std::string> libs;
		ustring::explode(slibs,".",libs);
		if(libs.empty() == false)
		{
			Lua::GetGlobal(l,libs.front()); /* 1 */
			if(Lua::IsSet(l,-1) == false)
			{
				Lua::Pop(l,1); /* 0 */
				return {};
			}

			auto numPop = 0u;
			for(auto it=libs.begin() +1;it!=libs.end();++it)
			{
				auto bLast = (it == libs.end() -1);
				auto &lib = *it;
				auto t = Lua::GetStackTop(l);
				auto status = Lua::GetProtectedTableValue(l,t,lib);
				if(status == Lua::StatusCode::Ok)
					++numPop; /* 1 */
				if(status != Lua::StatusCode::Ok || Lua::IsSet(l,-1) == false)
				{
					if(bLast == true)
						break;
					Lua::Pop(l,numPop +1); /* 0 */
					return {};
				}
				if(bLast == true)
				{
					Lua::Pop(l,numPop +1); /* 0 */

					auto *nw = engine->GetNetworkState(l);
					auto *game = nw->GetGameState();
					auto *classInfo = game->GetLuaClassManager().FindClassInfo(fullClassName);
					if(classInfo)
					{
						// Re-register base classes for this class, in case they have been changed
						classInfo->regFunc.push(l); /* 1 */
						fRegisterBaseClasses();
						Lua::Pop(l,1); /* 0 */

						return classInfo->classObject;
					}

					return {};
				}
			}
			Lua::Pop(l,numPop +1); /* 0 */
		}
	}

	if(slibs.empty())
	{
		// Check if class already exists
		Lua::GetGlobal(l,className); /* 1 */
		if(Lua::IsUserData(l,-1))
		{
			Lua::Pop(l,1); /* 0 */

			auto *nw = engine->GetNetworkState(l);
			auto *game = nw->GetGameState();
			auto *classInfo = game->GetLuaClassManager().FindClassInfo(fullClassName);
			if(classInfo)
			{
				// Re-register base classes for this class, in case they have been changed
				classInfo->regFunc.push(l); /* 1 */
				fRegisterBaseClasses();
				Lua::Pop(l,1); /* 0 */

				return classInfo->classObject;
			}

			return {};
		}
		Lua::Pop(l,1); /* 0 */
	}

	auto restorePreviousGlobalValue = slibs.empty() == false;
	if(restorePreviousGlobalValue)
		Lua::GetGlobal(l,className); /* +1 */
	std::stringstream ss;
	ss<<"return class '"<<className<<"'";
	auto r = Lua::RunString(l,ss.str(),1,"internal"); /* 1 */
	luabind::object oClass {};
	if(r == Lua::StatusCode::Ok)
	{
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		oClass = luabind::globals(l)[className];
		luabind::object regFc {luabind::from_stack(l,-1)};
		game->GetLuaClassManager().RegisterClass(fullClassName,oClass,regFc);

		// Init default constructor and print methods; They can still be overwritten by the Lua script
		oClass["__init"] = luabind::make_function(l,static_cast<void(*)(lua_State*,const luabind::object&)>([](lua_State *l,const luabind::object &o) {
			auto *crep = Lua::get_crep(o);
			if(!crep)
				return;
			std::vector<luabind::detail::class_rep*> initialized;
			for(auto &base : crep->bases())
			{
				if(std::find(initialized.begin(),initialized.end(),base.base) != initialized.end())
					continue;
				initialized.push_back(base.base);

				base.base->get_table(l);
				auto oBase = luabind::object{luabind::from_stack(l,-1)};
				oBase["__init"](o);
				Lua::Pop(l,1);
			}
		}));
		oClass["__tostring"] = luabind::make_function(l,static_cast<std::string(*)(lua_State*,const luabind::object&)>([](lua_State *l,const luabind::object &o) -> std::string {
			return luabind::get_class_info(luabind::from_stack(l,1)).name;
		}));

		fRegisterBaseClasses();
		Lua::Pop(l,1); /* 0 */

		if(slibs.empty() == false)
		{
			ss = std::stringstream{};
			ss<<slibs<<"="<<className;
			r = Lua::RunString(l,ss.str(),1,"internal"); /* 1 */
			if(r != Lua::StatusCode::Ok)
				Lua::HandleLuaError(l);
			Lua::Pop(l,1); /* 0 */

			Lua::PushNil(l); /* 1 */
			Lua::SetGlobal(l,className); /* 0 */
		}
	}

	if(restorePreviousGlobalValue)
		Lua::SetGlobal(l,className); /* -1 */
	return oClass;
}
luabind::object Lua::util::register_class(lua_State *l,const std::string &className,const luabind::object &oBase0)
{
	if(luabind::type(oBase0) == LUA_TTABLE)
	{
		uint32_t n = 0;
		for(luabind::iterator i(oBase0), e; i != e; ++i)
		{
			(*i).push(l);
			++n;
		}

		auto r = ::register_class(l,className,3);
		for(auto i=decltype(n){0u};i<n;++i)
			Lua::RemoveValue(l,-2);
		return r;
	}
	return ::register_class(l,className,2);
}
luabind::object Lua::util::register_class(lua_State *l,const std::string &className,const luabind::object &oBase0,const luabind::object &oBase1)
{
	return ::register_class(l,className,2);
}
luabind::object Lua::util::register_class(lua_State *l,const std::string &className,const luabind::object &oBase0,const luabind::object &oBase1,const luabind::object &oBase2)
{
	return ::register_class(l,className,2);
}
luabind::object Lua::util::register_class(lua_State *l,const std::string &className,const luabind::object &oBase0,const luabind::object &oBase1,const luabind::object &oBase2,const luabind::object &oBase3)
{
	return ::register_class(l,className,2);
}
luabind::object Lua::util::register_class(lua_State *l,const std::string &className,const luabind::object &oBase0,const luabind::object &oBase1,const luabind::object &oBase2,const luabind::object &oBase3,const luabind::object &oBase4)
{
	return ::register_class(l,className,2);
}

luabind::object Lua::util::register_class(lua_State *l,const std::string &pclassName) {return ::register_class(l,pclassName,2);}

void Lua::util::splash_damage(
	lua_State *l,
	const ::util::SplashDamageInfo &splashDamageInfo
)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto callback = splashDamageInfo.callback;
	if(splashDamageInfo.cone.has_value())
	{
		auto coneAngle = 1.f -(splashDamageInfo.cone->second /180.f) *2.f;
		auto &forward = splashDamageInfo.cone->first;
		auto posEnd = splashDamageInfo.origin +forward *static_cast<float>(splashDamageInfo.radius);
		callback = [&splashDamageInfo,posEnd,forward,coneAngle](BaseEntity *ent,DamageInfo &dmgInfo) -> bool {
			Vector3 min {};
			Vector3 max {};
			auto pPhysComponent = ent->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				pPhysComponent->GetCollisionBounds(&min,&max);
			auto pTrComponent = ent->GetTransformComponent();
			auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
			min += pos;
			max += pos;
			Vector3 posCone {};
			umath::geometry::closest_point_on_aabb_to_point(min,max,posEnd,&posCone);
			auto dirEnt = posCone -splashDamageInfo.origin;
			uvec::normalize(&dirEnt);
			if(uvec::dot(forward,dirEnt) < coneAngle)
				return false;
			if(splashDamageInfo.callback != nullptr)
				return splashDamageInfo.callback(ent,dmgInfo);
			return true;
		};
	}
	game->SplashDamage(splashDamageInfo.origin,splashDamageInfo.radius,const_cast<DamageInfo&>(splashDamageInfo.damageInfo),splashDamageInfo.callback);
}

static void shake_screen(lua_State *l,const Vector3 &pos,float radius,float amplitude,float frequency,float duration,float fadeIn,float fadeOut,bool useRadius)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *ent = game->CreateEntity("env_quake");
	auto *pQuakeComponent = (ent != nullptr) ? static_cast<pragma::BaseEnvQuakeComponent*>(ent->FindComponent("quake").get()) : nullptr;
	if(pQuakeComponent != nullptr)
	{
		auto spawnFlags = SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE;
		if(useRadius == true)
		{
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent != nullptr)
				pTrComponent->SetPosition(pos);
			pQuakeComponent->SetRadius(radius);
		}
		else
			spawnFlags |= SF_QUAKE_GLOBAL_SHAKE;
		ent->SetKeyValue("spawnflags",ustring::int_to_string(spawnFlags));
		pQuakeComponent->SetAmplitude(CFloat(amplitude));
		pQuakeComponent->SetFrequency(CFloat(frequency));
		pQuakeComponent->SetDuration(CFloat(duration));
		pQuakeComponent->SetFadeInDuration(CFloat(fadeIn));
		pQuakeComponent->SetFadeOutDuration(CFloat(fadeOut));
		ent->Spawn();
		auto *pInputComponent = static_cast<pragma::BaseIOComponent*>(ent->FindComponent("io").get());
		if(pInputComponent != nullptr)
			pInputComponent->Input("StartShake",nullptr,nullptr,"");
	}
}
void Lua::util::shake_screen(lua_State *l,const Vector3 &pos,float radius,float amplitude,float frequency,float duration,float fadeIn,float fadeOut)
{
	::shake_screen(l,pos,radius,amplitude,frequency,duration,fadeIn,fadeOut,true);
}
void Lua::util::shake_screen(lua_State *l,float amplitude,float frequency,float duration,float fadeIn,float fadeOut)
{
	::shake_screen(l,{},{},amplitude,frequency,duration,fadeIn,fadeOut,false);
}

float Lua::util::get_faded_time_factor(float cur,float dur,float fadeIn,float fadeOut) {return ::util::get_faded_time_factor(CFloat(cur),CFloat(dur),fadeIn,fadeOut);}
float Lua::util::get_faded_time_factor(float cur,float dur,float fadeIn) {return get_faded_time_factor(cur,dur,fadeIn,0.f);}
float Lua::util::get_faded_time_factor(float cur,float dur) {return get_faded_time_factor(cur,dur,0.f);}

float Lua::util::get_scale_factor(float val,float min,float max) {return ::util::get_scale_factor(CFloat(val),CFloat(min),CFloat(max));}
float Lua::util::get_scale_factor(float val,float min) {return ::util::get_scale_factor(CFloat(val),CFloat(min));}

Quat Lua::util::local_to_world(lua_State *l,const Quat &r0,const Quat &r1)
{
	auto res = r1;
	uvec::local_to_world(r0,res);
	return res;
}

Vector3 Lua::util::local_to_world(lua_State *l,const Vector3 &vLocal,const Quat &rLocal,const Vector3 &v)
{
	auto vOut = v;
	uvec::local_to_world(vLocal,rLocal,vOut);
	return vOut;
}

void Lua::util::local_to_world(lua_State *l,const Vector3 &vLocal,const Quat &rLocal,const Vector3 &v,const Quat &r)
{
	auto vOut = v;
	auto rOut = r;
	uvec::local_to_world(vLocal,rLocal,vOut,rOut);
	Lua::Push<Vector3>(l,vOut);
	Lua::Push<Quat>(l,rOut);
}

Quat Lua::util::world_to_local(lua_State *l,const Quat &rLocal,const Quat &r)
{
	auto rOut = r;
	uvec::world_to_local(rLocal,rOut);
	return rOut;
}

Vector3 Lua::util::world_to_local(lua_State *l,const Vector3 &vLocal,const Quat &rLocal,const Vector3 &v)
{
	auto vOut = v;
	uvec::world_to_local(vLocal,rLocal,vOut);
	return vOut;
}

void Lua::util::world_to_local(lua_State *l,const Vector3 &vLocal,const Quat &rLocal,const Vector3 &v,const Quat &r)
{
	auto vOut = v;
	auto rOut = r;
	uvec::world_to_local(vLocal,rLocal,vOut,rOut);
	Lua::Push<Vector3>(l,vOut);
	Lua::Push<Quat>(l,rOut);
}

Vector3 Lua::util::calc_world_direction_from_2d_coordinates(
	lua_State *l,const Vector3 &forward,const Vector3 &right,const Vector3 &up,
	float fov,float nearZ,float farZ,float aspectRatio,const Vector2 &uv
)
{
	return uvec::calc_world_direction_from_2d_coordinates(forward,right,up,static_cast<float>(umath::deg_to_rad(fov)),nearZ,farZ,aspectRatio,0.f,0.f,uv);
}
void Lua::util::world_space_point_to_screen_space_uv(lua_State *l,const Vector3 &point,const Mat4 &vp,float nearZ,float farZ)
{
	float dist;
	auto uv = uvec::calc_screenspace_uv_from_worldspace_position(point,vp,nearZ,farZ,dist);
	Lua::Push(l,uv);
	Lua::PushNumber(l,dist);
}
::Vector2 Lua::util::world_space_point_to_screen_space_uv(lua_State *l,const Vector3 &point,const Mat4 &vp)
{
	return uvec::calc_screenspace_uv_from_worldspace_position(point,vp);
}
Vector2 Lua::util::world_space_direction_to_screen_space(lua_State *l,const Vector3 &dir,const Mat4 &vp)
{
	return uvec::calc_screenspace_direction_from_worldspace_direction(dir,vp);
}
float Lua::util::calc_screenspace_distance_to_worldspace_position(lua_State *l,const Vector3 &point,const Mat4 &vp,float nearZ,float farZ)
{
	return uvec::calc_screenspace_distance_to_worldspace_position(point,vp,nearZ,farZ);
}
float Lua::util::depth_to_distance(lua_State *l,float depth,float nearZ,float farZ)
{
	return uvec::depth_to_distance(depth,nearZ,farZ);
}
void Lua::util::open_url_in_browser(const std::string &url) {return ::util::open_url_in_browser(url);}
void Lua::util::open_path_in_explorer(const std::string &spath,const std::string &selectFile)
{
	auto path = ::util::Path::CreatePath(spath) +::util::Path::CreateFile(selectFile);
	std::string strAbsPath;
	if(FileManager::FindAbsolutePath(path.GetString(),strAbsPath) == false)
		return;
	auto absPath = ::util::Path::CreateFile(strAbsPath);
	::util::open_path_in_explorer(absPath.GetPath(),absPath.GetFileName());
}
void Lua::util::open_path_in_explorer(const std::string &spath)
{
	auto path = ::util::Path::CreatePath(spath);
	std::string strAbsPath;
	if(FileManager::FindAbsolutePath(path.GetString(),strAbsPath) == false)
		return;
	auto absPath = ::util::Path::CreatePath(strAbsPath);
	::util::open_path_in_explorer(absPath.GetPath());
}
void Lua::util::clamp_resolution_to_aspect_ratio(lua_State *l,uint32_t w,uint32_t h,float aspectRatio)
{
	Vector2i size {w,h};
	w = size.y *aspectRatio;
	h = size.y;
	if(w > size.x)
	{
		w = size.x;
		h = size.x /aspectRatio;
	}
	Lua::PushNumber(l,w);
	Lua::PushNumber(l,h);
}
std::string Lua::util::get_pretty_bytes(uint32_t bytes) {return ::util::get_pretty_bytes(bytes);}
std::string Lua::util::get_pretty_duration(lua_State *l,uint32_t ms) {return get_pretty_duration(l,ms,0,true);}
std::string Lua::util::get_pretty_duration(lua_State *l,uint32_t ms,uint32_t segments) {return get_pretty_duration(l,ms,segments,true);}
std::string Lua::util::get_pretty_duration(lua_State *l,uint32_t ms,uint32_t segments,bool noMs)
{
	return ::util::get_pretty_duration(ms,segments,noMs);
}
bool Lua::util::is_same_object(lua_State *l,const luabind::object &o0,const luabind::object &o1) {return lua_rawequal(l,1,2) == 1;}
std::string Lua::util::get_pretty_time(lua_State *l,float t)
{
	auto sign = umath::sign(static_cast<float>(t));
	t *= sign;
	auto seconds = umath::floor(t);
	auto milliseconds = umath::floor((t -seconds) *1'000.f);
	auto minutes = umath::floor(seconds /60.f);
	seconds -= minutes *60.0;
	auto hours = umath::floor(minutes /60.f);
	minutes -= hours *60.f;

	auto prettyTime = ustring::fill_zeroes(::util::round_string(seconds),2) +'.' +
		ustring::fill_zeroes(::util::round_string(milliseconds),3);

	prettyTime = ustring::fill_zeroes(::util::round_string(minutes),2) +':' +prettyTime;

	if(hours > 0.f)
		prettyTime = ustring::fill_zeroes(::util::round_string(hours),2) +':' +prettyTime;

	if(sign < 0)
		prettyTime = '-' +prettyTime;
	return prettyTime;
}
double Lua::util::units_to_metres(double units) {return ::util::pragma::units_to_metres(units);}
double Lua::util::metres_to_units(double metres) {return ::util::pragma::metres_to_units(metres);}
luabind::object Lua::util::read_scene_file(lua_State *l,const std::string &fileName)
{
	auto fname = "scenes\\" +FileManager::GetCanonicalizedPath(fileName);
	auto f = FileManager::OpenFile(fname.c_str(),"r");
	if(f == nullptr)
		return {};
	se::SceneScriptValue root {};
	if(se::read_scene(f,root) != ::util::MarkupFile::ResultCode::Ok)
		return {};
	std::function<void(const se::SceneScriptValue&)> fPushValue = nullptr;
	fPushValue = [l,&fPushValue](const se::SceneScriptValue &val) {
		auto t = Lua::CreateTable(l);

		Lua::PushString(l,"identifier");
		Lua::PushString(l,val.identifier);
		Lua::SetTableValue(l,t);

		Lua::PushString(l,"parameters");
		auto tParams = Lua::CreateTable(l);
		auto paramIdx = 1u;
		for(auto &param : val.parameters)
		{
			Lua::PushInt(l,paramIdx++);
			Lua::PushString(l,param);
			Lua::SetTableValue(l,tParams);
		}
		Lua::SetTableValue(l,t);

		Lua::PushString(l,"children");
		auto tChildren = Lua::CreateTable(l);
		auto childIdx = 1u;
		for(auto &child : val.subValues)
		{
			Lua::PushInt(l,childIdx++);
			fPushValue(*child);
			Lua::SetTableValue(l,tChildren);
		}
		Lua::SetTableValue(l,t);
	};
	fPushValue(root);
	auto o = luabind::object{luabind::from_stack{l,-1}};
	Lua::Pop(l,1);
	return o;
}

template<class TProperty,typename TUnderlyingType>
	static luabind::object fade_property_generic(Game &game,lua_State *l,TProperty &vProp,const TUnderlyingType &vDst,float duration,const std::function<TUnderlyingType(const TUnderlyingType&,const TUnderlyingType&,float)> &fLerp)
{
	auto vSrc = vProp.GetValue();
	if(duration == 0.f)
	{
		vProp.SetValue(vDst);
		return luabind::object{};
	}
	auto tStart = game.RealTime();
	auto cb = FunctionCallback<void>::Create(nullptr);
	cb.get<Callback<void>>()->SetFunction([&game,tStart,duration,vProp,vSrc,vDst,cb,fLerp]() mutable {
		auto tDelta = game.RealTime() -tStart;
		auto sc = umath::min(umath::smooth_step(0.f,1.f,static_cast<float>(tDelta /duration)),1.f);
		auto vNew = fLerp(vSrc,vDst,sc);
		vProp.SetValue(vNew);
		if(sc == 1.f)
		{
			if(cb.IsValid())
				cb.Remove();
		}
	});
	game.AddCallback("Think",cb);
	return luabind::object{l,cb};
}

template<class TProperty,typename TUnderlyingType>
	luabind::object fade_vector_property_generic(Game &game,lua_State *l,TProperty &vProp,const TUnderlyingType &vDst,float duration,const std::function<TUnderlyingType(const TUnderlyingType&,const TUnderlyingType&,float)> &fLerp)
{
	return fade_property_generic<TProperty,TUnderlyingType>(game,l,vProp,vDst,duration,fLerp);
}

luabind::object Lua::util::fade_property(lua_State *l,LColorProperty &colProp,const Color &colDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto hsvSrc = ::util::rgb_to_hsv(*colProp);
	auto hsvDst = ::util::rgb_to_hsv(colDst);
	auto aSrc = (*colProp)->a;
	auto aDst = colDst.a;

	auto tStart = game.RealTime();
	auto cb = FunctionCallback<void>::Create(nullptr);
	cb.get<Callback<void>>()->SetFunction([&game,tStart,duration,colProp,hsvSrc,hsvDst,aSrc,aDst,cb]() mutable {
		auto tDelta = game.RealTime() -tStart;
		auto sc = umath::min(umath::smooth_step(0.f,1.f,static_cast<float>(tDelta /duration)),1.f);
		auto hsv = ::util::lerp_hsv(hsvSrc,hsvDst,sc);
		auto newColor = ::util::hsv_to_rgb(hsv);
		newColor.a = aSrc +(aDst -aSrc) *sc;
		*colProp = newColor;
		if(sc == 1.f)
		{
			if(cb.IsValid())
				cb.Remove();
		}
	});
	game.AddCallback("Think",cb);
	return luabind::object{l,cb};
}

luabind::object Lua::util::fade_property(lua_State *l,LVector2Property &vProp,const ::Vector2 &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector2Property,Vector2>(game,l,vProp,vDst,duration,[](const Vector2 &a,const Vector2 &b,float factor) -> Vector2 {
		return Vector2{umath::lerp(a.x,b.x,factor),umath::lerp(a.y,b.y,factor)};
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LVector2iProperty &vProp,const ::Vector2i &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector2iProperty,Vector2i>(game,l,vProp,vDst,duration,[](const Vector2i &a,const Vector2i &b,float factor) -> Vector2i {
		return Vector2i{static_cast<int32_t>(umath::lerp(a.x,b.x,factor)),static_cast<int32_t>(umath::lerp(a.y,b.y,factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LVector3Property &vProp,const ::Vector3 &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector3Property,Vector3>(game,l,vProp,vDst,duration,uvec::lerp);
}

luabind::object Lua::util::fade_property(lua_State *l,LVector3iProperty &vProp,const ::Vector3i &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector3iProperty,Vector3i>(game,l,vProp,vDst,duration,uvec::lerp);
}

luabind::object Lua::util::fade_property(lua_State *l,LVector4Property &vProp,const ::Vector4 &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector4Property,Vector4>(game,l,vProp,vDst,duration,[](const Vector4 &a,const Vector4 &b,float factor) -> Vector4 {
		return Vector4{umath::lerp(a.x,b.x,factor),umath::lerp(a.y,b.y,factor),umath::lerp(a.z,b.z,factor),umath::lerp(a.w,b.w,factor)};
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LVector4iProperty &vProp,const ::Vector4i &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector4iProperty,Vector4i>(game,l,vProp,vDst,duration,[](const Vector4i &a,const Vector4i &b,float factor) -> Vector4i {
		return Vector4i{static_cast<int32_t>(umath::lerp(a.x,b.x,factor)),static_cast<int32_t>(umath::lerp(a.y,b.y,factor)),static_cast<int32_t>(umath::lerp(a.z,b.z,factor)),static_cast<int32_t>(umath::lerp(a.w,b.w,factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LQuatProperty &vProp,const ::Quat &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LQuatProperty,Quat>(game,l,vProp,vDst,duration,uquat::slerp);
}

luabind::object Lua::util::fade_property(lua_State *l,LEulerAnglesProperty &vProp,const ::EulerAngles &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LEulerAnglesProperty,EulerAngles>(game,l,vProp,vDst,duration,[](const EulerAngles &a,const EulerAngles &b,float factor) -> EulerAngles {
		return EulerAngles{static_cast<float>(umath::lerp_angle(a.p,b.p,factor)),static_cast<float>(umath::lerp_angle(a.y,b.y,factor)),static_cast<float>(umath::lerp_angle(a.r,b.r,factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LGenericIntPropertyWrapper &vProp,const int64_t &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LGenericIntPropertyWrapper,int64_t>(game,l,vProp,vDst,duration,[](const int64_t &a,const int64_t &b,float factor) -> int64_t {
		return umath::lerp(a,b,factor);
	});
}

luabind::object Lua::util::fade_property(lua_State *l,LGenericFloatPropertyWrapper &vProp,const double &vDst,float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LGenericFloatPropertyWrapper,double>(game,l,vProp,vDst,duration,[](const double &a,const double &b,float factor) -> double {
		return umath::lerp(a,b,factor);
	});
}

std::string Lua::util::round_string(lua_State *l,float value) {return round_string(l,value,0);}
std::string Lua::util::round_string(lua_State *l,float value,uint32_t places) {return ::util::round_string(value,places);}

std::string Lua::util::get_type_name(lua_State *l,const luabind::object &o)
{
	if(Lua::IsSet(l,1) == false)
		return Lua::IsNone(l,1) ? "none" : "nil";

	auto *crep = Lua::get_crep(o);
	if(crep)
		return crep->name();
	return lua_typename(l,lua_type(l,-1));
}

std::string Lua::util::variable_type_to_string(::util::VarType varType) {return ::util::variable_type_to_string(varType);}
std::string Lua::util::get_string_hash(const std::string &str) {return std::to_string(std::hash<std::string>{}(str));}

luabind::object Lua::util::get_class_value(lua_State *l,const luabind::object &oClass,const std::string &key)
{
	int32_t t = 1;
	Lua::CheckUserData(l,t);
	
	auto n = Lua::GetStackTop(l);
	Lua::PushString(l,key); /* 1 */
	auto r = Lua::GetProtectedTableValue(l,t); /* 2 */
	if(r != Lua::StatusCode::Ok)
	{
		Lua::Pop(l,Lua::GetStackTop(l) -n);
		return {};
	}
	// Pop key from stack
	Lua::RemoveValue(l,-2); /* 1 */
	luabind::object ro {luabind::from_stack{l,-1}};
	Lua::Pop(l);
	return ro;
}

void Lua::util::pack_zip_archive(lua_State *l,const std::string &pzipFileName,const luabind::table<> &t)
{
	auto zipFileName = pzipFileName;
	ufile::remove_extension_from_filename(zipFileName);
	zipFileName += ".zip";
	if(Lua::file::validate_write_operation(l,zipFileName) == false)
	{
		Lua::PushBool(l,false);
		return;
	}

	std::unordered_map<std::string,std::string> files {};
	std::unordered_map<std::string,std::string> customTextFiles {};
	std::unordered_map<std::string,DataStream> customBinaryFiles {};
	auto numFiles = Lua::GetObjectLength(l,t);
	if(numFiles > 0)
	{
		// Table format: t{[1] = diskFileName/zipFileName,...}
		files.reserve(numFiles);
		for(luabind::iterator i(t), e; i != e; ++i)
		{
			auto fileName = luabind::object_cast<std::string>(*i);
			files[fileName] = fileName;
		}
	}
	else
	{
		// Table format: t{[zipFileName] = diskFileName,...}
		for(luabind::iterator i(t), e; i != e; ++i)
		{
			auto zipFileName = luabind::object_cast<std::string>(i.key());
			auto value = *i;
			if(luabind::type(value) == LUA_TTABLE)
			{
				auto *ds = luabind::object_cast_nothrow<DataStream*>(value["contents"],static_cast<DataStream*>(nullptr));
				if(ds)
					customBinaryFiles[zipFileName] = *ds;
				else
					customTextFiles[zipFileName] = luabind::object_cast<std::string>(value["contents"]);
			}
			else
			{
				auto diskFileName = luabind::object_cast<std::string>(*i);
				files[zipFileName] = diskFileName;
			}
		}
	}

	auto zip = ZIPFile::Open(zipFileName,ZIPFile::OpenFlags::CreateIfNotExist);
	if(zip == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto tNotFound = luabind::newtable(l);
	uint32_t notFoundIdx = 1;
	for(auto &pair : files)
	{
		auto f = FileManager::OpenFile(pair.second.c_str(),"rb");
		if(f == nullptr)
		{
			tNotFound[notFoundIdx++] = pair.second;
			continue;
		}
		auto sz = f->GetSize();
		std::vector<uint8_t> data {};
		data.resize(sz);
		f->Read(data.data(),sz);
		zip->AddFile(pair.first,data.data(),sz);
	}
	for(auto &pair : customTextFiles)
		zip->AddFile(pair.first,pair.second);
	for(auto &pair : customBinaryFiles)
	{
		auto &ds = pair.second;
		zip->AddFile(pair.first,ds->GetData(),ds->GetInternalSize());
	}
	zip = nullptr;
	Lua::PushBool(l,true);
	tNotFound.push(l);
}

std::string Lua::util::get_addon_path(lua_State *l,const std::string &relPath)
{
	std::string rpath;
	if(FileManager::FindAbsolutePath(relPath,rpath) == false)
		return relPath;
	::util::Path path{relPath};
	path.MakeRelative(::util::get_program_path());
	return path.GetString();
}

std::string Lua::util::get_addon_path(lua_State *l)
{
	auto path = Lua::get_current_file(l);
	ustring::replace(path,"\\","/");
	if(ustring::compare(path.c_str(),"addons/",false,7) == false)
		return "";

	// Get path up to addon directory
	auto br = path.find('/',7);
	if(br != std::string::npos)
		path = path.substr(0,br);
	path += '/';
	return path;
}
