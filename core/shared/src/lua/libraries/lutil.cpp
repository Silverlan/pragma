/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lutil.h"
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

extern DLLENGINE Engine *engine;

#pragma optimize("",off)
static auto s_bIgnoreIncludeCache = false;
void Lua::set_ignore_include_cache(bool b) {s_bIgnoreIncludeCache = b;}

int Lua::global::include(lua_State *l)
{
	std::string f = Lua::CheckString(l,1);
	auto bIgnoreCache = s_bIgnoreIncludeCache;
	if(Lua::IsSet(l,2) && bIgnoreCache == false)
		bIgnoreCache = Lua::CheckBool(l,2);
	auto *lInterface = engine->GetLuaInterface(l);
	std::vector<std::string> *includeCache = (lInterface != nullptr) ? &lInterface->GetIncludeCache() : nullptr;
	auto fShouldInclude = [includeCache,bIgnoreCache](std::string fpath) -> bool {
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
		if(it != includeCache->end())
			return bIgnoreCache;
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
		return 0;
	}
	if(fShouldInclude(f) == true)
	{
		//auto r = Lua::Execute(l,[&f,l](int(*traceback)(lua_State*)) {
		//	return Lua::IncludeFile(l,f,traceback);
		//});
		auto n = Lua::GetStackTop(l);
		auto r = Lua::IncludeFile(l,f,Lua::HandleTracebackError,LUA_MULTRET);
		switch(r)
		{
			case Lua::StatusCode::ErrorFile:
				lua_error(l);
				/* unreachable */
				break;
			case Lua::StatusCode::ErrorSyntax:
				Lua::HandleSyntaxError(l,r,f);
				break;
			case Lua::StatusCode::Ok:
				return Lua::GetStackTop(l) -n;
		}
	}
	return 0;
}

int Lua::global::get_script_path(lua_State *l)
{
	auto path = Lua::GetIncludePath();
	Lua::PushString(l,path);
	return 1;
}

int Lua::global::angle_rand(lua_State *l)
{
	luabind::object(l,EulerAngles(umath::random(-180.f,180.f),umath::random(-180.f,180.f),umath::random(-180.f,180.f))).push(l);
	return 1;
}

int Lua::global::create_from_string(lua_State *l)
{
	auto *str = Lua::CheckString(l,1);
	EulerAngles ang(str);
	Lua::Push<EulerAngles>(l,ang);
	return 1;
}

static bool check_valid_lua_object(lua_State *l,int32_t stackIdx)
{
	auto o = luabind::object(luabind::from_stack(l,stackIdx));
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

static bool is_valid(lua_State *l)
{
	auto bValid = Lua::IsSet(l,1);
	if(bValid == true)
	{
		if(Lua::IsUserData(l,1))
			bValid = check_valid_lua_object(l,1);
		else if(Lua::IsTable(l,1))
		{
			auto numEls = Lua::GetObjectLength(l,1);
			for(auto i=decltype(numEls){0};i<numEls;++i)
			{
				Lua::PushInt(l,i +1);
				Lua::GetTableValue(l,1);

				bValid = check_valid_lua_object(l,-1);

				Lua::Pop(l,1);
				if(bValid == false)
					break;
			}
		}
	}
	return bValid;
}

int Lua::util::is_valid(lua_State *l)
{
	Lua::PushBool(l,::is_valid(l));
	return 1;
}

int Lua::util::is_valid_entity(lua_State *l)
{
	if(!Lua::IsSet(l,1) || !Lua::IsEntity(l,1))
	{
		Lua::PushBool(l,false);
		return 1;
	}
	return ::is_valid(l);
}

int Lua::util::remove(lua_State *l)
{
	auto valid = ::is_valid(l);
	if(valid == false)
		return 0;
	auto o = luabind::object(luabind::from_stack(l,1));
	auto *pEnt = luabind::object_cast_nothrow<EntityHandle*>(o,static_cast<EntityHandle*>(nullptr));
	if(pEnt != nullptr)// Used frequently, and is faster than looking up "IsValid"
	{
		if(pEnt->IsValid())
			(*pEnt)->Remove();
		return 0;
	}
	try
	{
		auto oRemove = o["Remove"];
		if(!oRemove)
			return 0;
		luabind::call_member<void>(o,"Remove");
	}
	catch(std::exception&) // No "IsValid" method exists
	{}
	return 0;
}

int Lua::util::is_table(lua_State *l)
{
	lua_pushboolean(l,(!lua_isnoneornil(l,1) && lua_istable(l,1)) ? true : false);
	return 1;
}

int Lua::util::date_time(lua_State *l)
{
	int argc = lua_gettop(l);
	std::string datetime;
	if(argc == 0)
		datetime = engine->GetDate();
	else
	{
		std::string format = luaL_checkstring(l,1);
		datetime = engine->GetDate(format);
	}
	lua_pushstring(l,datetime.c_str());
	return 1;
}

int Lua::util::fire_bullets(lua_State *l,const std::function<void(DamageInfo&,TraceData&,TraceResult&,uint32_t&)> &f)
{
	auto *bulletInfo = Lua::CheckBulletInfo(l,1);
	auto bHitReport = false;
	if(Lua::IsSet(l,2) == true)
		bHitReport = Lua::CheckBool(l,2);

	DamageInfo dmg;
	dmg.SetDamage(umath::min(CUInt16(bulletInfo->damage),CUInt16(std::numeric_limits<UInt16>::max())));
	dmg.SetDamageType(bulletInfo->damageType);
	dmg.SetSource(bulletInfo->effectOrigin);
	dmg.SetAttacker(bulletInfo->hAttacker.get());
	dmg.SetInflictor(bulletInfo->hInflictor.get());

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	auto &src = dmg.GetSource();
	int32_t idxResultTable = -1;
	if(bHitReport == true)
		idxResultTable = Lua::CreateTable(l);
	int32_t tIdx = 1;
	for(Int32 i=0;i<bulletInfo->bulletCount;i++)
	{
		auto randSpread = EulerAngles(umath::random(-bulletInfo->spread.p,bulletInfo->spread.p),umath::random(-bulletInfo->spread.y,bulletInfo->spread.y),0);
		auto bulletDir = bulletInfo->direction;
		uvec::rotate(&bulletDir,randSpread);
		TraceData data;
		data.SetSource(src);
		data.SetTarget(src +bulletDir *bulletInfo->distance);
		data.SetCollisionFilterMask(CollisionMask::AllHitbox &~CollisionMask::Trigger); // Let everything pass (Except specific filters below)
		auto *attacker = dmg.GetAttacker();
		data.SetFilter([attacker](pragma::physics::IShape &shape,pragma::physics::IRigidBody &body) -> RayCastHitType {
			auto *phys = body.GetPhysObj();
			auto *ent = phys ? phys->GetOwner() : nullptr;
			if(ent == nullptr || &ent->GetEntity() == attacker) // Attacker can't shoot themselves
				return RayCastHitType::None;
			auto filterGroup = phys->GetCollisionFilter();
			auto mdlComponent = ent->GetEntity().GetModelComponent();
			if(mdlComponent.valid() && mdlComponent->GetHitboxCount() > 0 && (filterGroup &CollisionMask::NPC) != CollisionMask::None || (filterGroup &CollisionMask::Player) != CollisionMask::None) // Filter out player and NPC collision objects, since we only want to check their hitboxes
				return RayCastHitType::None;
			return RayCastHitType::Block;
		});
		auto filterGroup = CollisionMask::None;
		if(attacker != nullptr)
		{
			auto pPhysComponent = attacker->GetPhysicsComponent();
			if(pPhysComponent.valid())
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
				dmg.SetForce(bulletDir *bulletInfo->force);
				dmg.SetHitPosition(result.position);
				pDamageableComponent->TakeDamage(dmg);
			}
		}
		if(idxResultTable != -1)
		{
			if(results.empty())
			{
				TraceResult result {data};
				Lua::PushInt(l,tIdx++);
				Lua_TraceData_FillTraceResultTable(l,result);
				Lua::SetTableValue(l,idxResultTable);
			}
			else
			{
				for(auto &result : results)
				{
					Lua::PushInt(l,tIdx++);
					Lua_TraceData_FillTraceResultTable(l,result);
					Lua::SetTableValue(l,idxResultTable);
				}
			}
		}
		if(f != nullptr)
		{
			if(results.empty())
			{
				TraceResult result {data};
				f(dmg,data,result,bulletInfo->tracerCount);
			}
			else
			{
				for(auto &result : results)
					f(dmg,data,result,bulletInfo->tracerCount);
			}
		}
	}
	return (idxResultTable == -1) ? 0 : 1;
}
int Lua::util::fire_bullets(lua_State *l) {return fire_bullets(l,nullptr);}

int Lua::util::register_class(lua_State *l)
{
	std::string className = Lua::CheckString(l,1);
	auto fullClassName = className;

	auto nParentClasses = Lua::GetStackTop(l) -1;
	auto fRegisterBaseClasses = [l,nParentClasses]() {
		for(auto i=decltype(nParentClasses){2};i<=(nParentClasses +1);++i)
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
				Lua::PushBool(l,false);
				return 1;
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
					Lua::PushBool(l,false);
					return 1;
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
						classInfo->classObject.push(l); /* 1 */
						fRegisterBaseClasses();
						Lua::Pop(l,1); /* 0 */
					}

					Lua::PushBool(l,false);
					return 1;
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
				classInfo->classObject.push(l); /* 1 */
				fRegisterBaseClasses();
				Lua::Pop(l,1); /* 0 */
			}

			Lua::PushBool(l,false);
			return 1;
		}
		Lua::Pop(l,1); /* 0 */
	}

	auto restorePreviousGlobalValue = slibs.empty() == false;
	if(restorePreviousGlobalValue)
		Lua::GetGlobal(l,className); /* +1 */
	std::stringstream ss;
	ss<<"return class '"<<className<<"'";
	auto r = Lua::RunString(l,ss.str(),1,"internal"); /* 1 */
	if(r == Lua::StatusCode::Ok)
	{
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		luabind::object oClass {luabind::from_stack(l,-1)};
		game->GetLuaClassManager().RegisterClass(fullClassName,oClass);

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

	Lua::PushBool(l,true);
	return 1;
}

int Lua::util::splash_damage(
	lua_State *l,
	::util::SplashDamageInfo &splashDamageInfo
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
			if(pPhysComponent.valid())
				pPhysComponent->GetCollisionBounds(&min,&max);
			auto pTrComponent = ent->GetTransformComponent();
			auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
			min += pos;
			max += pos;
			Vector3 posCone {};
			Geometry::ClosestPointOnAABBToPoint(min,max,posEnd,&posCone);
			auto dirEnt = posCone -splashDamageInfo.origin;
			uvec::normalize(&dirEnt);
			if(uvec::dot(forward,dirEnt) < coneAngle)
				return false;
			if(splashDamageInfo.callback != nullptr)
				return splashDamageInfo.callback(ent,dmgInfo);
			return true;
		};
	}
	game->SplashDamage(splashDamageInfo.origin,splashDamageInfo.radius,splashDamageInfo.damageInfo,splashDamageInfo.callback);
	return 0;
}

int Lua::util::splash_damage(lua_State *l)
{
	auto &splashDamageInfo = Lua::Check<::util::SplashDamageInfo>(l,1);
	return splash_damage(l,splashDamageInfo);
}

int Lua::util::shake_screen(lua_State *l)
{
	auto iarg = 1;
	Vector3 pos(0.f,0.f,0.f);
	auto radius = 0.f;
	auto bUseRadius = false;
	if(Lua::IsVector(l,iarg))
	{
		auto *lpos = Lua::CheckVector(l,iarg++);
		pos = *lpos;
		radius = CFloat(Lua::CheckNumber(l,iarg++));
		bUseRadius = true;
	}
	auto amplitude = Lua::CheckNumber(l,iarg++);
	auto frequency = Lua::CheckNumber(l,iarg++);
	auto duration = Lua::CheckNumber(l,iarg++);
	auto fadeIn = Lua::CheckNumber(l,iarg++);
	auto fadeOut = Lua::CheckNumber(l,iarg++);

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *ent = game->CreateEntity("env_quake");
	auto *pQuakeComponent = (ent != nullptr) ? static_cast<pragma::BaseEnvQuakeComponent*>(ent->FindComponent("quake").get()) : nullptr;
	if(pQuakeComponent != nullptr)
	{
		auto spawnFlags = SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE;
		if(bUseRadius == true)
		{
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent.valid())
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
	return 0;
}

int Lua::util::get_faded_time_factor(lua_State *l)
{
	auto cur = Lua::CheckNumber(l,1);
	auto dur = Lua::CheckNumber(l,2);
	auto fadeIn = 0.f;
	if(Lua::IsSet(l,3))
		fadeIn = CFloat(Lua::CheckNumber(l,3));
	auto fadeOut = 0.f;
	if(Lua::IsSet(l,4))
		fadeOut = CFloat(Lua::CheckNumber(l,4));
	Lua::PushNumber(l,::util::get_faded_time_factor(CFloat(cur),CFloat(dur),fadeIn,fadeOut));
	return 1;
}

int Lua::util::get_scale_factor(lua_State *l)
{
	auto val = Lua::CheckNumber(l,1);
	auto min = Lua::CheckNumber(l,2);
	if(!Lua::IsSet(l,3))
	{
		Lua::PushNumber(l,::util::get_scale_factor(CFloat(val),CFloat(min)));
		return 1;
	}
	auto max = Lua::CheckNumber(l,3);
	Lua::PushNumber(l,::util::get_scale_factor(CFloat(val),CFloat(min),CFloat(max)));
	return 1;
}

int Lua::util::local_to_world(lua_State *l)
{
	if(Lua::IsQuaternion(l,1))
	{
		auto *rLocal = Lua::CheckQuaternion(l,1);
		auto *r = Lua::CheckQuaternion(l,2);
		auto rOut = *r;
		uvec::local_to_world(*rLocal,rOut);
		Lua::Push<Quat>(l,rOut);
		return 1;
	}
	auto *vLocal = Lua::CheckVector(l,1);
	auto *rLocal = Lua::CheckQuaternion(l,2);
	auto *v = Lua::CheckVector(l,3);
	if(!Lua::IsSet(l,4))
	{
		auto vOut = *v;
		uvec::local_to_world(*vLocal,*rLocal,vOut);
		Lua::Push<Vector3>(l,vOut);
		return 1;
	}
	auto *r = Lua::CheckQuaternion(l,4);
	auto vOut = *v;
	auto rOut = *r;
	uvec::local_to_world(*vLocal,*rLocal,vOut,rOut);
	Lua::Push<Vector3>(l,vOut);
	Lua::Push<Quat>(l,rOut);
	return 2;
}
int Lua::util::world_to_local(lua_State *l)
{
	if(Lua::IsQuaternion(l,1))
	{
		auto *rLocal = Lua::CheckQuaternion(l,1);
		auto *r = Lua::CheckQuaternion(l,2);
		auto rOut = *r;
		uvec::world_to_local(*rLocal,rOut);
		Lua::Push<Quat>(l,rOut);
		return 1;
	}
	auto *vLocal = Lua::CheckVector(l,1);
	auto *rLocal = Lua::CheckQuaternion(l,2);
	auto *v = Lua::CheckVector(l,3);
	if(!Lua::IsSet(l,4))
	{
		auto vOut = *v;
		uvec::world_to_local(*vLocal,*rLocal,vOut);
		Lua::Push<Vector3>(l,vOut);
		return 1;
	}
	auto *r = Lua::CheckQuaternion(l,4);
	auto vOut = *v;
	auto rOut = *r;
	uvec::world_to_local(*vLocal,*rLocal,vOut,rOut);
	Lua::Push<Vector3>(l,vOut);
	Lua::Push<Quat>(l,rOut);
	return 2;
}
int Lua::util::calc_world_direction_from_2d_coordinates(lua_State *l)
{
	int32_t arg = 1;
	auto *forward = Lua::CheckVector(l,arg++);
	auto *right = Lua::CheckVector(l,arg++);
	auto *up = Lua::CheckVector(l,arg++);
	auto fov = Lua::CheckNumber(l,arg++);
	auto nearZ = Lua::CheckNumber(l,arg++);
	auto farZ = Lua::CheckNumber(l,arg++);
	auto aspectRatio = Lua::CheckNumber(l,arg++);
	auto width = Lua::CheckInt(l,arg++);
	auto height = Lua::CheckInt(l,arg++);
	auto *uv = Lua::CheckVector2(l,arg++);

	auto dir = uvec::calc_world_direction_from_2d_coordinates(*forward,*right,*up,static_cast<float>(umath::deg_to_rad(fov)),nearZ,farZ,aspectRatio,static_cast<float>(width),static_cast<float>(height),*uv);
	Lua::Push<Vector3>(l,dir);
	return 1;
}
int Lua::util::open_url_in_browser(lua_State *l)
{
	std::string url = Lua::CheckString(l,1);
	::util::open_url_in_browser(url);
	return 0;
}
int Lua::util::open_path_in_explorer(lua_State *l)
{
	std::string path = Lua::CheckString(l,1);
	path = ::util::get_program_path() +'/' +path;
	std::optional<std::string> selectFile {};
	if(Lua::IsSet(l,2))
		selectFile = Lua::CheckString(l,2);
	::util::open_path_in_explorer(path,selectFile);
	return 0;
}
int Lua::util::clamp_resolution_to_aspect_ratio(lua_State *l)
{
	auto w = Lua::CheckInt(l,1);
	auto h = Lua::CheckInt(l,2);
	auto aspectRatio = Lua::CheckNumber(l,3);
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
	return 2;
}
int Lua::util::get_pretty_bytes(lua_State *l)
{
	auto bytes = Lua::CheckInt(l,1);
	auto r = ::util::get_pretty_bytes(bytes);
	Lua::PushString(l,r);
	return 1;
}
int Lua::util::get_pretty_duration(lua_State *l)
{
	auto ms = Lua::CheckInt(l,1);
	int32_t addSegments = 0;
	if(Lua::IsSet(l,2))
		addSegments = static_cast<int32_t>(Lua::CheckInt(l,2));
	auto bNoMs = true;
	if(Lua::IsSet(l,3))
		bNoMs = Lua::CheckBool(l,3);
	auto r = ::util::get_pretty_duration(ms,addSegments,bNoMs);
	Lua::PushString(l,r);
	return 1;
}
int Lua::util::is_same_object(lua_State *l)
{
	Lua::PushBool(l,lua_rawequal(l,1,2) == 1);
	return 1;
}
int Lua::util::get_pretty_time(lua_State *l)
{
	auto t = Lua::CheckNumber(l,1);
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
	Lua::PushString(l,prettyTime);
	return 1;
}
int Lua::util::units_to_metres(lua_State *l)
{
	auto units = Lua::CheckNumber(l,1);
	Lua::PushNumber(l,::util::pragma::units_to_metres(units));
	return 1;
}
int Lua::util::metres_to_units(lua_State *l)
{
	auto metres = Lua::CheckNumber(l,1);
	Lua::PushNumber(l,::util::pragma::metres_to_units(metres));
	return 1;
}
int Lua::util::read_scene_file(lua_State *l)
{
	auto fname = "scenes\\" +FileManager::GetCanonicalizedPath(Lua::CheckString(l,1));
	auto f = FileManager::OpenFile(fname.c_str(),"r");
	if(f == nullptr)
		return 0;
	se::SceneScriptValue root {};
	if(se::read_scene(f,root) != ::util::MarkupFile::ResultCode::Ok)
		return 0;
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
	return 1;
}

template<class TProperty,typename TUnderlyingType>
	bool fade_property_generic(Game &game,lua_State *l,const std::function<TUnderlyingType(lua_State*,int32_t)> &fCheck,const std::function<TUnderlyingType(const TUnderlyingType&,const TUnderlyingType&,float)> &fLerp)
{
	if(Lua::IsType<TProperty>(l,1) == false)
		return false;
	auto &vProp = Lua::Check<TProperty>(l,1);
	auto vSrc = vProp.GetValue();
	auto vDst = fCheck(l,2);

	auto duration = Lua::CheckNumber(l,3);
	if(duration == 0.f)
	{
		vProp.SetValue(vDst);
		return  false;
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
	Lua::Push<CallbackHandle>(l,cb);
	return true;
}

template<class TProperty,typename TUnderlyingType>
	bool fade_vector_property_generic(Game &game,lua_State *l,const std::function<TUnderlyingType(const TUnderlyingType&,const TUnderlyingType&,float)> &fLerp)
{
	return fade_property_generic<TProperty,TUnderlyingType>(game,l,Lua::Check<TUnderlyingType>,fLerp);
}

int Lua::util::fade_property(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	if(Lua::IsType<LColorProperty>(l,1))
	{
		auto &colProp = Lua::Check<LColorProperty>(l,1);
		auto &colDst = Lua::Check<Color>(l,2);
		auto hsvSrc = ::util::rgb_to_hsv(*colProp);
		auto hsvDst = ::util::rgb_to_hsv(colDst);
		auto aSrc = (*colProp)->a;
		auto aDst = colDst.a;

		auto duration = Lua::CheckNumber(l,3);
		if(duration == 0.f)
		{
			*colProp = colDst;
			return 0;
		}
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
		Lua::Push<CallbackHandle>(l,cb);
		return 1;
	}
	if(fade_vector_property_generic<LVector2Property,Vector2>(game,l,[](const Vector2 &a,const Vector2 &b,float factor) -> Vector2 {
			return Vector2{umath::lerp(a.x,b.x,factor),umath::lerp(a.y,b.y,factor)};
		}) ||
		fade_vector_property_generic<LVector2iProperty,Vector2i>(game,l,[](const Vector2i &a,const Vector2i &b,float factor) -> Vector2i {
			return Vector2i{static_cast<int32_t>(umath::lerp(a.x,b.x,factor)),static_cast<int32_t>(umath::lerp(a.y,b.y,factor))};
		}) ||
		fade_vector_property_generic<LVector3Property,Vector3>(game,l,uvec::lerp) ||
		fade_vector_property_generic<LVector3iProperty,Vector3i>(game,l,uvec::lerp) ||
		fade_vector_property_generic<LVector4Property,Vector4>(game,l,[](const Vector4 &a,const Vector4 &b,float factor) -> Vector4 {
			return Vector4{umath::lerp(a.x,b.x,factor),umath::lerp(a.y,b.y,factor),umath::lerp(a.z,b.z,factor),umath::lerp(a.w,b.w,factor)};
		}) ||
		fade_vector_property_generic<LVector4iProperty,Vector4i>(game,l,[](const Vector4i &a,const Vector4i &b,float factor) -> Vector4i {
			return Vector4i{static_cast<int32_t>(umath::lerp(a.x,b.x,factor)),static_cast<int32_t>(umath::lerp(a.y,b.y,factor)),static_cast<int32_t>(umath::lerp(a.z,b.z,factor)),static_cast<int32_t>(umath::lerp(a.w,b.w,factor))};
		}) ||
		fade_vector_property_generic<LQuatProperty,Quat>(game,l,uquat::slerp) ||
		fade_vector_property_generic<LEulerAnglesProperty,EulerAngles>(game,l,[](const EulerAngles &a,const EulerAngles &b,float factor) -> EulerAngles {
			return EulerAngles{static_cast<float>(umath::lerp_angle(a.p,b.p,factor)),static_cast<float>(umath::lerp_angle(a.y,b.y,factor)),static_cast<float>(umath::lerp_angle(a.r,b.r,factor))};
		}) ||
		fade_property_generic<LGenericIntPropertyWrapper,int64_t>(game,l,[](lua_State *l,int32_t idx) -> int64_t {return Lua::CheckInt(l,idx);},static_cast<Double(*)(Double,Double,Double)>(umath::lerp)) ||
		fade_property_generic<LGenericFloatPropertyWrapper,float>(game,l,[](lua_State *l,int32_t idx) -> float {return Lua::CheckNumber(l,idx);},static_cast<Double(*)(Double,Double,Double)>(umath::lerp))
	)
		return 1;
	return 0;
}

int Lua::util::round_string(lua_State *l)
{
	auto value = Lua::CheckNumber(l,1);
	auto places = 0;
	if(Lua::IsSet(l,2))
		places = Lua::CheckInt(l,2);
	Lua::PushString(l,::util::round_string(value,places));
	return 1;
}

int Lua::util::get_type_name(lua_State *l)
{
	if(Lua::IsSet(l,1) == false)
	{
		Lua::PushString(l,Lua::IsNone(l,1) ? "none" : "nil");
		return 1;
	}

	auto o = luabind::from_stack(l,1);
	auto classInfo = luabind::get_class_info(o);
	Lua::PushString(l,classInfo.name);
	return 1;
}

int Lua::util::variable_type_to_string(lua_State *l)
{
	auto type = static_cast<::util::VarType>(Lua::CheckInt(l,1));
	Lua::PushString(l,::util::variable_type_to_string(type));
	return 1;
}

int Lua::util::get_string_hash(lua_State *l)
{
	auto *str = Lua::CheckString(l,1);
	Lua::PushString(l,std::to_string(std::hash<std::string>{}(str)));
	return 1;
}

int Lua::util::get_class_value(lua_State *l)
{
	int32_t t = 1;
	Lua::CheckUserData(l,t);
	auto oClass = luabind::object{luabind::from_stack{l,t}};

	std::string key = Lua::CheckString(l,2);
	Lua::PushString(l,key); /* 1 */
	auto r = Lua::GetProtectedTableValue(l,t); /* 2 */
	if(r != Lua::StatusCode::Ok)
	{
		Lua::Pop(l,2); /* 0 */
		return 0;
	}
	// Pop key from stack
	Lua::RemoveValue(l,-2); /* 1 */
	return 1;
}

int Lua::util::pack_zip_archive(lua_State *l)
{
	std::string zipFileName = Lua::CheckString(l,1);
	ufile::remove_extension_from_filename(zipFileName);
	zipFileName += ".zip";
	if(Lua::file::validate_write_operation(l,zipFileName) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}

	int32_t t = 2;
	Lua::CheckTable(l,t);
	std::vector<std::string> files {};
	auto numFiles = Lua::GetObjectLength(l,t);
	files.reserve(numFiles);
	for(auto i=decltype(numFiles){0u};i<numFiles;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		std::string fileName = Lua::CheckString(l,-1);
		files.push_back(fileName);
		Lua::Pop(l,1);
	}

	auto zip = ZIPFile::Open(zipFileName,ZIPFile::OpenFlags::CreateIfNotExist);
	if(zip == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	for(auto &fileName : files)
	{
		auto f = FileManager::OpenFile(fileName.c_str(),"rb");
		if(f == nullptr)
			continue;
		auto sz = f->GetSize();
		std::vector<uint8_t> data {};
		data.resize(sz);
		f->Read(data.data(),sz);
		zip->AddFile(fileName,data.data(),sz);
	}
	zip = nullptr;
	Lua::PushBool(l,true);
	return 1;
}

int Lua::util::get_addon_path(lua_State *l)
{
	auto path = Lua::get_current_file(l);
	ustring::replace(path,"\\","/");
	if(ustring::compare(path.c_str(),"addons/",false,7) == false)
	{
		Lua::PushString(l,"");
		return 1;
	}

	// Get path up to addon directory
	auto br = path.find('/',7);
	if(br != std::string::npos)
		path = path.substr(0,br);
	path += '/';
	Lua::PushString(l,path);
	return 1;
}
#pragma optimize("",on)
