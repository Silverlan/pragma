/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LENTITY_COMPONENTS_BASE_TYPES_HPP__
#define __LENTITY_COMPONENTS_BASE_TYPES_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/model/animation/animation_event.h"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/util_weak_handle.hpp>

extern DLLENGINE Engine *engine;

DLLNETWORK void Lua_TraceData_FillTraceResultTable(lua_State *l,TraceResult &res);
namespace Lua
{
	DLLNETWORK bool get_bullet_master(BaseEntity &ent);
	DLLNETWORK AnimationEvent get_animation_event(lua_State *l,int32_t tArgs,uint32_t eventId);
	namespace Physics
	{
		template<class THandle>
			void InitializePhysics(lua_State *l,THandle &hEnt,::util::TSharedHandle<pragma::physics::IConvexShape> &shape,uint32_t flags)
		{
			pragma::Lua::check_component(l,hEnt);

			auto *phys = hEnt->InitializePhysics(*shape,static_cast<pragma::BasePhysicsComponent::PhysFlags>(flags));
			if(phys != NULL)
				luabind::object(l,phys->GetHandle()).push(l);
		}
		template<class THandle>
			void InitializePhysics(lua_State *l,THandle &hEnt,::util::TSharedHandle<pragma::physics::IConvexShape> &shape)
			{
				InitializePhysics<THandle>(l,hEnt,shape,umath::to_integral(pragma::BasePhysicsComponent::PhysFlags::None));
			}
	};
	namespace Shooter
	{
		template<class THandle>
			void FireBullets(lua_State *l,THandle &hEnt,const luabind::object&,bool bHitReport,bool bMaster)
		{
			pragma::Lua::check_component(l,hEnt);
			auto *bulletInfo = Lua::CheckBulletInfo(l,2);

			std::vector<TraceResult> results;
			hEnt->FireBullets(*bulletInfo,results,bMaster);
			if(bHitReport == false)
				return;
			auto t = Lua::CreateTable(l);
			for(auto i=decltype(results.size()){0};i<results.size();++i)
			{
				auto &r = results[i];
				Lua::PushInt(l,i +1);
				Lua_TraceData_FillTraceResultTable(l,r);

				Lua::SetTableValue(l,t);
			}
		}
	};
	namespace FuncWater
	{
		template<class THandle>
			void GetWaterPlane(lua_State *l,THandle &hEnt,bool bLocalSpace)
		{
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Vector3 n;
			double d;
			if(bLocalSpace == false)
				hEnt->GetWaterPlaneWs(n,d);
			else
				hEnt->GetWaterPlane(n,d);
			Lua::Push<Vector3>(l,n);
			Lua::PushNumber(l,d);
		}

		template<class THandle>
			void CalcLineSurfaceIntersection(lua_State *l,THandle &hEnt,const Vector3 &lineOrigin,const Vector3 &lineDir,bool bCull)
		{
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			double t,u,v;
			auto r = hEnt->CalcLineSurfaceIntersection(lineOrigin,lineDir,&t,&u,&v,bCull);
			Lua::PushBool(l,r);
			Lua::PushNumber(l,t);
			Lua::PushNumber(l,u);
			Lua::PushNumber(l,v);
		}
	};
	namespace SoundEmitter
	{
		template<class THandle>
			void EmitSound(lua_State *l,THandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch)
		{
			pragma::Lua::check_component(l,hEnt);
			std::shared_ptr<ALSound> snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch);
			if(snd == nullptr)
				return;
			luabind::object(l,snd).push(l);
		}
	};
	namespace AI
	{
		template<class THandle>
			void GetMoveSpeed(lua_State *l,THandle &hNPC,uint32_t animId)
		{
			pragma::Lua::check_component(l,hNPC);
			Lua::PushNumber(l,hNPC.get()->GetMoveSpeed(animId));
		}
		template<class THandle>
			void SetLookTarget(lua_State *l,THandle &hNPC,const Vector3 &tgt,float t)
		{
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->SetLookTarget(tgt,t);
		}
		template<class THandle>
			void SetLookTarget(lua_State *l,THandle &hNPC,EntityHandle &hEnt,float t)
		{
			pragma::Lua::check_component(l,hNPC);
			LUA_CHECK_ENTITY(l,hEnt);
			hNPC.get()->SetLookTarget(*hEnt.get(),t);
		}
	};
	namespace Weapon
	{
		template<class THandle>
			void PrimaryAttack(lua_State *l,THandle &hEnt,bool bOnce)
		{
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->PrimaryAttack();
			if(bOnce == true)
				wep->EndPrimaryAttack();
		}
		template<class THandle>
			void SecondaryAttack(lua_State *l,THandle &hEnt,bool bOnce)
		{
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->SecondaryAttack();
			if(bOnce == true)
				wep->EndSecondaryAttack();
		}
	};
	namespace Transform
	{
		template<class THandle>
			void GetDirection(lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetDirection(p,bIgnoreYAxis));
		}
		template<class THandle>
			void GetDirection(lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetDirection(hOther->GetEntity(),bIgnoreYAxis));
		}
		template<class THandle>
			void GetDirection(lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::Push<Vector3>(l,hEnt->GetDirection(*hOther.get(),bIgnoreYAxis));
		}
		template<class THandle>
			void GetAngles(lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(p,bIgnoreYAxis));
		}
		template<class THandle>
			void GetAngles(lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(hOther->GetEntity(),bIgnoreYAxis));
		}
		template<class THandle>
			void GetAngles(lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(*hOther.get(),bIgnoreYAxis));
		}
		template<class THandle>
			void GetDotProduct(lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetDotProduct(p,bIgnoreYAxis));
		}
		template<class THandle>
			void GetDotProduct(lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetDotProduct(hOther->GetEntity(),bIgnoreYAxis));
		}
		template<class THandle>
			void GetDotProduct(lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis)
		{
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::PushNumber(l,hEnt->GetDotProduct(*hOther.get(),bIgnoreYAxis));
		}
	};
	template<class TLuaClass,class THandle>
		void register_base_func_water_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("CreateSplash",static_cast<void(*)(lua_State*,THandle&,const Vector3&,float,float)>([](lua_State *l,THandle &hEnt,const Vector3 &origin,float radius,float force) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->CreateSplash(origin,radius,force);
		}));
		def.def("GetStiffness",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetStiffness());
		}));
		def.def("SetStiffness",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float stiffness) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetStiffness(stiffness);
		}));
		def.def("GetPropagation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetPropagation());
		}));
		def.def("SetPropagation",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float propagation) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetPropagation(propagation);
		}));
		def.def("GetWaterVelocity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Push<Vector3>(l,hEnt->GetWaterVelocity());
		}));
		def.def("SetWaterVelocity",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &velocity) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetWaterVelocity(velocity);
		}));
		def.def("GetDensity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetDensity());
		}));
		def.def("SetDensity",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,double density) {
		if(pragma::Lua::check_component(l,hEnt) == false)
			return;
		hEnt->SetDensity(density);
		}));
		def.def("GetLinearDragCoefficient",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetLinearDragCoefficient());
		}));
		def.def("SetLinearDragCoefficient",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,double coefficient) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetLinearDragCoefficient(coefficient);
		}));
		def.def("GetTorqueDragCoefficient",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetTorqueDragCoefficient());
		}));
		def.def("SetTorqueDragCoefficient",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,double coefficient) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetTorqueDragCoefficient(coefficient);
		}));
		def.def("CalcLineSurfaceIntersection",static_cast<void(*)(lua_State*,THandle&,const Vector3&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &lineOrigin,const Vector3 &lineDir) {
			Lua::FuncWater::CalcLineSurfaceIntersection<THandle>(l,hEnt,lineOrigin,lineDir,false);
		}));
		def.def("CalcLineSurfaceIntersection",static_cast<void(*)(lua_State*,THandle&,const Vector3&,const Vector3&,bool)>([](lua_State *l,THandle &hEnt,const Vector3 &lineOrigin,const Vector3 &lineDir,bool bCull) {
			Lua::FuncWater::CalcLineSurfaceIntersection<THandle>(l,hEnt,lineOrigin,lineDir,bCull);
		}));
		def.def("GetWaterPlane",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool bLocalSpace) {
			Lua::FuncWater::GetWaterPlane<THandle>(l,hEnt,bLocalSpace);
		}));
		def.def("GetWaterPlane",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			Lua::FuncWater::GetWaterPlane<THandle>(l,hEnt,false);
		}));
		def.def("ProjectToSurface",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &pos) {
			auto projectedPos = hEnt->ProjectToSurface(pos);
		}));
		def.add_static_constant("EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED",pragma::BaseFuncWaterComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
	}
	template<class TLuaClass,class THandle>
		void register_base_toggle_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("TurnOn",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->TurnOn();
		}));
		def.def("TurnOff",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->TurnOff();
		}));
		def.def("Toggle",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->Toggle();
		}));
		def.def("IsTurnedOn",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsTurnedOn());
		}));
		def.def("IsTurnedOff",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,!hEnt.get()->IsTurnedOn());
		}));
		def.def("SetTurnedOn",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool b) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetTurnedOn(b);
		}));
		def.def("GetTurnedOnProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetTurnedOnProperty());
		}));
		def.add_static_constant("EVENT_ON_TURN_ON",pragma::BaseToggleComponent::EVENT_ON_TURN_ON);
		def.add_static_constant("EVENT_ON_TURN_OFF",pragma::BaseToggleComponent::EVENT_ON_TURN_OFF);
	}
	template<class TLuaClass,class THandle>
		void register_base_wheel_component_methods(lua_State *l,TLuaClass &def)
	{
		// TODO
	}
	template<class TLuaClass,class THandle>
		void register_base_decal_component_methods(lua_State *l,TLuaClass &def)
	{}
	template<class TLuaClass,class THandle>
		void register_base_env_light_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetFalloffExponent",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetFalloffExponent());
		}));
		def.def("SetFalloffExponent",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float falloffExponent) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetFalloffExponent(falloffExponent);
		}));
		def.def("SetLightIntensityType",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t type) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetLightIntensityType(static_cast<pragma::BaseEnvLightComponent::LightIntensityType>(type));
		}));
		def.def("GetLightIntensityType",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,umath::to_integral(hEnt->GetLightIntensityType()));
		}));
		def.def("SetLightIntensity",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float intensity) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetLightIntensity(intensity);
		}));
		def.def("SetLightIntensity",static_cast<void(*)(lua_State*,THandle&,float,uint32_t)>([](lua_State *l,THandle &hEnt,float intensity,uint32_t type) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetLightIntensity(intensity,static_cast<pragma::BaseEnvLightComponent::LightIntensityType>(type));
		}));
		def.def("GetLightIntensity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetLightIntensity());
		}));
		def.def("GetLightIntensityCandela",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetLightIntensityCandela());
		}));
		def.add_static_constant("INTENSITY_TYPE_CANDELA",umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Candela));
		def.add_static_constant("INTENSITY_TYPE_LUMEN",umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Lumen));
		def.add_static_constant("INTENSITY_TYPE_LUX",umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Lux));
	}
	template<class TLuaClass,class THandle>
		void register_base_env_light_spot_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetOuterCutoffAngle",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float ang) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetOuterCutoffAngle(ang);
		}));
		def.def("GetOuterCutoffAngle",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetOuterCutoffAngle());
		}));
		def.def("SetInnerCutoffAngle",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float ang) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetInnerCutoffAngle(ang);
		}));
		def.def("GetInnerCutoffAngle",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushNumber(l,hEnt->GetInnerCutoffAngle());
		}));
	}
	template<class TLuaClass,class THandle>
		void register_base_env_light_point_component_methods(lua_State *l,TLuaClass &def)
	{}
	template<class TLuaClass,class THandle>
		void register_base_env_light_directional_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetAmbientColor",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Push<Color>(l,hEnt->GetAmbientColor());
		}));
		def.def("GetAmbientColorProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt->GetAmbientColorProperty());
		}));
		def.def("SetAmbientColor",static_cast<void(*)(lua_State*,THandle&,const Color&)>([](lua_State *l,THandle &hEnt,const Color &color) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetAmbientColor(color);
		}));
	}
	template<class TLuaClass,class THandle>
		void register_base_env_particle_system_component_methods(lua_State *l,TLuaClass &def)
	{}
	template<class TLuaClass,class THandle>
		void register_base_flammable_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("Ignite",static_cast<void(*)(lua_State*,THandle&,float,EntityHandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,float duration,EntityHandle &hAttacker,EntityHandle &hInflictor) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hAttacker);
			LUA_CHECK_ENTITY(l,hInflictor);
			hEnt->Ignite(duration,hAttacker.get(),hInflictor.get());
		}));
		def.def("Ignite",static_cast<void(*)(lua_State*,THandle&,float,EntityHandle&)>([](lua_State *l,THandle &hEnt,float duration,EntityHandle &hAttacker) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hAttacker);
			hEnt->Ignite(duration,hAttacker.get());
		}));
		def.def("Ignite",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float duration) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->Ignite(duration);
		}));
		def.def("Ignite",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->Ignite(0.f);
		}));
		def.def("IsOnFire",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->IsOnFire());
		}));
		def.def("IsIgnitable",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->IsIgnitable());
		}));
		def.def("Extinguish",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->Extinguish();
		}));
		def.def("SetIgnitable",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool b) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetIgnitable(b);
		}));
		def.def("GetOnFireProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetOnFireProperty());
		}));
		def.def("GetIgnitableProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetIgnitableProperty());
		}));
		def.add_static_constant("EVENT_ON_IGNITED",pragma::BaseFlammableComponent::EVENT_ON_IGNITED);
		def.add_static_constant("EVENT_ON_EXTINGUISHED",pragma::BaseFlammableComponent::EVENT_ON_EXTINGUISHED);
	}

	template<class TLuaClass,class THandle>
		void register_base_flex_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_health_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetHealth",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			unsigned short health = hEnt->GetHealth();
			Lua::PushInt(l,health);
		}));
		def.def("SetHealth",static_cast<void(*)(lua_State*,THandle&,unsigned short)>([](lua_State *l,THandle &hEnt,unsigned short health) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetHealth(health);
		}));
		def.def("SetMaxHealth",static_cast<void(*)(lua_State*,THandle&,uint16_t)>([](lua_State *l,THandle &hEnt,uint16_t maxHealth) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->SetMaxHealth(maxHealth);
		}));
		def.def("GetMaxHealth",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt.get()->GetMaxHealth());
		}));
		def.def("GetHealthProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetHealthProperty());
		}));
		def.def("GetMaxHealthProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetMaxHealthProperty());
		}));
		def.add_static_constant("EVENT_ON_TAKEN_DAMAGE",pragma::BaseHealthComponent::EVENT_ON_TAKEN_DAMAGE);
		def.add_static_constant("EVENT_ON_HEALTH_CHANGED",pragma::BaseHealthComponent::EVENT_ON_HEALTH_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_name_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetName",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string name) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetName(name);
		}));
		def.def("GetName",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushString(l,hEnt->GetName());
		}));
		def.def("GetNameProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetNameProperty());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_networked_component_methods(lua_State *l,TLuaClass &def)
	{
#if NETWORKED_VARS_ENABLED != 0
		def.def("GetNetVarProperty",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t id) {
			pragma::Lua::check_component(l,hEnt);
			auto &prop = hEnt->GetNetworkedVariableProperty(id);
			if(prop == nullptr)
				return;
			Lua::Property::push(l,*prop);
		}));
		def.def("CreateNetVar",static_cast<void(*)(lua_State*,THandle&,const std::string&,uint32_t)>([](lua_State *l,THandle &hEnt,const std::string &name,uint32_t type) {
			pragma::Lua::check_component(l,hEnt);
			auto id = hEnt->CreateNetworkedVariable(name,static_cast<pragma::BaseNetworkedComponent::NetworkedVariable::Type>(type));
			Lua::PushInt(l,id);
		}));
		def.def("SetNetVarValue",static_cast<void(*)(lua_State*,THandle&,uint32_t,luabind::object)>([](lua_State *l,THandle &hEnt,uint32_t id,luabind::object value) {
			pragma::Lua::check_component(l,hEnt);
			auto type = hEnt->GetNetworkedVariableType(id);
			if(type == pragma::BaseNetworkedComponent::NetworkedVariable::Type::Invalid)
				return;
			switch(type)
			{
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool:
				{
					auto v = Lua::CheckBool(l,3);
					hEnt->SetNetworkedVariable<bool>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double:
				{
					auto v = Lua::CheckNumber(l,3);
					hEnt->SetNetworkedVariable<double>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float:
				{
					auto v = Lua::CheckNumber(l,3);
					hEnt->SetNetworkedVariable<float>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<int8_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<int16_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<int32_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<int64_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
				{
					auto v = Lua::CheckNumber(l,3);
					hEnt->SetNetworkedVariable<long double>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::String:
				{
					std::string v = Lua::CheckString(l,3);
					hEnt->SetNetworkedVariable<std::string>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<uint8_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<uint16_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<uint32_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
				{
					auto v = Lua::CheckInt(l,3);
					hEnt->SetNetworkedVariable<uint64_t>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
				{
					auto &v = *Lua::CheckEulerAngles(l,3);
					hEnt->SetNetworkedVariable<EulerAngles>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color:
				{
					auto &v = *Lua::CheckColor(l,3);
					hEnt->SetNetworkedVariable<Color>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
				{
					auto &v = *Lua::CheckVector(l,3);
					hEnt->SetNetworkedVariable<Vector3>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
				{
					auto &v = *Lua::CheckVector2(l,3);
					hEnt->SetNetworkedVariable<Vector2>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
				{
					auto &v = *Lua::CheckVector4(l,3);
					hEnt->SetNetworkedVariable<Vector4>(id,v);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity:
				{
					auto *v = Lua::CheckEntity(l,3);
					auto h = (v != nullptr) ? v->GetHandle() : EntityHandle{};
					hEnt->SetNetworkedVariable<EntityHandle>(id,h);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion:
				{
					auto &v = *Lua::CheckQuaternion(l,3);
					hEnt->SetNetworkedVariable<Quat>(id,v);
					return;
				}
			}
		}));
		def.def("GetNetVarValue",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t id) {
			pragma::Lua::check_component(l,hEnt);
			auto type = hEnt->GetNetworkedVariableType(id);
			auto &prop = hEnt->GetNetworkedVariableProperty(id);
			if(type == pragma::BaseNetworkedComponent::NetworkedVariable::Type::Invalid || prop == nullptr)
				return;
			switch(type)
			{
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool:
					Lua::PushBool(l,static_cast<::util::BoolProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double:
					Lua::PushNumber(l,static_cast<::util::DoubleProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float:
					Lua::PushNumber(l,static_cast<::util::FloatProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8:
					Lua::PushInt(l,static_cast<::util::Int8Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16:
					Lua::PushInt(l,static_cast<::util::Int16Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32:
					Lua::PushInt(l,static_cast<::util::Int32Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64:
					Lua::PushInt(l,static_cast<::util::Int64Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
					Lua::PushNumber(l,static_cast<::util::LongDoubleProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::String:
					Lua::PushString(l,static_cast<::util::StringProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
					Lua::PushInt(l,static_cast<::util::UInt8Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
					Lua::PushInt(l,static_cast<::util::UInt16Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
					Lua::PushInt(l,static_cast<::util::UInt32Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
					Lua::PushInt(l,static_cast<::util::UInt64Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
					Lua::Push<EulerAngles>(l,static_cast<::util::EulerAnglesProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color:
					Lua::Push<Color>(l,static_cast<::util::ColorProperty&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
					Lua::Push<Vector3>(l,static_cast<::util::Vector3Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
					Lua::Push<Vector2>(l,static_cast<::util::Vector2Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
					Lua::Push<Vector4>(l,static_cast<::util::Vector4Property&>(*prop).GetValue());
					return;
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity:
				{
					auto &hEnt = static_cast<::pragma::EntityProperty&>(*prop).GetValue();
					if(hEnt.IsValid())
						return;
					hEnt->GetLuaObject()->push(l);
					return;
				}
				case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion:
					Lua::Push<Quat>(l,static_cast<::util::QuatProperty&>(*prop).GetValue());
					return;
			}
		}));
		def.add_static_constant("NET_VAR_TYPE_BOOL",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool));
		def.add_static_constant("NET_VAR_TYPE_DOUBLE",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double));
		def.add_static_constant("NET_VAR_TYPE_FLOAT",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float));
		def.add_static_constant("NET_VAR_TYPE_INT8",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8));
		def.add_static_constant("NET_VAR_TYPE_INT16",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16));
		def.add_static_constant("NET_VAR_TYPE_INT32",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32));
		def.add_static_constant("NET_VAR_TYPE_INT64",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64));
		def.add_static_constant("NET_VAR_TYPE_LONG_DOUBLE",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble));
		def.add_static_constant("NET_VAR_TYPE_STRING",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::String));
		def.add_static_constant("NET_VAR_TYPE_UINT8",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8));
		def.add_static_constant("NET_VAR_TYPE_UINT16",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16));
		def.add_static_constant("NET_VAR_TYPE_UINT32",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32));
		def.add_static_constant("NET_VAR_TYPE_UINT64",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64));
		def.add_static_constant("NET_VAR_TYPE_EULER_ANGLES",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles));
		def.add_static_constant("NET_VAR_TYPE_COLOR",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color));
		def.add_static_constant("NET_VAR_TYPE_VECTOR",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector));
		def.add_static_constant("NET_VAR_TYPE_VECTOR2",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2));
		def.add_static_constant("NET_VAR_TYPE_VECTOR4",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4));
		def.add_static_constant("NET_VAR_TYPE_ENTITY",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity));
		def.add_static_constant("NET_VAR_TYPE_QUATERNION",umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion));
#endif
	}

	template<class TLuaClass,class THandle>
		void register_base_observable_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetLocalCameraOrigin",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t camType,const Vector3 &origin) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalCameraOrigin(static_cast<pragma::BaseObservableComponent::CameraType>(camType),origin);
		}));
		def.def("GetLocalCameraOrigin",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t camType) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetLocalCameraOrigin(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
		}));
		def.def("SetLocalCameraOffset",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t camType,const Vector3 &offset) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalCameraOffset(static_cast<pragma::BaseObservableComponent::CameraType>(camType),offset);
			}));
		def.def("GetLocalCameraOffset",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t camType) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetLocalCameraOffset(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
		}));
		def.def("GetCameraData",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t camType) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<pragma::ObserverCameraData*>(l,&hEnt->GetCameraData(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
		}));
		def.def("SetCameraEnabled",static_cast<void(*)(lua_State*,THandle&,uint32_t,bool)>([](lua_State *l,THandle &hEnt,uint32_t camType,bool enabled) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCameraEnabled(static_cast<pragma::BaseObservableComponent::CameraType>(camType),enabled);
		}));
		def.def("GetCameraEnabledProperty",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t camType) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Property::push(l,*hEnt->GetCameraEnabledProperty(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
		}));
		def.def("GetCameraOffsetProperty",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t camType) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Property::push(l,*hEnt->GetCameraOffsetProperty(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
		}));
		def.add_static_constant("CAMERA_TYPE_FIRST_PERSON",umath::to_integral(pragma::BaseObservableComponent::CameraType::FirstPerson));
		def.add_static_constant("CAMERA_TYPE_THIRD_PERSON",umath::to_integral(pragma::BaseObservableComponent::CameraType::ThirdPerson));

		auto defObsCamData = luabind::class_<pragma::ObserverCameraData>("CameraData");
		defObsCamData.def_readwrite("rotateWithObservee",&pragma::ObserverCameraData::rotateWithObservee);
		defObsCamData.def("SetAngleLimits",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&,const EulerAngles&,const EulerAngles&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData,const EulerAngles &min,const EulerAngles &max) {
			obsCamData.angleLimits = {min,max};
		}));
		defObsCamData.def("GetAngleLimits",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData) {
			if(obsCamData.angleLimits.has_value() == false)
				return;
			Lua::Push<EulerAngles>(l,obsCamData.angleLimits->first);
			Lua::Push<EulerAngles>(l,obsCamData.angleLimits->second);
		}));
		defObsCamData.def("ClearAngleLimits",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData) {
			obsCamData.angleLimits = {};
		}));
		defObsCamData.property("enabled",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData) {
			Lua::PushBool(l,*obsCamData.enabled);
		}),static_cast<void(*)(lua_State*,pragma::ObserverCameraData&,bool)>([](lua_State *l,pragma::ObserverCameraData &obsCamData,bool enabled) {
			*obsCamData.enabled = enabled;
		}));
		defObsCamData.property("localOrigin",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData) {
			if(obsCamData.localOrigin.has_value() == false)
				return;
			Lua::Push<Vector3>(l,*obsCamData.localOrigin);
		}),static_cast<void(*)(lua_State*,pragma::ObserverCameraData&,const Vector3&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData,const Vector3 &origin) {
			obsCamData.localOrigin = origin;
		}));
		defObsCamData.property("offset",static_cast<void(*)(lua_State*,pragma::ObserverCameraData&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData) {
			Lua::Push<Vector3>(l,*obsCamData.offset);
		}),static_cast<void(*)(lua_State*,pragma::ObserverCameraData&,const Vector3&)>([](lua_State *l,pragma::ObserverCameraData &obsCamData,const Vector3 &offset) {
			*obsCamData.offset = offset;
		}));
		def.scope[defObsCamData];
	}

	template<class TLuaClass,class THandle>
		void register_base_shooter_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("FireBullets",static_cast<void(*)(lua_State*,THandle&,const luabind::object&,bool,bool)>([](lua_State *l,THandle &hEnt,const luabind::object &o,bool bHitReport,bool bMaster) {
			Lua::Shooter::FireBullets(l,hEnt,o,bHitReport,bMaster);
		}));
		def.def("FireBullets",static_cast<void(*)(lua_State*,THandle&,const luabind::object&,bool)>([](lua_State *l,THandle &hEnt,const luabind::object &o,bool bHitReport) {
			Lua::Shooter::FireBullets(l,hEnt,o,bHitReport,get_bullet_master(hEnt->GetEntity()));
		}));
		def.def("FireBullets",static_cast<void(*)(lua_State*,THandle&,const luabind::object&)>([](lua_State *l,THandle &hEnt,const luabind::object &o) {
			Lua::Shooter::FireBullets(l,hEnt,o,false,get_bullet_master(hEnt->GetEntity()));
		}));
		def.add_static_constant("EVENT_ON_FIRE_BULLETS",pragma::BaseShooterComponent::EVENT_ON_FIRE_BULLETS);
	}

	template<class TLuaClass,class THandle>
		void register_base_physics_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetCollisionBounds",static_cast<void(*)(lua_State*,THandle&,Vector3,Vector3)>([](lua_State *l,THandle &hEnt,Vector3 min,Vector3 max) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionBounds(min,max);
		}));
		def.def("GetCollisionBounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 min,max;
			hEnt->GetCollisionBounds(&min,&max);
			luabind::object(l,min).push(l);
			luabind::object(l,max).push(l);
		}));
		def.def("GetCollisionExtents",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetCollisionExtents()).push(l);
		}));
		def.def("GetCollisionCenter",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetCollisionCenter()).push(l);
		}));
		def.def("GetMoveType",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			MOVETYPE mt = hEnt->GetMoveType();
			Lua::PushInt(l,int(mt));
		}));
		def.def("SetMoveType",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hEnt,int moveType) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetMoveType(MOVETYPE(moveType));
		}));
		def.def("GetPhysicsObject",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			PhysObj *phys = hEnt->GetPhysicsObject();
			if(phys == NULL)
				return;
			luabind::object(l,phys->GetHandle()).push(l);
		}));
		def.def("InitializePhysics",static_cast<void(*)(lua_State*,THandle&,uint32_t,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t type,uint32_t physFlags) {
			pragma::Lua::check_component(l,hEnt);

			PhysObj *phys = hEnt->InitializePhysics(PHYSICSTYPE(type),static_cast<pragma::BasePhysicsComponent::PhysFlags>(physFlags));
			if(phys != NULL)
				luabind::object(l,phys->GetHandle()).push(l);
		}));
		def.def("InitializePhysics",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t type) {
			pragma::Lua::check_component(l,hEnt);
			
			PhysObj *phys = hEnt->InitializePhysics(PHYSICSTYPE(type));
			if(phys != NULL)
				luabind::object(l,phys->GetHandle()).push(l);
		}));
		def.def("InitializePhysics",static_cast<void(*)(lua_State*,THandle&,::util::TSharedHandle<pragma::physics::IConvexShape>&,uint32_t)>([](lua_State *l,THandle &hEnt,::util::TSharedHandle<pragma::physics::IConvexShape> &shape,uint32_t physFlags) {
			Lua::Physics::InitializePhysics<THandle>(l,hEnt,shape,physFlags);
		}));
		def.def("InitializePhysics",static_cast<void(*)(lua_State*,THandle&,::util::TSharedHandle<pragma::physics::IConvexShape>&,uint32_t)>([](lua_State *l,THandle &hEnt,::util::TSharedHandle<pragma::physics::IConvexShape> &shape,uint32_t physFlags) {
			Lua::Physics::InitializePhysics<THandle>(l,hEnt,shape,physFlags);
		}));
		def.def("InitializePhysics",static_cast<void(*)(lua_State*,THandle&,::util::TSharedHandle<pragma::physics::IConvexShape>&)>([](lua_State *l,THandle &hEnt,::util::TSharedHandle<pragma::physics::IConvexShape> &shape) {
			Lua::Physics::InitializePhysics<THandle>(l,hEnt,shape,0);
		}));
		def.def("DestroyPhysicsObject",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->DestroyPhysicsObject();
		}));
		def.def("DropToFloor",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->DropToFloor();
		}));
		def.def("IsTrigger",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->IsTrigger());
		}));
		def.def("SetKinematic",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool b) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetKinematic(b);
		}));
		def.def("IsKinematic",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->IsKinematic());
		}));

		def.def("GetCollisionCallbacksEnabled",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->GetCollisionCallbacksEnabled());
		}));
		def.def("SetCollisionCallbacksEnabled",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionCallbacksEnabled(true);
		}));
		def.def("GetCollisionContactReportEnabled",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->GetCollisionContactReportEnabled());
		}));
		def.def("SetCollisionContactReportEnabled",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionContactReportEnabled(true);
		}));
		def.def("SetCollisionFilterMask",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hEnt,int mask) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionFilterMask(static_cast<CollisionMask>(mask));
		}));
		def.def("GetCollisionFilterMask",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->GetCollisionFilterMask());
		}));
		def.def("SetCollisionFilterGroup",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hEnt,int group) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionFilterGroup(static_cast<CollisionMask>(group));
		}));
		def.def("GetCollisionFilterGroup",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->GetCollisionFilter());
		}));
		def.def("SetCollisionFilter",static_cast<void(*)(lua_State*,THandle&,int,int)>([](lua_State *l,THandle &hEnt,int group,int mask) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCollisionFilter(static_cast<CollisionMask>(group),static_cast<CollisionMask>(mask));
		}));
		def.def("EnableCollisions",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hEntOther) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hEntOther);
			hEnt->EnableCollisions(&hEntOther->GetEntity());
		}));
		def.def("DisableCollisions",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hEntOther) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hEntOther);
			hEnt->DisableCollisions(&hEntOther->GetEntity());
		}));
		def.def("SetCollisionsEnabled",static_cast<void(*)(lua_State*,THandle&,THandle&,bool)>([](lua_State *l,THandle &hEnt,THandle &hEntOther,bool b) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hEntOther);
			hEnt->SetCollisionsEnabled(&hEntOther->GetEntity(),b);
		}));
		def.def("ResetCollisions",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hEntOther) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hEntOther);
			hEnt->ResetCollisions(&hEntOther->GetEntity());
		}));

		def.def("GetPhysJointConstraints",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto &joints = hEnt->GetPhysConstraints();
			auto table = Lua::CreateTable(l); /* 1 */
			auto n = 1;
			for(auto it=joints.begin();it!=joints.end();++it)
			{
				auto &joint = *it;
				if(joint.constraint.IsValid())
				{
					auto tConstraint = Lua::CreateTable(l); /* 2 */

					Lua::PushString(l,"source"); /* 3 */
					Lua::PushInt(l,joint.source); /* 4 */
					Lua::SetTableValue(l,tConstraint); /* 2 */

					Lua::PushString(l,"target"); /* 3 */
					Lua::PushInt(l,joint.target); /* 4 */
					Lua::SetTableValue(l,tConstraint); /* 2 */

					Lua::PushString(l,"constraint"); /* 3 */
					joint.constraint->Push(l); /* 4 */
					Lua::SetTableValue(l,tConstraint); /* 2 */

					Lua::PushInt(l,n); /* 3 */
					Lua::PushValue(l,tConstraint); /* 4 */
					Lua::SetTableValue(l,table); /* 2 */
					Lua::Pop(l,1); /* 1 */
					n++;
				}
			}
		}));
		def.def("GetRotatedCollisionBounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 min,max;
			hEnt->GetRotatedCollisionBounds(&min,&max);
			Lua::Push<Vector3>(l,min);
			Lua::Push<Vector3>(l,max);
		}));

		def.def("GetPhysicsType",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto type = hEnt->GetPhysicsType();
			Lua::Push<std::underlying_type_t<decltype(type)>>(l,umath::to_integral(type));
		}));
		def.def("GetCollisionRadius",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetCollisionRadius());
		}));
		def.def("IsPhysicsProp",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto physType = hEnt->GetPhysicsType();
			Lua::PushBool(l,(physType != PHYSICSTYPE::NONE && physType != PHYSICSTYPE::STATIC && physType != PHYSICSTYPE::BOXCONTROLLER && physType != PHYSICSTYPE::CAPSULECONTROLLER) ? true : false);
		}));

		def.def("GetAABBDistance",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &p) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetAABBDistance(p));
		}));
		def.def("GetAABBDistance",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hOther) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hOther);
			Lua::PushNumber(l,hEnt->GetAABBDistance(hOther->GetEntity()));
		}));
		def.def("GetAABBDistance",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::PushNumber(l,hEnt->GetAABBDistance(*hOther.get()));
		}));

		def.def("IsRagdoll",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->IsRagdoll());
		}));

		def.def("ApplyForce",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &force) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyForce(force);
		}));
		def.def("ApplyForce",static_cast<void(*)(lua_State*,THandle&,const Vector3&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &force,const Vector3 &relPos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyForce(force,relPos);
		}));
		def.def("ApplyImpulse",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &impulse) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyImpulse(impulse);
		}));
		def.def("ApplyImpulse",static_cast<void(*)(lua_State*,THandle&,const Vector3&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &impulse,const Vector3 &relPos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyImpulse(impulse,relPos);
		}));
		def.def("ApplyTorque",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &torque) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyTorque(torque);
		}));
		def.def("ApplyTorqueImpulse",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &torque) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ApplyTorqueImpulse(torque);
		}));
		def.def("GetMass",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetMass());
		}));
		def.add_static_constant("EVENT_ON_PHYSICS_INITIALIZED",pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED);
		def.add_static_constant("EVENT_ON_PHYSICS_DESTROYED",pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED);
		def.add_static_constant("EVENT_ON_PHYSICS_UPDATED",pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED);
		def.add_static_constant("EVENT_ON_DYNAMIC_PHYSICS_UPDATED",pragma::BasePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED);
		def.add_static_constant("EVENT_ON_PRE_PHYSICS_SIMULATE",pragma::BasePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE);
		def.add_static_constant("EVENT_ON_POST_PHYSICS_SIMULATE",pragma::BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE);
		def.add_static_constant("EVENT_ON_WAKE",pragma::BasePhysicsComponent::EVENT_ON_WAKE);
		def.add_static_constant("EVENT_ON_SLEEP",pragma::BasePhysicsComponent::EVENT_ON_SLEEP);
		def.add_static_constant("EVENT_HANDLE_RAYCAST",pragma::BasePhysicsComponent::EVENT_HANDLE_RAYCAST);
		def.add_static_constant("EVENT_INITIALIZE_PHYSICS",pragma::BasePhysicsComponent::EVENT_INITIALIZE_PHYSICS);

		def.add_static_constant("MOVETYPE_NONE",umath::to_integral(MOVETYPE::NONE));
		def.add_static_constant("MOVETYPE_WALK",umath::to_integral(MOVETYPE::WALK));
		def.add_static_constant("MOVETYPE_NOCLIP",umath::to_integral(MOVETYPE::NOCLIP));
		def.add_static_constant("MOVETYPE_FLY",umath::to_integral(MOVETYPE::FLY));
		def.add_static_constant("MOVETYPE_FREE",umath::to_integral(MOVETYPE::FREE));
		def.add_static_constant("MOVETYPE_PHYSICS",umath::to_integral(MOVETYPE::PHYSICS));
	}

	template<class TLuaClass,class THandle>
		void register_base_render_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetCastShadows",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool b) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCastShadows(b);
		}));
		def.def("GetCastShadows",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt->GetCastShadows());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_soft_body_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_sound_emitter_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("CreateSound",static_cast<void(*)(lua_State*,THandle&,std::string,uint32_t)>([](lua_State *l,THandle &hEnt,std::string sndname,uint32_t soundType) {
			pragma::Lua::check_component(l,hEnt);
			std::shared_ptr<ALSound> snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType));
			if(snd == nullptr)
				return;
			luabind::object(l,snd).push(l);
		}));
		def.def("EmitSound",static_cast<void(*)(lua_State*,THandle&,std::string,uint32_t,float,float)>([](lua_State *l,THandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch) {
			Lua::SoundEmitter::EmitSound(l,hEnt,sndname,soundType,gain,pitch);
		}));
		def.def("EmitSound",static_cast<void(*)(lua_State*,THandle&,std::string,uint32_t,float)>([](lua_State *l,THandle &hEnt,std::string sndname,uint32_t soundType,float gain) {
			Lua::SoundEmitter::EmitSound(l,hEnt,sndname,soundType,gain,1.f);
		}));
		def.def("EmitSound",static_cast<void(*)(lua_State*,THandle&,std::string,uint32_t)>([](lua_State *l,THandle &hEnt,std::string sndname,uint32_t soundType) {
			Lua::SoundEmitter::EmitSound(l,hEnt,sndname,soundType,1.f,1.f);
		}));
		def.def("StopSounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->StopSounds();
		}));
		def.def("GetSounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			std::vector<std::shared_ptr<ALSound>> *sounds;
			hEnt->GetSounds(&sounds);
			lua_newtable(l);
			int top = lua_gettop(l);
			for(int i=0;i<sounds->size();i++)
			{
				luabind::object(l,(*sounds)[i]).push(l);
				lua_rawseti(l,top,i +1);
			}
		}));
		def.add_static_constant("EVENT_ON_SOUND_CREATED",pragma::BaseSoundEmitterComponent::EVENT_ON_SOUND_CREATED);
	}

	template<class TLuaClass,class THandle>
		void register_base_transform_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetOrigin",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetOrigin()).push(l);
		}));
		def.def("GetPos",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetPosition()).push(l);
		}));
		def.def("SetPos",static_cast<void(*)(lua_State*,THandle&,Vector3)>([](lua_State *l,THandle &hEnt,Vector3 pos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetPosition(pos);
		}));
		def.def("GetRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetOrientation()).push(l);
		}));
		def.def("SetRotation",static_cast<void(*)(lua_State*,THandle&,Quat)>([](lua_State *l,THandle &hEnt,Quat q) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetOrientation(q);
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			EulerAngles ang = hEnt->GetAngles();
			luabind::object(l,ang).push(l);
		}));
		def.def("SetAngles",static_cast<void(*)(lua_State*,THandle&,EulerAngles)>([](lua_State *l,THandle &hEnt,EulerAngles ang) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetAngles(ang);
		}));
		def.def("GetForward",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetForward()).push(l);
		}));
		def.def("GetRight",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetRight()).push(l);
		}));
		def.def("GetUp",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			luabind::object(l,hEnt->GetUp()).push(l);
		}));

		def.def("GetEyePos",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetEyePosition());
		}));
		def.def("GetEyeOffset",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetEyeOffset());
		}));
		def.def("SetEyeOffset",static_cast<void(*)(lua_State*,THandle&,Vector3&)>([](lua_State *l,THandle &hEnt,Vector3 &offset) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetEyeOffset(offset);
		}));

		def.def("LocalToWorld",static_cast<void(*)(lua_State*,THandle&,Vector3)>([](lua_State *l,THandle &hEnt,Vector3 origin) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->LocalToWorld(&origin);
			Lua::Push<Vector3>(l,origin);
		}));
		def.def("LocalToWorld",static_cast<void(*)(lua_State*,THandle&,Quat)>([](lua_State *l,THandle &hEnt,Quat rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->LocalToWorld(&rot);
			Lua::Push<Quat>(l,rot);
		}));
		def.def("LocalToWorld",static_cast<void(*)(lua_State*,THandle&,Vector3,Quat)>([](lua_State *l,THandle &hEnt,Vector3 origin,Quat rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->LocalToWorld(&origin,&rot);
			Lua::Push<Vector3>(l,origin);
			Lua::Push<Quat>(l,rot);
		}));
		def.def("WorldToLocal",static_cast<void(*)(lua_State*,THandle&,Vector3)>([](lua_State *l,THandle &hEnt,Vector3 origin) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->WorldToLocal(&origin);
			Lua::Push<Vector3>(l,origin);
		}));
		def.def("WorldToLocal",static_cast<void(*)(lua_State*,THandle&,Quat)>([](lua_State *l,THandle &hEnt,Quat rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->WorldToLocal(&rot);
			Lua::Push<Quat>(l,rot);
		}));
		def.def("WorldToLocal",static_cast<void(*)(lua_State*,THandle&,Vector3,Quat)>([](lua_State *l,THandle &hEnt,Vector3 origin,Quat rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->WorldToLocal(&origin,&rot);
			Lua::Push<Vector3>(l,origin);
			Lua::Push<Quat>(l,rot);
		}));

		def.def("GetPitch",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetPitch());
		}));
		def.def("GetYaw",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetYaw());
		}));
		def.def("GetRoll",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetRoll());
		}));
		def.def("SetPitch",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float pitch) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetPitch(pitch);
		}));
		def.def("SetYaw",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float yaw) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetYaw(yaw);
		}));
		def.def("SetRoll",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float roll) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetRoll(roll);
		}));

		def.def("GetLastMoveTime",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetLastMoveTime());
		}));
		def.def("GetScale",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetScale());
		}));
		def.def("GetMaxAxisScale",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetMaxAxisScale());
		}));
		def.def("GetAbsMaxAxisScale",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetAbsMaxAxisScale());
		}));
		def.def("SetScale",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetScale(scale);
		}));
		def.def("SetScale",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetScale(Vector3{scale,scale,scale});
		}));

		def.def("GetDistance",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &p) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetDistance(p));
		}));
		def.def("GetDistance",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hOther) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hOther);
			Lua::PushNumber(l,hEnt->GetDistance(hOther->GetEntity()));
		}));
		def.def("GetDistance",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::PushNumber(l,hEnt->GetDistance(*hOther.get()));
		}));

		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,const Vector3&,bool)>([](lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt->GetDirection(p,bIgnoreYAxis));
		}));
		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,THandle&,bool)>([](lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hOther);
			Lua::Push<Vector3>(l,hEnt->GetDirection(hOther->GetEntity(),bIgnoreYAxis));
		}));
		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,bool)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::Push<Vector3>(l,hEnt->GetDirection(*hOther.get(),bIgnoreYAxis));
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,const Vector3&,bool)>([](lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(p,bIgnoreYAxis));
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,THandle&,bool)>([](lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hOther);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(hOther->GetEntity(),bIgnoreYAxis));
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,bool)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::Push<EulerAngles>(l,hEnt->GetAngles(*hOther.get(),bIgnoreYAxis));
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,const Vector3&,bool)>([](lua_State *l,THandle &hEnt,const Vector3 &p,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetDotProduct(p,bIgnoreYAxis));
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,THandle&,bool)>([](lua_State *l,THandle &hEnt,THandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,hOther);
			Lua::PushNumber(l,hEnt->GetDotProduct(hOther->GetEntity(),bIgnoreYAxis));
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,bool)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther,bool bIgnoreYAxis) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hOther);
			Lua::PushNumber(l,hEnt->GetDotProduct(*hOther.get(),bIgnoreYAxis));
		}));

		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &p) {
			Lua::Transform::GetDirection<THandle>(l,hEnt,p,false);
		}));
		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hOther) {
			Lua::Transform::GetDirection<THandle>(l,hEnt,hOther,false);
		}));
		def.def("GetDirection",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther) {
			Lua::Transform::GetDirection<THandle>(l,hEnt,hOther,false);
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &p) {
			Lua::Transform::GetAngles<THandle>(l,hEnt,p,false);
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hOther) {
			Lua::Transform::GetAngles<THandle>(l,hEnt,hOther,false);
		}));
		def.def("GetAngles",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther) {
			Lua::Transform::GetAngles<THandle>(l,hEnt,hOther,false);
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &p) {
			Lua::Transform::GetDotProduct<THandle>(l,hEnt,p,false);
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,THandle&)>([](lua_State *l,THandle &hEnt,THandle &hOther) {
			Lua::Transform::GetDotProduct<THandle>(l,hEnt,hOther,false);
		}));
		def.def("GetDotProduct",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOther) {
			Lua::Transform::GetDotProduct<THandle>(l,hEnt,hOther,false);
		}));

		def.def("GetPosProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetPosProperty());
		}));
		def.def("GetRotationProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetOrientationProperty());
		}));
		def.def("GetScaleProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetScaleProperty());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_color_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetColorProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetColorProperty());
		}));
		def.def("GetColor",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Color>(l,hComponent->GetColor());
		}));
		def.def("SetColor",static_cast<void(*)(lua_State*,THandle&,const Color&)>([](lua_State *l,THandle &hComponent,const Color &color) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetColor(color);
		}));
		def.def("SetColor",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hComponent,const Vector3 &color) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetColor(color);
		}));
		def.def("SetColor",static_cast<void(*)(lua_State*,THandle&,const Vector4&)>([](lua_State *l,THandle &hComponent,const Vector4 &color) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetColor(color);
		}));
		def.add_static_constant("EVENT_ON_COLOR_CHANGED",pragma::BaseColorComponent::EVENT_ON_COLOR_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_score_component_methods(lua_State *l,TLuaClass &def)
		{
			def.def("GetScoreProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
				pragma::Lua::check_component(l,hComponent);
				Lua::Property::push(l,*hComponent->GetScoreProperty());
			}));
			def.def("GetScore",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
				pragma::Lua::check_component(l,hComponent);
				Lua::Push<pragma::BaseScoreComponent::Score>(l,hComponent->GetScore());
			}));
			def.def("SetScore",static_cast<void(*)(lua_State*,THandle&,pragma::BaseScoreComponent::Score)>([](lua_State *l,THandle &hComponent,pragma::BaseScoreComponent::Score score) {
				pragma::Lua::check_component(l,hComponent);
				hComponent->SetScore(score);
			}));
			def.def("AddScore",static_cast<void(*)(lua_State*,THandle&,pragma::BaseScoreComponent::Score)>([](lua_State *l,THandle &hComponent,pragma::BaseScoreComponent::Score score) {
				pragma::Lua::check_component(l,hComponent);
				hComponent->AddScore(score);
			}));
			def.def("SubtractScore",static_cast<void(*)(lua_State*,THandle&,pragma::BaseScoreComponent::Score)>([](lua_State *l,THandle &hComponent,pragma::BaseScoreComponent::Score score) {
				pragma::Lua::check_component(l,hComponent);
				hComponent->SubtractScore(score);
			}));
			def.add_static_constant("EVENT_ON_SCORE_CHANGED",pragma::BaseScoreComponent::EVENT_ON_SCORE_CHANGED);
		}

	template<class TLuaClass,class THandle>
		void register_base_radius_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetRadiusProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetRadiusProperty());
		}));
		def.def("GetRadius",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetRadius());
		}));
		def.def("SetRadius",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float radius) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetRadius(radius);
		}));
		def.add_static_constant("EVENT_ON_RADIUS_CHANGED",pragma::BaseRadiusComponent::EVENT_ON_RADIUS_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_env_sound_dsp_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_camera_component_methods(lua_State *l,TLuaClass &def)
	{
		def.add_static_constant("DEFAULT_NEAR_Z",pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z);
		def.add_static_constant("DEFAULT_FAR_Z",pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z);
		def.add_static_constant("DEFAULT_FOV",pragma::BaseEnvCameraComponent::DEFAULT_FOV);
		def.add_static_constant("DEFAULT_VIEWMODEL_FOV",pragma::BaseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV);
		def.def("GetProjectionMatrix",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Mat4>(l,hComponent->GetProjectionMatrix());
		}));
		def.def("GetViewMatrix",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Mat4>(l,hComponent->GetViewMatrix());
		}));
		def.def("LookAt",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hComponent,const Vector3 &lookAtPos) {
			pragma::Lua::check_component(l,hComponent);
			auto &trComponent = hComponent->GetEntity().GetTransformComponent();
			if(trComponent.expired())
				return;
			auto &camPos = trComponent->GetPosition();
			auto dir = lookAtPos -camPos;
			uvec::normalize(&dir);
			trComponent->SetOrientation(uquat::create_look_rotation(dir,trComponent->GetUp()));
		}));
		def.def("UpdateMatrices",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->UpdateMatrices();
		}));
		def.def("UpdateViewMatrix",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->UpdateViewMatrix();
		}));
		def.def("UpdateProjectionMatrix",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->UpdateProjectionMatrix();
		}));
		def.def("SetFOV",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float fov) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetFOV(fov);
		}));
		def.def("GetProjectionMatrixProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetProjectionMatrixProperty());
		}));
		def.def("GetViewMatrixProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetViewMatrixProperty());
		}));
		def.def("GetNearZProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetNearZProperty());
		}));
		def.def("GetFarZProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetFarZProperty());
		}));
		def.def("GetFOVProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetFOVProperty());
		}));
		def.def("GetAspectRatioProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetAspectRatioProperty());
		}));
		def.def("SetAspectRatio",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float aspectRatio) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetAspectRatio(aspectRatio);
		}));
		def.def("SetNearZ",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float nearZ) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetNearZ(nearZ);
		}));
		def.def("SetFarZ",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float farZ) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetFarZ(farZ);
		}));
		def.def("GetFOV",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetFOV());
		}));
		def.def("GetFOVRad",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetFOVRad());
		}));
		def.def("GetAspectRatio",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetAspectRatio());
		}));
		def.def("GetNearZ",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetNearZ());
		}));
		def.def("GetFarZ",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetFarZ());
		}));
		def.def("GetFrustumPlanes",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			std::vector<Plane> planes;
			hComponent->GetFrustumPlanes(planes);

			lua_newtable(l);
			int top = lua_gettop(l);
			for(unsigned int i=0;i<planes.size();i++)
			{
				Lua::Push<Plane>(l,planes[i]);
				lua_rawseti(l,top,i +1);
			}
		}));
		def.def("GetFarPlaneCenter",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Vector3>(l,hComponent->GetFarPlaneCenter());
		}));
		def.def("GetNearPlaneCenter",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Vector3>(l,hComponent->GetNearPlaneCenter());
		}));
		def.def("GetPlaneCenter",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float z) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<Vector3>(l,hComponent->GetPlaneCenter(z));
		}));
		def.def("GetFarPlaneBoundaries",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			std::array<Vector3,4> farBounds;
			hComponent->GetFarPlaneBoundaries(farBounds);

			lua_newtable(l);
			int top = lua_gettop(l);
			for(unsigned int i=0;i<farBounds.size();i++)
			{
				Lua::Push<Vector3>(l,farBounds[i]);
				lua_rawseti(l,top,i +1);
			}
		}));
		def.def("GetNearPlaneBoundaries",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			std::array<Vector3,4> nearBounds;
			hComponent->GetNearPlaneBoundaries(nearBounds);

			lua_newtable(l);
			int top = lua_gettop(l);
			for(unsigned int i=0;i<nearBounds.size();i++)
			{
				Lua::Push<Vector3>(l,nearBounds[i]);
				lua_rawseti(l,top,i +1);
			}
		}));
		def.def("GetPlaneBoundaries",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float z) {
			pragma::Lua::check_component(l,hComponent);
			std::array<Vector3,4> bounds;
			hComponent->GetPlaneBoundaries(z,bounds);

			lua_newtable(l);
			int top = lua_gettop(l);
			for(unsigned int i=0;i<bounds.size();i++)
			{
				Lua::Push<Vector3>(l,bounds[i]);
				lua_rawseti(l,top,i +1);
			}
		}));
		def.def("GetPlaneBoundaries",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			std::array<Vector3,8> bounds;
			hComponent->GetPlaneBoundaries(bounds);

			lua_newtable(l);
			int top = lua_gettop(l);
			for(unsigned int i=0;i<4;i++)
			{
				Lua::Push<Vector3>(l,bounds[i]);
				lua_rawseti(l,top,i +1);
			}

			lua_newtable(l);
			top = lua_gettop(l);
			for(unsigned int i=4;i<8;i++)
			{
				Lua::Push<Vector3>(l,bounds[i]);
				lua_rawseti(l,top,i -4 +1);
			}
		}));
		def.def("SetProjectionMatrix",static_cast<void(*)(lua_State*,THandle&,const Mat4&)>([](lua_State *l,THandle &hComponent,const Mat4 &mat) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetProjectionMatrix(mat);
		}));
		def.def("SetViewMatrix",static_cast<void(*)(lua_State*,THandle&,const Mat4&)>([](lua_State *l,THandle &hComponent,const Mat4 &mat) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetViewMatrix(mat);
		}));
		def.def("GetNearPlaneBounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			float wNear,hNear;
			hComponent->GetNearPlaneBounds(&wNear,&hNear);
			Lua::PushNumber(l,wNear);
			Lua::PushNumber(l,hNear);
		}));
		def.def("GetFarPlaneBounds",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			float wNear,hNear;
			hComponent->GetFarPlaneBounds(&wNear,&hNear);
			Lua::PushNumber(l,wNear);
			Lua::PushNumber(l,hNear);
		}));
		def.def("GetFarPlaneBounds",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float z) {
			pragma::Lua::check_component(l,hComponent);
			float w,h;
			hComponent->GetPlaneBounds(z,w,h);
			Lua::PushNumber(l,w);
			Lua::PushNumber(l,h);
		}));
		def.def("GetFrustumPoints",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			std::vector<Vector3> points;
			hComponent->GetFrustumPoints(points);

			int table = Lua::CreateTable(l);
			for(unsigned int i=0;i<points.size();i++)
			{
				Lua::PushInt(l,i +1);
				Lua::Push<Vector3>(l,points[i]);
				Lua::SetTableValue(l,table);
			}
		}));
		def.def("GetNearPlanePoint",static_cast<void(*)(lua_State*,THandle&,const Vector2&)>([](lua_State *l,THandle &hComponent,const Vector2 &uv) {
			pragma::Lua::check_component(l,hComponent);
			auto point = hComponent->GetNearPlanePoint(uv);
			Lua::Push<decltype(point)>(l,point);
		}));
		def.def("GetFarPlanePoint",static_cast<void(*)(lua_State*,THandle&,const Vector2&)>([](lua_State *l,THandle &hComponent,const Vector2 &uv) {
			pragma::Lua::check_component(l,hComponent);
			auto point = hComponent->GetFarPlanePoint(uv);
			Lua::Push<decltype(point)>(l,point);
		}));
		def.def("GetPlanePoint",static_cast<void(*)(lua_State*,THandle&,float,const Vector2&)>([](lua_State *l,THandle &hComponent,float z,const Vector2 &uv) {
			pragma::Lua::check_component(l,hComponent);
			auto point = hComponent->GetPlanePoint(z,uv);
			Lua::Push<decltype(point)>(l,point);
		}));
		def.def("GetFrustumNeighbors",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hComponent,int planeID) {
			pragma::Lua::check_component(l,hComponent);
			if(planeID < 0 || planeID > 5)
				return;
			FrustumPlane neighborIDs[4];
			hComponent->GetFrustumNeighbors(FrustumPlane(planeID),&neighborIDs[0]);
			int table = Lua::CreateTable(l);
			for(unsigned int i=0;i<4;i++)
			{
				Lua::PushInt(l,i +1);
				Lua::PushInt(l,static_cast<int>(neighborIDs[i]));
				Lua::SetTableValue(l,table);
			}
		}));
		def.def("GetFrustumPlaneCornerPoints",static_cast<void(*)(lua_State*,THandle&,int,int)>([](lua_State *l,THandle &hComponent,int planeA,int planeB) {
			pragma::Lua::check_component(l,hComponent);
			if(planeA < 0 || planeB < 0 || planeA > 5 || planeB > 5)
				return;

			FrustumPoint cornerPoints[2];
			hComponent->GetFrustumPlaneCornerPoints(FrustumPlane(planeA),FrustumPlane(planeB),&cornerPoints[0]);

			Lua::PushInt(l,static_cast<int>(cornerPoints[0]));
			Lua::PushInt(l,static_cast<int>(cornerPoints[1]));
		}));
		def.def("CreateFrustumKDop",static_cast<void(*)(lua_State*,THandle&,const Vector2&,const Vector2&)>([](lua_State *l,THandle &hComponent,const Vector2 &uvStart,const Vector2 &uvEnd) {
			pragma::Lua::check_component(l,hComponent);
			std::vector<Plane> kDop;
			hComponent->CreateFrustumKDop(uvStart,uvEnd,kDop);

			auto table = Lua::CreateTable(l);
			auto idx = 1u;
			for(auto &plane : kDop)
			{
				Lua::PushInt(l,idx++);
				Lua::Push<Plane>(l,plane);
				Lua::SetTableValue(l,table);
			}
		}));
		def.def("CreateFrustumMesh",static_cast<void(*)(lua_State*,THandle&,const Vector2&,const Vector2&)>([](lua_State *l,THandle &hComponent,const Vector2 &uvStart,const Vector2 &uvEnd) {
			pragma::Lua::check_component(l,hComponent);
			std::vector<Vector3> verts;
			std::vector<uint16_t> indices;
			hComponent->CreateFrustumMesh(uvStart,uvEnd,verts,indices);
			auto t = Lua::CreateTable(l);
			auto vertIdx = 1u;
			for(auto &v : verts)
			{
				Lua::PushInt(l,vertIdx++);
				Lua::Push<Vector3>(l,v);
				Lua::SetTableValue(l,t);
			}

			t = Lua::CreateTable(l);
			auto idx = 1u;
			for(auto i : indices)
			{
				Lua::PushInt(l,idx++);
				Lua::PushInt(l,i);
				Lua::SetTableValue(l,t);
			}
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_env_explosion_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_fire_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_fog_controller_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_light_spot_vol_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_microphone_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_quake_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_smoke_trail_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_sound_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetSoundSource",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hComponent,const std::string &sndName) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetSoundSource(sndName);
		}));
		def.def("SetPitch",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float pitch) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetPitch(pitch);
		}));
		def.def("SetGain",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float gain) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetGain(gain);
		}));
		def.def("SetRolloffFactor",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float rolloffFactor) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetRolloffFactor(rolloffFactor);
		}));
		def.def("SetMinGain",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float minGain) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetRolloffFactor(minGain);
		}));
		def.def("SetMaxGain",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float maxGain) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetRolloffFactor(maxGain);
		}));
		def.def("SetInnerConeAngle",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float angle) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetInnerConeAngle(angle);
		}));
		def.def("SetOuterConeAngle",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float angle) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetOuterConeAngle(angle);
		}));
		def.def("SetOffset",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float offset) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetOffset(offset);
		}));
		def.def("GetOffset",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetOffset());
		}));
		def.def("SetTimeOffset",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float offset) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetTimeOffset(offset);
		}));
		def.def("GetTimeOffset",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent->GetTimeOffset());
		}));
		def.def("SetReferenceDistance",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float referenceDist) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetReferenceDistance(referenceDist);
		}));
		def.def("SetMaxDistance",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float maxDist) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetMaxDistance(maxDist);
		}));
		def.def("SetRelativeToListener",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hComponent,bool bRelative) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetRelativeToListener(bRelative);
		}));
		def.def("SetPlayOnSpawn",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hComponent,bool bPlayOnSpawn) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetPlayOnSpawn(bPlayOnSpawn);
		}));
		def.def("SetLooping",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hComponent,bool bLoop) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetLooping(bLoop);
		}));
		def.def("SetSoundTypes",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hComponent,uint32_t soundTypes) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetSoundType(static_cast<ALSoundType>(soundTypes));
		}));
		def.def("Play",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->Play();
		}));
		def.def("Stop",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->Stop();
		}));
		def.def("Pause",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->Pause();
		}));
		def.def("GetSound",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			auto &snd = hComponent->GetSound();
			if(snd == nullptr)
				return;
			Lua::Push<std::shared_ptr<ALSound>>(l,snd);
		}));
		def.def("IsPlaying",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->IsPlaying());
		}));
		def.def("IsPaused",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->IsPaused());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_env_soundscape_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_sprite_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_timescale_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_weather_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_wind_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_env_filter_name_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("ShouldPass",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hComponent,EntityHandle &hEnt) {
			pragma::Lua::check_component(l,hComponent);
			LUA_CHECK_ENTITY(l,hEnt);
			Lua::PushBool(l,hComponent->ShouldPass(*hEnt.get()));
		}));
		def.add_static_constant("EVENT_ON_NAME_CHANGED",pragma::BaseNameComponent::EVENT_ON_NAME_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_env_filter_class_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("ShouldPass",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hComponent,EntityHandle &hEnt) {
			pragma::Lua::check_component(l,hComponent);
			LUA_CHECK_ENTITY(l,hEnt);
			Lua::PushBool(l,hComponent->ShouldPass(*hEnt.get()));
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_func_brush_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_button_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_kinematic_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_physics_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_portal_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_soft_physics_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_func_surface_material_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_logic_relay_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_bot_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_path_node_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_ball_socket_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_cone_twist_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_dof_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_fixed_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_hinge_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_constraint_slider_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_render_target_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_target_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_prop_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_prop_dynamic_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_prop_physics_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_trigger_hurt_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_trigger_push_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_trigger_remove_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_trigger_teleport_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_touch_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetTriggerFlags",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hComponent,uint32_t triggerFlags) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetTriggerFlags(static_cast<pragma::BaseTouchComponent::TriggerFlags>(triggerFlags));
		}));
		def.def("GetTriggerFlags",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hComponent,uint32_t triggerFlags) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushInt(l,umath::to_integral(hComponent->GetTriggerFlags()));
		}));

		def.add_static_constant("EVENT_CAN_TRIGGER",pragma::BaseTouchComponent::EVENT_CAN_TRIGGER);
		def.add_static_constant("EVENT_ON_START_TOUCH",pragma::BaseTouchComponent::EVENT_ON_START_TOUCH);
		def.add_static_constant("EVENT_ON_END_TOUCH",pragma::BaseTouchComponent::EVENT_ON_END_TOUCH);
		def.add_static_constant("EVENT_ON_TRIGGER",pragma::BaseTouchComponent::EVENT_ON_TRIGGER);
		
		def.add_static_constant("TRIGGER_FLAG_NONE",umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::None));
		def.add_static_constant("TRIGGER_FLAG_BIT_PLAYERS",umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Players));
		def.add_static_constant("TRIGGER_FLAG_BIT_NPCS",umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::NPCs));
		def.add_static_constant("TRIGGER_FLAG_BIT_PHYSICS",umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Physics));
		def.add_static_constant("TRIGGER_FLAG_EVERYTHING",umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Everything));
	}

	template<class TLuaClass,class THandle>
		void register_base_trigger_gravity_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_flashlight_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_skybox_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetSkyAngles",static_cast<void(*)(lua_State*,THandle&,const EulerAngles&)>([](lua_State *l,THandle &hNPC,const EulerAngles &skyAngles) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->SetSkyAngles(skyAngles);
		}));
		def.def("GetSkyAngles",static_cast<void(*)(lua_State*,THandle&,const EulerAngles&)>([](lua_State *l,THandle &hNPC,const EulerAngles &skyAngles) {
			pragma::Lua::check_component(l,hNPC);
			Lua::Push<EulerAngles>(l,hNPC.get()->GetSkyAngles());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_world_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_ai_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetMoveSpeed",static_cast<void(*)(lua_State*,THandle&,const std::string&,float)>([](lua_State *l,THandle &hNPC,const std::string &anim,float speed) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->SetMoveSpeed(anim,speed);
		}));
		def.def("SetMoveSpeed",static_cast<void(*)(lua_State*,THandle&,uint32_t,float)>([](lua_State *l,THandle &hNPC,uint32_t animId,float speed) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->SetMoveSpeed(animId,speed);
		}));
		def.def("GetMoveSpeed",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hNPC,const std::string &anim) {
			pragma::Lua::check_component(l,hNPC);
			auto mdlComponent = hNPC->GetEntity().GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			if(mdl == nullptr)
				return;
			auto animId = mdl->LookupAnimation(anim);
			if(animId < 0)
				return;
			Lua::AI::GetMoveSpeed(l,hNPC,animId);
		}));
		def.def("GetMoveSpeed",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hNPC,uint32_t animId) {
			Lua::AI::GetMoveSpeed(l,hNPC,animId);
		}));
		def.def("ClearMoveSpeed",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hNPC,const std::string &anim) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->ClearMoveSpeed(anim);
		}));
		def.def("ClearMoveSpeed",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hNPC,uint32_t animId) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->ClearMoveSpeed(animId);
		}));
		def.def("ClearLookTarget",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hNPC) {
			pragma::Lua::check_component(l,hNPC);
			hNPC.get()->ClearLookTarget();
		}));
		def.def("SetLookTarget",static_cast<void(*)(lua_State*,THandle&,const Vector3&,float)>([](lua_State *l,THandle &hNPC,const Vector3 &tgt,float t) {
			Lua::AI::SetLookTarget(l,hNPC,tgt,t);
		}));
		def.def("SetLookTarget",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hNPC,const Vector3 &tgt) {
			Lua::AI::SetLookTarget(l,hNPC,tgt,std::numeric_limits<float>::max());
		}));
		def.def("SetLookTarget",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,float)>([](lua_State *l,THandle &hNPC,EntityHandle &hEnt,float t) {
			Lua::AI::SetLookTarget(l,hNPC,hEnt,t);
		}));
		def.def("SetLookTarget",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hNPC,EntityHandle &hEnt) {
			Lua::AI::SetLookTarget(l,hNPC,hEnt,std::numeric_limits<float>::max());
		}));
		def.def("GetLookTarget",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hNPC) {
			pragma::Lua::check_component(l,hNPC);
			Lua::Push<Vector3>(l,hNPC.get()->GetLookTarget());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_character_component_methods(lua_State *l,TLuaClass &def)
	{
		// Actor
		def.def("GetFrozenProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetFrozenProperty());
		}));
		def.add_static_constant("EVENT_ON_KILLED",pragma::BaseActorComponent::EVENT_ON_KILLED);
		def.add_static_constant("EVENT_ON_RESPAWN",pragma::BaseActorComponent::EVENT_ON_RESPAWN);

		// Character
		def.def("IsAlive",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			Lua::PushBool(l,hEntity.get()->IsAlive());
		}));
		def.def("IsDead",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			Lua::PushBool(l,hEntity.get()->IsDead());
		}));
		def.def("IsFrozen",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			Lua::PushBool(l,hEntity.get()->IsFrozen());
		}));
		def.def("SetFrozen",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEntity,bool b) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetFrozen(b);
		}));
		def.def("Kill",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->Kill();
		}));
		def.def("GetOrientationAxes",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			Vector3 *forward,*right,*up;
			hEntity.get()->GetOrientationAxes(&forward,&right,&up);
			Lua::Push<Vector3>(l,*forward);
			Lua::Push<Vector3>(l,*right);
			Lua::Push<Vector3>(l,*up);
		}));
		def.def("GetOrientationAxesRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto &rot = hEntity.get()->GetOrientationAxesRotation();
			Lua::Push<Quat>(l,rot);
		}));
		def.def("GetShootPos",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto shootPos = hEntity.get()->GetShootPosition();
			Lua::Push<Vector3>(l,shootPos);
		}));
		def.def("GetSlopeLimit",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto slopeLimit = hEntity.get()->GetSlopeLimit();
			Lua::PushNumber(l,umath::rad_to_deg(slopeLimit));
		}));
		def.def("GetStepOffset",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto stepOffset = hEntity.get()->GetStepOffset();
			Lua::PushNumber(l,stepOffset);
		}));
		def.def("GetTurnSpeed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto turnSpeed = hEntity.get()->GetTurnSpeed();
			Lua::PushNumber(l,turnSpeed);
		}));
		def.def("GetUpDirection",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto &up = hEntity.get()->GetUpDirection();
			Lua::Push<Vector3>(l,up);
		}));
		def.def("GetViewAngles",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto ang = hEntity.get()->GetViewAngles();
			Lua::Push<EulerAngles>(l,ang);
		}));
		def.def("GetViewForward",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto viewForward = hEntity.get()->GetViewForward();
			Lua::Push<Vector3>(l,viewForward);
		}));
		def.def("GetViewRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto &rot = hEntity.get()->GetViewOrientation();
			Lua::Push<Quat>(l,rot);
		}));
		def.def("GetViewRight",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto right = hEntity.get()->GetViewRight();
			Lua::Push<Vector3>(l,right);
		}));
		def.def("GetViewUp",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto up = hEntity.get()->GetViewUp();
			Lua::Push<Vector3>(l,up);
		}));
		def.def("Ragdolize",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->Ragdolize();
		}));
		def.def("SetSlopeLimit",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEntity,float slopeLimit) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetSlopeLimit(umath::deg_to_rad(slopeLimit));
		}));
		def.def("SetStepOffset",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEntity,float stepOffset) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetStepOffset(stepOffset);
		}));
		def.def("SetTurnSpeed",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEntity,float turnSpeed) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetTurnSpeed(turnSpeed);
		}));
		def.def("SetUpDirection",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEntity,const Vector3 &up) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetUpDirection(up);
		}));
		def.def("SetViewAngles",static_cast<void(*)(lua_State*,THandle&,const EulerAngles&)>([](lua_State *l,THandle &hEntity,const EulerAngles &ang) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetViewAngles(ang);
		}));
		def.def("SetViewRotation",static_cast<void(*)(lua_State*,THandle&,const Quat&)>([](lua_State *l,THandle &hEntity,const Quat &rot) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->SetViewOrientation(rot);
		}));
		def.def("NormalizeViewRotation",static_cast<void(*)(lua_State*,THandle&,Quat&)>([](lua_State *l,THandle &hEntity,Quat &rot) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->NormalizeViewOrientation(rot);
		}));
		def.def("NormalizeViewRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			hEntity.get()->NormalizeViewOrientation();
		}));
		def.def("GetHitboxPhysicsObject",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEntity) {
			pragma::Lua::check_component(l,hEntity);
			auto hPhys = hEntity.get()->GetHitboxPhysicsObject();
			if(!hPhys.IsValid())
				return;
			Lua::Push<decltype(hPhys)>(l,hPhys);
		}));
		def.def("GetWeapons",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto &weapons = hEnt.get()->GetWeapons();
			auto t = Lua::CreateTable(l);
			auto idx = 1;
			for(auto it=weapons.begin();it!=weapons.end();++it)
			{
				if(it->IsValid())
				{
					Lua::PushInt(l,idx++);
					lua_pushentity(l,it->get());
					Lua::SetTableValue(l,t);
				}
			}
		}));
		def.def("GetWeapons",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &className) {
			pragma::Lua::check_component(l,hEnt);
			auto weapons = hEnt.get()->GetWeapons(className);
			auto t = Lua::CreateTable(l);
			auto idx = 1;
			for(auto it=weapons.begin();it!=weapons.end();++it)
			{
				Lua::PushInt(l,idx++);
				lua_pushentity(l,(*it));
				Lua::SetTableValue(l,t);
			}
		}));
		def.def("GetWeaponCount",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt.get()->GetWeapons().size());
		}));
		def.def("GetActiveWeapon",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *wep = hEnt.get()->GetActiveWeapon();
			if(wep == NULL)
				return;
			lua_pushentity(l,wep);
		}));
		def.def("HasWeapon",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &className) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt.get()->HasWeapon(className));
		}));
		def.def("GetAimRayData",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<TraceData>(l,hEnt.get()->GetAimTraceData());
		}));
		def.def("GetAimRayData",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float maxDist) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<TraceData>(l,hEnt.get()->GetAimTraceData(maxDist));
		}));
		def.def("FootStep",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t foot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->FootStep(static_cast<::pragma::BaseCharacterComponent::FootType>(foot));
		}));
		def.def("IsMoving",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt.get()->IsMoving());
		}));
		def.def("SetNeckControllers",static_cast<void(*)(lua_State*,THandle&,const std::string&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &yawController,const std::string &pitchController) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->SetNeckControllers(yawController,pitchController);
		}));
		def.def("GetNeckYawController",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt.get()->GetNeckYawBlendController());
		}));
		def.def("GetNeckPitchController",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt.get()->GetNeckPitchBlendController());
		}));
		def.def("SetMoveController",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &moveController) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->SetMoveController(moveController);
		}));
		def.def("GetMoveController",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt.get()->GetMoveController());
		}));
		def.def("GetMoveVelocity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt.get()->GetMoveVelocity());
		}));
		def.def("GetRelativeVelocity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Vector3>(l,hEnt.get()->GetLocalVelocity());
		}));

		def.def("SetOrientation",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hEnt,const Vector3 &up) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->SetCharacterOrientation(up);
		}));
		def.def("GetLocalOrientationAngles",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt.get()->GetLocalOrientationAngles());
		}));
		def.def("GetLocalOrientationRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Quat>(l,hEnt.get()->GetLocalOrientationRotation());
		}));
		def.def("GetLocalOrientationViewAngles",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt.get()->GetLocalOrientationViewAngles());
		}));
		def.def("GetLocalOrientationViewRotation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Quat>(l,hEnt.get()->GetLocalOrientationViewRotation());
		}));
		def.def("WorldToLocalOrientation",static_cast<void(*)(lua_State*,THandle&,const Quat&)>([](lua_State *l,THandle &hEnt,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Quat>(l,hEnt.get()->WorldToLocalOrientation(rot));
		}));
		def.def("WorldToLocalOrientation",static_cast<void(*)(lua_State*,THandle&,const EulerAngles&)>([](lua_State *l,THandle &hEnt,const EulerAngles &ang) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt.get()->WorldToLocalOrientation(ang));
		}));
		def.def("LocalOrientationToWorld",static_cast<void(*)(lua_State*,THandle&,const Quat&)>([](lua_State *l,THandle &hEnt,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<Quat>(l,hEnt.get()->LocalOrientationToWorld(rot));
		}));
		def.def("LocalOrientationToWorld",static_cast<void(*)(lua_State*,THandle&,const EulerAngles&)>([](lua_State *l,THandle &hEnt,const EulerAngles &ang) {
			pragma::Lua::check_component(l,hEnt);
			Lua::Push<EulerAngles>(l,hEnt.get()->LocalOrientationToWorld(ang));
		}));

		def.def("GetAmmoCount",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hPl,const std::string &ammoType) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushInt(l,hPl.get()->GetAmmoCount(ammoType));
		}));
		def.def("GetAmmoCount",static_cast<void(*)(lua_State*,THandle&,UInt32)>([](lua_State *l,THandle &hPl,UInt32 ammoType) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushInt(l,hPl.get()->GetAmmoCount(ammoType));
		}));
		def.def("SetAmmoCount",static_cast<void(*)(lua_State*,THandle&,const std::string&,UInt32)>([](lua_State *l,THandle &hPl,const std::string &ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetAmmoCount(ammoType,umath::limit<UInt16>(count));
		}));
		def.def("SetAmmoCount",static_cast<void(*)(lua_State*,THandle&,UInt32,UInt32)>([](lua_State *l,THandle &hPl,UInt32 ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetAmmoCount(ammoType,umath::limit<UInt16>(count));
		}));

		def.def("AddAmmo",static_cast<void(*)(lua_State*,THandle&,const std::string&,UInt32)>([](lua_State *l,THandle &hPl,const std::string &ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->AddAmmo(ammoType,count);
		}));
		def.def("AddAmmo",static_cast<void(*)(lua_State*,THandle&,UInt32,UInt32)>([](lua_State *l,THandle &hPl,UInt32 ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->AddAmmo(ammoType,count);
		}));
		def.def("RemoveAmmo",static_cast<void(*)(lua_State*,THandle&,const std::string&,UInt32)>([](lua_State *l,THandle &hPl,const std::string &ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->RemoveAmmo(ammoType,count);
		}));
		def.def("RemoveAmmo",static_cast<void(*)(lua_State*,THandle&,UInt32,UInt32)>([](lua_State *l,THandle &hPl,UInt32 ammoType,UInt32 count) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->RemoveAmmo(ammoType,count);
		}));
		def.def("GetUpDirectionProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetUpDirectionProperty());
		}));
		def.def("GetSlopeLimitProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetSlopeLimitProperty());
		}));
		def.def("GetStepOffsetProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetStepOffsetProperty());
		}));
		def.def("GetJumpPower",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushNumber(l,hComponent.get()->GetJumpPower());
		}));
		def.def("SetJumpPower",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hComponent,float jumpPower) {
			pragma::Lua::check_component(l,hComponent);
			hComponent.get()->SetJumpPower(jumpPower);
		}));
		def.def("Jump",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent.get()->Jump());
		}));
		def.def("Jump",static_cast<void(*)(lua_State*,THandle&,const Vector3&)>([](lua_State *l,THandle &hComponent,const Vector3 &velocity) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent.get()->Jump(velocity));
		}));
		def.def("GetJumpPowerProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Property::push(l,*hComponent->GetJumpPowerProperty());
		}));

		def.add_static_constant("EVENT_ON_FOOT_STEP",pragma::BaseCharacterComponent::EVENT_ON_FOOT_STEP);
		def.add_static_constant("EVENT_ON_CHARACTER_ORIENTATION_CHANGED",pragma::BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED);
		def.add_static_constant("EVENT_ON_DEPLOY_WEAPON",pragma::BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON);
		def.add_static_constant("EVENT_ON_SET_ACTIVE_WEAPON",pragma::BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON);
		def.add_static_constant("EVENT_PLAY_FOOTSTEP_SOUND",pragma::BaseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND);
		def.add_static_constant("EVENT_ON_DEATH",pragma::BaseCharacterComponent::EVENT_ON_DEATH);
		def.add_static_constant("EVENT_CALC_MOVEMENT_SPEED",pragma::BaseCharacterComponent::EVENT_CALC_MOVEMENT_SPEED);
		def.add_static_constant("EVENT_CALC_AIR_MOVEMENT_MODIFIER",pragma::BaseCharacterComponent::EVENT_CALC_AIR_MOVEMENT_MODIFIER);
		def.add_static_constant("EVENT_CALC_MOVEMENT_ACCELERATION",pragma::BaseCharacterComponent::EVENT_CALC_MOVEMENT_ACCELERATION);
		def.add_static_constant("EVENT_CALC_MOVEMENT_DIRECTION",pragma::BaseCharacterComponent::EVENT_CALC_MOVEMENT_DIRECTION);
		def.add_static_constant("EVENT_IS_MOVING",pragma::BaseCharacterComponent::EVENT_IS_MOVING);
		def.add_static_constant("EVENT_HANDLE_VIEW_ROTATION",pragma::BaseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION);
		def.add_static_constant("EVENT_ON_JUMP",pragma::BaseCharacterComponent::EVENT_ON_JUMP);

		def.add_static_constant("FOOT_LEFT",umath::to_integral(pragma::BaseCharacterComponent::FootType::Left));
		def.add_static_constant("FOOT_RIGHT",umath::to_integral(pragma::BaseCharacterComponent::FootType::Right));
	}

	template<class TLuaClass,class THandle>
		void register_base_vehicle_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetSpeedKmh",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt.get()->GetSpeedKmh());
		}));
		def.def("GetSteeringFactor",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt.get()->GetSteeringFactor());
		}));
		def.def("GetSteeringWheel",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *ent = hEnt.get()->GetSteeringWheel();
			if(ent == nullptr)
				return;
			lua_pushentity(l,ent);
		}));
		def.def("HasDriver",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushBool(l,hEnt.get()->HasDriver());
		}));
		def.def("GetDriver",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *driver = hEnt.get()->GetDriver();
			if(driver == nullptr)
				return;
			driver->GetLuaObject()->push(l);
		}));
		def.def("SetDriver",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hDriver) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hDriver);
			hEnt.get()->SetDriver(hDriver.get());
		}));
		def.def("ClearDriver",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->ClearDriver();
		}));
		def.def("SetupSteeringWheel",static_cast<void(*)(lua_State*,THandle&,const std::string&,float)>([](lua_State *l,THandle &hEnt,const std::string &model,float maxSteeringAngle) {
			pragma::Lua::check_component(l,hEnt);
			hEnt.get()->SetupSteeringWheel(model,maxSteeringAngle);
		}));
		def.def("GetPhysicsVehicle",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *physVehicle = hEnt.get()->GetPhysicsVehicle();
			if(physVehicle == nullptr)
				return;
			physVehicle->Push(l);
		}));
		def.def("SetupPhysics",static_cast<void(*)(lua_State*,THandle&,const pragma::physics::VehicleCreateInfo&,luabind::object)>([](lua_State *l,THandle &hEnt,const pragma::physics::VehicleCreateInfo &vhcCreateInfo,luabind::object oWheelModels) {
			pragma::Lua::check_component(l,hEnt);
			auto tWheelModels = 3;
			Lua::CheckTable(l,tWheelModels);
			auto numWheels = Lua::GetObjectLength(l,tWheelModels);
			std::vector<std::string> wheelModels {};
			wheelModels.reserve(numWheels);
			for(auto i=decltype(numWheels){0u};i<numWheels;++i)
			{
				Lua::PushInt(l,i +1u);
				Lua::GetTableValue(l,tWheelModels);
				wheelModels.push_back(Lua::CheckString(l,-1));
				Lua::Pop(l,1);
			}
			hEnt.get()->SetupVehicle(vhcCreateInfo,wheelModels);
		}));
		def.add_static_constant("EVENT_ON_DRIVER_ENTERED",pragma::BaseVehicleComponent::EVENT_ON_DRIVER_ENTERED);
		def.add_static_constant("EVENT_ON_DRIVER_EXITED",pragma::BaseVehicleComponent::EVENT_ON_DRIVER_EXITED);
	}

	template<class TLuaClass,class THandle>
		void register_base_weapon_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetNextPrimaryAttack",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,Double t) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetNextPrimaryAttack(CFloat(t));
		}));
		def.def("SetNextSecondaryAttack",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,Double t) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetNextSecondaryAttack(CFloat(t));
		}));
		def.def("SetNextAttack",static_cast<void(*)(lua_State*,THandle&,double)>([](lua_State *l,THandle &hEnt,Double t) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetNextAttack(CFloat(t));
		}));
		def.def("SetAutomaticPrimary",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,Bool b) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetAutomaticPrimary(b);
		}));
		def.def("SetAutomaticSecondary",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,Bool b) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt.get()->SetAutomaticSecondary(b);
		}));
		def.def("IsAutomaticPrimary",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsAutomaticPrimary());
		}));
		def.def("IsAutomaticSecondary",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsAutomaticSecondary());
		}));
		def.def("IsDeployed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsDeployed());
		}));
		def.def("GetPrimaryAmmoType",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetPrimaryAmmoType());
		}));
		def.def("GetPrimaryAmmoTypeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt->GetPrimaryAmmoTypeProperty());
		}));
		def.def("GetSecondaryAmmoType",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetSecondaryAmmoType());
		}));
		def.def("GetSecondaryAmmoTypeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt->GetSecondaryAmmoTypeProperty());
		}));
		def.def("HasPrimaryAmmo",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->HasPrimaryAmmo());
		}));
		def.def("HasSecondaryAmmo",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->HasSecondaryAmmo());
		}));
		def.def("IsPrimaryClipEmpty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsPrimaryClipEmpty());
		}));
		def.def("IsSecondaryClipEmpty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->IsSecondaryClipEmpty());
		}));
		def.def("HasAmmo",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushBool(l,hEnt.get()->HasAmmo());
		}));
		def.def("GetPrimaryClipSize",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetPrimaryClipSize());
		}));
		def.def("GetPrimaryClipSizeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt.get()->GetPrimaryClipSizeProperty());
		}));
		def.def("GetSecondaryClipSize",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetSecondaryClipSize());
		}));
		def.def("GetSecondaryClipSizeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt.get()->GetSecondaryClipSizeProperty());
		}));
		def.def("GetMaxPrimaryClipSize",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetMaxPrimaryClipSize());
		}));
		def.def("GetMaxPrimaryClipSizeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt.get()->GetMaxPrimaryClipSizeProperty());
		}));
		def.def("GetMaxSecondaryClipSize",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::PushInt(l,hEnt.get()->GetMaxSecondaryClipSize());
		}));
		def.def("GetMaxSecondaryClipSizeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			Lua::Property::push(l,*hEnt.get()->GetMaxSecondaryClipSizeProperty());
		}));
		def.def("PrimaryAttack",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool bOnce) {
			Lua::Weapon::PrimaryAttack(l,hEnt,bOnce);
		}));
		def.def("PrimaryAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			Lua::Weapon::PrimaryAttack(l,hEnt,false);
		}));
		def.def("SecondaryAttack",static_cast<void(*)(lua_State*,THandle&,bool)>([](lua_State *l,THandle &hEnt,bool bOnce) {
			Lua::Weapon::SecondaryAttack(l,hEnt,bOnce);
		}));
		def.def("SecondaryAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			Lua::Weapon::SecondaryAttack(l,hEnt,false);
		}));
		def.def("TertiaryAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->TertiaryAttack();
		}));
		def.def("Attack4",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->Attack4();
		}));
		def.def("Reload",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->Reload();
		}));
		def.def("Deploy",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->Deploy();
		}));
		def.def("Holster",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->Holster();
		}));
		def.def("EndAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->EndAttack();
		}));
		def.def("EndPrimaryAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->EndPrimaryAttack();
		}));
		def.def("EndSecondaryAttack",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *wep = hEnt.get();
			wep->EndSecondaryAttack();
		}));

		def.add_static_constant("EVENT_ON_DEPLOY",pragma::BaseWeaponComponent::EVENT_ON_DEPLOY);
		def.add_static_constant("EVENT_ON_HOLSTER",pragma::BaseWeaponComponent::EVENT_ON_HOLSTER);
		def.add_static_constant("EVENT_ON_PRIMARY_ATTACK",pragma::BaseWeaponComponent::EVENT_ON_PRIMARY_ATTACK);
		def.add_static_constant("EVENT_ON_SECONDARY_ATTACK",pragma::BaseWeaponComponent::EVENT_ON_SECONDARY_ATTACK);
		def.add_static_constant("EVENT_ON_TERTIARY_ATTACK",pragma::BaseWeaponComponent::EVENT_ON_TERTIARY_ATTACK);
		def.add_static_constant("EVENT_ON_ATTACK4",pragma::BaseWeaponComponent::EVENT_ON_ATTACK4);
		def.add_static_constant("EVENT_ON_END_PRIMARY_ATTACK",pragma::BaseWeaponComponent::EVENT_ON_END_PRIMARY_ATTACK);
		def.add_static_constant("EVENT_ON_END_SECONDARY_ATTACK",pragma::BaseWeaponComponent::EVENT_ON_END_SECONDARY_ATTACK);
		def.add_static_constant("EVENT_ON_RELOAD",pragma::BaseWeaponComponent::EVENT_ON_RELOAD);
		def.add_static_constant("EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED",pragma::BaseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED);
		def.add_static_constant("EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED",pragma::BaseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_player_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetTimeConnected",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			double t = hPl.get()->TimeConnected();
			Lua::PushNumber(l,t);
		}));
		def.def("IsKeyDown",static_cast<void(*)(lua_State*,THandle&,int32_t)>([](lua_State *l,THandle &hPl,int key) {
			pragma::Lua::check_component(l,hPl);
			bool b = hPl.get()->GetActionInput(Action(key));
			lua_pushboolean(l,b);
		}));
		def.def("GetWalkSpeed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetWalkSpeed());
		}));
		def.def("GetRunSpeed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetRunSpeed());
		}));
		def.def("GetSprintSpeed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetSprintSpeed());
		}));
		def.def("SetWalkSpeed",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float speed) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetWalkSpeed(speed);
		}));
		def.def("SetRunSpeed",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float speed) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetRunSpeed(speed);
		}));
		def.def("SetSprintSpeed",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float speed) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetSprintSpeed(speed);
		}));
		def.def("GetCrouchedWalkSpeed",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetCrouchedWalkSpeed());
		}));
		def.def("SetCrouchedWalkSpeed",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float speed) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetCrouchedWalkSpeed(speed);
		}));
		def.def("IsLocalPlayer",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			lua_pushboolean(l,hPl.get()->IsLocalPlayer());
		}));
		def.def("GetStandHeight",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetStandHeight());
		}));
		def.def("SetStandHeight",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float height) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetStandHeight(height);
		}));
		def.def("GetCrouchHeight",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetCrouchHeight());
		}));
		def.def("SetCrouchHeight",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float height) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetCrouchHeight(height);
		}));
		def.def("GetStandEyeLevel",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetStandEyeLevel());
		}));
		def.def("SetStandEyeLevel",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float eyelevel) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetStandEyeLevel(eyelevel);
		}));
		def.def("GetCrouchEyeLevel",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetCrouchEyeLevel());
		}));
		def.def("SetCrouchEyeLevel",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hPl,float eyelevel) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetCrouchEyeLevel(eyelevel);
		}));
		def.def("SetObserverMode",static_cast<void(*)(lua_State*,THandle&,int32_t)>([](lua_State *l,THandle &hPl,int mode) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetObserverMode(OBSERVERMODE(mode));
		}));
		def.def("GetObserverMode",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushInt(l,int(hPl.get()->GetObserverMode()));
		}));
		def.def("GetObserverModeProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::Property::push(l,*hPl->GetObserverModeProperty());
		}));

		def.def("SetObserverTarget",static_cast<void(*)(lua_State*,THandle&,::util::WeakHandle<pragma::BaseObservableComponent>&)>([](lua_State *l,THandle &hPl,::util::WeakHandle<pragma::BaseObservableComponent> &hEntTarget) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetObserverTarget(hEntTarget.get());
		}));
		def.def("GetObserverTarget",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			auto *ent = hPl.get()->GetObserverTarget();
			if(ent == nullptr)
				return;
			ent->PushLuaObject(l);
		}));

		def.def("GetViewPos",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::Push<Vector3>(l,hPl.get()->GetViewPos());
		}));

		def.def("ApplyViewRotationOffset",static_cast<void(*)(lua_State*,THandle&,EulerAngles&,float)>([](lua_State *l,THandle &hPl,EulerAngles &ang,float dur) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->ApplyViewRotationOffset(ang,dur);
		}));
		def.def("ApplyViewRotationOffset",static_cast<void(*)(lua_State*,THandle&,EulerAngles&)>([](lua_State *l,THandle &hPl,EulerAngles &ang) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->ApplyViewRotationOffset(ang);
		}));
		def.def("PrintMessage",static_cast<void(*)(lua_State*,THandle&,const std::string&,uint32_t)>([](lua_State *l,THandle &hPl,const std::string &msg,uint32_t messageType) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->PrintMessage(msg,static_cast<MESSAGE>(messageType));
		}));
		def.def("GetActionInput",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hPl,UInt32 input) {
			pragma::Lua::check_component(l,hPl);
			auto r = hPl.get()->GetActionInput(static_cast<Action>(input));
			Lua::PushBool(l,r);
		}));
		def.def("GetActionInputs",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hPl) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushInt(l,umath::to_integral(hPl.get()->GetActionInputs()));
		}));
		def.def("GetActionInputAxisMagnitude",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hPl,uint32_t input) {
			pragma::Lua::check_component(l,hPl);
			Lua::PushNumber(l,hPl.get()->GetActionInputAxisMagnitude(static_cast<Action>(input)));
		}));
		def.def("SetActionInputAxisMagnitude",static_cast<void(*)(lua_State*,THandle&,uint32_t,float)>([](lua_State *l,THandle &hPl,uint32_t input,float magnitude) {
			pragma::Lua::check_component(l,hPl);
			hPl.get()->SetActionInputAxisMagnitude(static_cast<Action>(input),magnitude);
		}));

		def.add_static_constant("EVENT_HANDLE_ACTION_INPUT",pragma::BasePlayerComponent::EVENT_HANDLE_ACTION_INPUT);
		def.add_static_constant("EVENT_ON_OBSERVATION_MODE_CHANGED",pragma::BasePlayerComponent::EVENT_ON_OBSERVATION_MODE_CHANGED);

		def.add_static_constant("MESSAGE_TYPE_CONSOLE",umath::to_integral(MESSAGE::PRINTCONSOLE));
		def.add_static_constant("MESSAGE_TYPE_CHAT",umath::to_integral(MESSAGE::PRINTCHAT));

		// Enums
		def.add_static_constant("OBSERVERMODE_NONE",umath::to_integral(OBSERVERMODE::NONE));
		def.add_static_constant("OBSERVERMODE_FIRSTPERSON",umath::to_integral(OBSERVERMODE::FIRSTPERSON));
		def.add_static_constant("OBSERVERMODE_THIRDPERSON",umath::to_integral(OBSERVERMODE::THIRDPERSON));
		def.add_static_constant("OBSERVERMODE_SHOULDER",umath::to_integral(OBSERVERMODE::SHOULDER));
		def.add_static_constant("OBSERVERMODE_ROAMING",umath::to_integral(OBSERVERMODE::ROAMING));
	}

	template<class TLuaClass,class THandle>
		void register_base_io_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("Input",static_cast<void(*)(lua_State*,THandle&,std::string,EntityHandle&,EntityHandle&,std::string)>([](lua_State *l,THandle &hIo,std::string input,EntityHandle &hActivator,EntityHandle &hCaller,std::string data) {
			pragma::Lua::check_component(l,hIo);
			LUA_CHECK_ENTITY(l,hActivator);
			LUA_CHECK_ENTITY(l,hCaller);
			hIo->Input(input,hActivator.get(),hCaller.get(),data);
		}));
		def.def("Input",static_cast<void(*)(lua_State*,THandle&,std::string,EntityHandle&,std::string)>([](lua_State *l,THandle &hIo,std::string input,EntityHandle &hActivator,std::string data) {
			pragma::Lua::check_component(l,hIo);
			LUA_CHECK_ENTITY(l,hActivator);
			hIo->Input(input,hActivator.get(),NULL,data);
		}));
		def.def("Input",static_cast<void(*)(lua_State*,THandle&,std::string,std::string)>([](lua_State *l,THandle &hIo,std::string input,std::string data) {
			pragma::Lua::check_component(l,hIo);
			hIo->Input(input,NULL,NULL,data);
		}));
		def.def("Input",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hIo,std::string input) {
			pragma::Lua::check_component(l,hIo);
			hIo->Input(input,NULL,NULL,"");
		}));
		def.def("StoreOutput",static_cast<void(*)(lua_State*,THandle&,const std::string&,const std::string&,const std::string&,const std::string&,float,int32_t)>([](lua_State *l,THandle &hIo,const std::string &name,const std::string &entities,const std::string &input,const std::string &param,float delay,int32_t times) {
			pragma::Lua::check_component(l,hIo);
			hIo->StoreOutput(name,entities,input,param,delay,times);
		}));
		def.def("StoreOutput",static_cast<void(*)(lua_State*,THandle&,const std::string&,const std::string&,const std::string&,const std::string&,float)>([](lua_State *l,THandle &hIo,const std::string &name,const std::string &entities,const std::string &input,const std::string &param,float delay) {
			pragma::Lua::check_component(l,hIo);
			hIo->StoreOutput(name,entities,input,param,delay);
		}));
		def.def("StoreOutput",static_cast<void(*)(lua_State*,THandle&,const std::string&,const std::string&,const std::string&,const std::string&)>([](lua_State *l,THandle &hIo,const std::string &name,const std::string &entities,const std::string &input,const std::string &param) {
			pragma::Lua::check_component(l,hIo);
			hIo->StoreOutput(name,entities,input,param);
		}));
		def.def("StoreOutput",static_cast<void(*)(lua_State*,THandle&,const std::string&,const std::string&)>([](lua_State *l,THandle &hIo,const std::string &name,const std::string &info) {
			pragma::Lua::check_component(l,hIo);
			hIo->StoreOutput(name,info);
		}));
		def.def("FireOutput",static_cast<void(*)(lua_State*,THandle&,const std::string&,EntityHandle&)>([](lua_State *l,THandle &hIo,const std::string &name,EntityHandle &hEnt) {
			pragma::Lua::check_component(l,hIo);
			LUA_CHECK_ENTITY(l,hEnt);
			hIo->TriggerOutput(name,hEnt.get());
		}));
		def.add_static_constant("EVENT_HANDLE_INPUT",pragma::BaseIOComponent::EVENT_HANDLE_INPUT);
	}

	template<class TLuaClass,class THandle>
		void register_base_model_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetModel",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hModel) {
			pragma::Lua::check_component(l,hModel);
			hModel->SetModel(std::shared_ptr<Model>{nullptr});
		}));
		def.def("SetModel",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hModel,const std::string &mdl) {
			pragma::Lua::check_component(l,hModel);
			hModel->SetModel(mdl);
		}));
		def.def("SetModel",static_cast<void(*)(lua_State*,THandle&,const std::shared_ptr<Model>&)>([](lua_State *l,THandle &hModel,const std::shared_ptr<Model> &mdl) {
			pragma::Lua::check_component(l,hModel);
			hModel->SetModel(mdl);
		}));
		def.def("SetSkin",static_cast<void(*)(lua_State*,THandle&,unsigned int)>([](lua_State *l,THandle &hModel,unsigned int skin) {
			pragma::Lua::check_component(l,hModel);
			hModel->SetSkin(skin);
		}));
		def.def("GetSkin",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hModel) {
			pragma::Lua::check_component(l,hModel);
			Lua::PushInt(l,hModel->GetSkin());
		}));
		def.def("GetSkinProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hModel) {
			pragma::Lua::check_component(l,hModel);
			Lua::Property::push(l,*hModel->GetSkinProperty());
		}));
		def.def("SetRandomSkin",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hModel) {
			pragma::Lua::check_component(l,hModel);
			auto &mdl = hModel->GetModel();
			if(mdl == nullptr)
				return;
			hModel->SetSkin(umath::random(0,umath::max(mdl->GetTextureGroups().size(),static_cast<size_t>(1)) -1));
		}));
		def.def("GetModel",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hModel) {
			pragma::Lua::check_component(l,hModel);
			auto mdl = hModel->GetModel();
			if(mdl == nullptr)
				return;
			luabind::object(l,mdl).push(l);
		}));
		def.def("GetBodyGroup",static_cast<void(*)(lua_State*,THandle&,UInt32)>([](lua_State *l,THandle &hModel,UInt32 groupId) {
			pragma::Lua::check_component(l,hModel);
			auto val = hModel->GetBodyGroup(groupId);
			Lua::PushInt(l,val);
		}));
		def.def("SetBodyGroup",static_cast<void(*)(lua_State*,THandle&,UInt32,UInt32)>([](lua_State *l,THandle &hModel,UInt32 groupId,UInt32 val) {
			pragma::Lua::check_component(l,hModel);
			hModel->SetBodyGroup(groupId,val);
		}));
		def.def("LookupAnimation",static_cast<void(*)(lua_State*,THandle&,const char*)>([](lua_State *l,THandle &hEnt,const char *anim) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->LookupAnimation(anim));
		}));

		def.def("GetModelName",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushString(l,hEnt->GetModelName().c_str());
		}));

		def.def("LookupBlendController",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string controller) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->LookupBlendController(controller));
		}));
		def.def("LookupAttachment",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string attachment) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->LookupAttachment(attachment));
		}));
		def.def("GetHitboxCount",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->GetHitboxCount());
		}));
		def.def("GetHitboxBounds",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 min,max,origin;
			auto rot = uquat::identity();
			hEnt->GetHitboxBounds(boneId,min,max,origin,rot);
			Lua::Push<Vector3>(l,min);
			Lua::Push<Vector3>(l,max);
			Lua::Push<Vector3>(l,origin);
			Lua::Push<Quat>(l,rot);
		}));
		def.def("LookupBone",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string boneName) {
			pragma::Lua::check_component(l,hEnt);
			int bone = hEnt->LookupBone(boneName);
			Lua::PushInt(l,bone);
		}));
		def.def("GetAttachmentTransform",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string attachment) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 offset(0,0,0);
			auto rot = uquat::identity();
			if(hEnt->GetAttachment(attachment,&offset,&rot) == false)
				return;
			Lua::Push<Vector3>(l,offset);
			Lua::Push<Quat>(l,rot);
		}));
		def.def("GetAttachmentTransform",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hEnt,int attachment) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 offset(0,0,0);
			auto rot = uquat::identity();
			if(hEnt->GetAttachment(attachment,&offset,&rot) == false)
				return;
			Lua::Push<Vector3>(l,offset);
			Lua::Push<Quat>(l,rot);
		}));

		def.add_static_constant("EVENT_ON_MODEL_CHANGED",pragma::BaseModelComponent::EVENT_ON_MODEL_CHANGED);
		def.add_static_constant("EVENT_ON_MODEL_MATERIALS_LOADED",pragma::BaseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED);
	}

	template<class TLuaClass,class THandle>
		void register_base_animated_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("PlayAnimation",static_cast<void(*)(lua_State*,THandle&,int,uint32_t)>([](lua_State *l,THandle &hAnim,int anim,uint32_t flags) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayAnimation(anim,static_cast<pragma::FPlayAnim>(flags));
		}));
		def.def("PlayAnimation",static_cast<void(*)(lua_State*,THandle&,std::string,uint32_t)>([](lua_State *l,THandle &hAnim,std::string anim,uint32_t flags) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushBool(l,hAnim->PlayAnimation(anim,static_cast<pragma::FPlayAnim>(flags)));
		}));
		def.def("PlayAnimation",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hAnim,int anim) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayAnimation(anim,pragma::FPlayAnim::Default);
		}));
		def.def("PlayAnimation",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hAnim,std::string anim) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushBool(l,hAnim->PlayAnimation(anim,pragma::FPlayAnim::Default));
		}));
		def.def("GetAnimation",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hAnim) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushInt(l,hAnim->GetAnimation());
		}));
		def.def("PlayActivity",static_cast<void(*)(lua_State*,THandle&,int,uint32_t)>([](lua_State *l,THandle &hAnim,int activity,uint32_t flags) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayActivity(static_cast<Activity>(activity),static_cast<pragma::FPlayAnim>(flags));
		}));
		def.def("PlayActivity",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hAnim,int activity) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayActivity(static_cast<Activity>(activity),pragma::FPlayAnim::Default);
		}));
		def.def("GetActivity",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hAnim) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushInt(l,umath::to_integral(hAnim->GetActivity()));
		}));
		def.def("PlayLayeredAnimation",static_cast<void(*)(lua_State*,THandle&,int,int)>([](lua_State *l,THandle &hAnim,int slot,int anim) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayLayeredAnimation(slot,anim);
		}));
		def.def("PlayLayeredAnimation",static_cast<void(*)(lua_State*,THandle&,int,std::string)>([](lua_State *l,THandle &hAnim,int slot,std::string anim) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayLayeredAnimation(slot,anim);
		}));
		def.def("PlayLayeredActivity",static_cast<void(*)(lua_State*,THandle&,int,int)>([](lua_State *l,THandle &hAnim,int slot,int activity) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->PlayLayeredActivity(slot,static_cast<Activity>(activity));
		}));
		def.def("StopLayeredAnimation",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hAnim,int slot) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->StopLayeredAnimation(slot);
		}));
		def.def("GetLayeredAnimation",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hAnim,uint32_t slot) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushInt(l,hAnim->GetLayeredAnimation(slot));
		}));
		def.def("GetLayeredActivity",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hAnim,uint32_t slot) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushInt(l,umath::to_integral(hAnim->GetLayeredActivity(slot)));
		}));
		def.def("SetPlaybackRate",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hAnim,float rate) {
			pragma::Lua::check_component(l,hAnim);
			hAnim->SetPlaybackRate(rate);
		}));
		def.def("GetPlaybackRate",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hAnim) {
			pragma::Lua::check_component(l,hAnim);
			Lua::PushNumber(l,hAnim->GetPlaybackRate());
		}));
		def.def("GetPlaybackRateProperty",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hAnim) {
			pragma::Lua::check_component(l,hAnim);
			Lua::Property::push(l,*hAnim->GetPlaybackRateProperty());
		}));
		def.def("GetBoneMatrix",static_cast<void(*)(lua_State*,THandle&,unsigned int)>([](lua_State *l,THandle &hAnim,unsigned int boneID) {
			pragma::Lua::check_component(l,hAnim);
			auto mat = hAnim->GetBoneMatrix(boneID);
			if(mat.has_value() == false)
				return;
			luabind::object(l,*mat).push(l);
		}));
		def.def("GetBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 pos = {};
			auto rot = uquat::identity();
			Vector3 scale = {1.f,1.f,1.f};
			auto r = hEnt->GetBonePosition(boneId,pos,rot,scale);
			if(r == false)
				return;
			Lua::Push<Vector3>(l,pos);
			Lua::Push<Quat>(l,rot);
			Lua::Push<Vector3>(l,scale);
		}));
		def.def("GetBonePose",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 pos = {};
			auto rot = uquat::identity();
			Vector3 scale = {1.f,1.f,1.f};
			auto r = hEnt->GetBonePosition(boneId,pos,rot,scale);
			if(r == false)
				return;
			Lua::Push<umath::ScaledTransform>(l,umath::ScaledTransform{pos,rot,scale});
		}));
		def.def("GetBonePos",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			auto *pos = hEnt->GetBonePosition(boneId);
			if(pos == nullptr)
				return;
			Lua::Push<Vector3>(l,*pos);
		}));
		def.def("GetBoneRot",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			auto *rot = hEnt->GetBoneRotation(boneId);
			if(rot == nullptr)
				return;
			Lua::Push<Quat>(l,*rot);
		}));
		def.def("GetLocalBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 pos = {};
			auto rot = uquat::identity();
			Vector3 scale(1.f,1.f,1.f);
			auto r = hEnt->GetLocalBonePosition(boneId,pos,rot,&scale);
			if(r == false)
				return;
			Lua::Push<Vector3>(l,pos);
			Lua::Push<Quat>(l,rot);
			Lua::Push<Vector3>(l,scale);
		}));
		def.def("GetGlobalBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			Vector3 pos = {};
			auto rot = uquat::identity();
			Vector3 scale(1.f,1.f,1.f);
			auto r = hEnt->GetGlobalBonePosition(boneId,pos,rot,&scale);
			if(r == false)
				return;
			Lua::Push<Vector3>(l,pos);
			Lua::Push<Quat>(l,rot);
			Lua::Push<Vector3>(l,scale);
		}));
		def.def("GetBoneRotation",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			auto *rot = hEnt->GetBoneRotation(boneId);
			if(rot == nullptr)
				return;
			Lua::Push<Quat>(l,*rot);
		}));
		def.def("GetBoneAngles",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			EulerAngles ang;
			auto r = hEnt->GetBoneAngles(boneId,ang);
			if(r == false)
				return;
			Lua::Push<EulerAngles>(l,ang);
		}));

		def.def("SetBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const Quat&,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBonePosition(boneId,pos,rot,scale);
		}));
		def.def("SetBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const Quat&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBonePosition(boneId,pos,rot);
		}));
		def.def("SetBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const EulerAngles&,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const EulerAngles &ang,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBonePosition(boneId,pos,uquat::create(ang),scale);
		}));
		def.def("SetBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const EulerAngles&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const EulerAngles &ang) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBonePosition(boneId,pos,ang);
		}));

		def.def("SetBonePos",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBonePosition(boneId,pos);
		}));
		def.def("SetBoneRot",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Quat&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBoneRotation(boneId,rot);
		}));
		def.def("SetBoneAngles",static_cast<void(*)(lua_State*,THandle&,uint32_t,const EulerAngles&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const EulerAngles &ang) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBoneRotation(boneId,uquat::create(ang));
		}));
		def.def("SetBoneScale",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBoneScale(boneId,scale);
		}));
		def.def("GetBoneScale",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t boneId) {
			pragma::Lua::check_component(l,hEnt);
			auto *scale = hEnt->GetBoneScale(boneId);
			if(scale == nullptr)
				return;
			Lua::Push<Vector3>(l,*scale);
		}));

		def.def("SetLocalBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const Quat&,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalBonePosition(boneId,pos,rot,scale);
		}));
		def.def("SetLocalBonePos",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalBonePosition(boneId,pos);
		}));
		def.def("SetLocalBoneRot",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Quat&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalBoneRotation(boneId,rot);
		}));

		def.def("SetGlobalBoneTransform",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&,const Quat&,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetGlobalBonePosition(boneId,pos,rot,scale);
		}));
		def.def("SetGlobalBonePos",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Vector3&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Vector3 &pos) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetGlobalBonePosition(boneId,pos);
		}));
		def.def("SetGlobalBoneRot",static_cast<void(*)(lua_State*,THandle&,uint32_t,const Quat&)>([](lua_State *l,THandle &hEnt,uint32_t boneId,const Quat &rot) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetGlobalBoneRotation(boneId,rot);
		}));

		def.def("SetCycle",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hEnt,float cycle) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetCycle(cycle);
		}));

		def.def("GetCycle",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			float cycle = hEnt->GetCycle();
			Lua::PushNumber(l,cycle);
		}));

		def.def("AddAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t,uint32_t,uint32_t,const luabind::object&)>([](lua_State *l,THandle &hEnt,uint32_t animId,uint32_t frameId,uint32_t eventId,const luabind::object &args) {
			pragma::Lua::check_component(l,hEnt);
			auto ev = get_animation_event(l,5,eventId);
			hEnt->AddAnimationEvent(animId,frameId,ev);
		}));
		def.def("AddAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t,uint32_t,const luabind::object&)>([](lua_State *l,THandle &hEnt,uint32_t animId,uint32_t frameId,const luabind::object &f) {
			pragma::Lua::check_component(l,hEnt);
			if(Lua::IsFunction(l,4))
			{
				auto hCb = hEnt->AddAnimationEvent(animId,frameId,CallbackHandle{std::shared_ptr<TCallback>(new LuaCallback(f))});
				Lua::Push<CallbackHandle>(l,hCb);
			}
			else
			{
				AnimationEvent ev {};
				ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l,4));
				hEnt->AddAnimationEvent(animId,frameId,ev);
			}
		}));
		def.def("AddAnimationEvent",static_cast<void(*)(lua_State*,THandle&,const std::string&,uint32_t,uint32_t,const luabind::object&)>([](lua_State *l,THandle &hEnt,const std::string &anim,uint32_t frameId,uint32_t eventId,const luabind::object &args) {
			pragma::Lua::check_component(l,hEnt);
			auto ev = get_animation_event(l,5,eventId);
			hEnt->AddAnimationEvent(anim,frameId,ev);
		}));
		def.def("AddAnimationEvent",static_cast<void(*)(lua_State*,THandle&,const std::string&,uint32_t,const luabind::object&)>([](lua_State *l,THandle &hEnt,const std::string &anim,uint32_t frameId,const luabind::object &f) {
			pragma::Lua::check_component(l,hEnt);
			if(Lua::IsFunction(l,4))
			{
				auto hCb = hEnt->AddAnimationEvent(anim,frameId,CallbackHandle{std::shared_ptr<TCallback>(new LuaCallback(f))});
				Lua::Push<CallbackHandle>(l,hCb);
			}
			else
			{
				AnimationEvent ev {};
				ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l,4));
				hEnt->AddAnimationEvent(anim,frameId,ev);
			}
		}));
		def.def("ClearAnimationEvents",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAnimationEvents();
		}));
		def.def("ClearAnimationEvents",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t animId) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAnimationEvents(animId);
		}));
		def.def("ClearAnimationEvents",static_cast<void(*)(lua_State*,THandle&,uint32_t,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t animId,uint32_t frameId) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAnimationEvents(animId,frameId);
		}));
		def.def("ClearAnimationEvents",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &anim) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAnimationEvents(anim);
		}));
		def.def("ClearAnimationEvents",static_cast<void(*)(lua_State*,THandle&,const std::string&,uint32_t)>([](lua_State *l,THandle &hEnt,const std::string &anim,uint32_t frameId) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAnimationEvents(anim,frameId);
		}));

		def.def("InjectAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t eventId) {
			pragma::Lua::check_component(l,hEnt);
			AnimationEvent ev {};
			ev.eventID = static_cast<AnimationEvent::Type>(eventId);
			hEnt->InjectAnimationEvent(ev);
		}));
		def.def("InjectAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t,const luabind::object&)>([](lua_State *l,THandle &hEnt,uint32_t eventId,const luabind::object &args) {
			pragma::Lua::check_component(l,hEnt);
			auto ev = get_animation_event(l,3,eventId);
			hEnt->InjectAnimationEvent(ev);
		}));
		def.def("BindAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t,luabind::object)>([](lua_State *l,THandle &hEnt,uint32_t eventId,luabind::object function) {
			pragma::Lua::check_component(l,hEnt);
			Lua::CheckFunction(l,3);
			hEnt->BindAnimationEvent(static_cast<AnimationEvent::Type>(eventId),[l,function](std::reference_wrapper<const AnimationEvent> ev) {
				Lua::CallFunction(l,[&function,&ev](lua_State *l) -> Lua::StatusCode {
					function.push(l);
					pragma::CEHandleAnimationEvent evData{ev};
					evData.PushArgumentVariadic(l);
					return Lua::StatusCode::Ok;
				});
			});
		}));
		def.def("BindAnimationEvent",static_cast<void(*)(lua_State*,THandle&,uint32_t,BaseEntityComponentHandle&,const std::string&)>([](lua_State *l,THandle &hEnt,uint32_t eventId,BaseEntityComponentHandle &component,const std::string &methodName) {
			pragma::Lua::check_component(l,hEnt);
			pragma::Lua::check_component(l,component);
			hEnt->BindAnimationEvent(static_cast<AnimationEvent::Type>(eventId),[component,methodName](std::reference_wrapper<const AnimationEvent> ev) {
				if(component.expired())
					return;
				auto o = component.get()->GetLuaObject();
				auto r = o[methodName];
				if(r)
				{
					auto *l = o.interpreter();
					auto c = Lua::CallFunction(l,[o,&methodName,&ev](lua_State *l) -> Lua::StatusCode {
						o.push(l);
						Lua::PushString(l,methodName);
						Lua::GetTableValue(l,-2);
						Lua::RemoveValue(l,-2);

						o.push(l);
						pragma::CEHandleAnimationEvent evData{ev};
						evData.PushArgumentVariadic(l);
						return Lua::StatusCode::Ok;
					},0);
				}
			});
		}));

		def.def("GetVertexPosition",static_cast<void(*)(lua_State*,THandle&,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,THandle &hEnt,uint32_t meshGroupId,uint32_t meshId,uint32_t subMeshId,uint32_t vertexId) {
			pragma::Lua::check_component(l,hEnt);
			auto pos = Vector3{};
			if(hEnt->GetVertexPosition(meshGroupId,meshId,subMeshId,vertexId,pos) == false)
				return;
			Lua::Push<Vector3>(l,pos);
		}));
		def.def("GetVertexPosition",static_cast<void(*)(lua_State*,THandle&,const std::shared_ptr<ModelSubMesh>&,uint32_t)>([](lua_State *l,THandle &hEnt,const std::shared_ptr<ModelSubMesh> &subMesh,uint32_t vertexId) {
			pragma::Lua::check_component(l,hEnt);
			auto pos = Vector3{};
			if(hEnt->GetVertexPosition(*subMesh,vertexId,pos) == false)
				return;
			Lua::Push<Vector3>(l,pos);
		}));
		def.def("SetBlendController",static_cast<void(*)(lua_State*,THandle&,unsigned int,float)>([](lua_State *l,THandle &hEnt,unsigned int controller,float val) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBlendController(controller,val);
		}));
		def.def("SetBlendController",static_cast<void(*)(lua_State*,THandle&,std::string,float)>([](lua_State *l,THandle &hEnt,std::string controller,float val) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetBlendController(controller,val);
		}));
		def.def("GetBlendController",static_cast<void(*)(lua_State*,THandle&,std::string)>([](lua_State *l,THandle &hEnt,std::string controller) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetBlendController(controller));
		}));
		def.def("GetBlendController",static_cast<void(*)(lua_State*,THandle&,unsigned int)>([](lua_State *l,THandle &hEnt,unsigned int controller) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushNumber(l,hEnt->GetBlendController(controller));
		}));
		def.def("GetBoneCount",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			Lua::PushInt(l,hEnt->GetBoneCount());
		}));

		def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT",pragma::BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
		def.add_static_constant("EVENT_ON_PLAY_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
		def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
		def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
		def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE",pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
		def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START",pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
		def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE",pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
		def.add_static_constant("EVENT_ON_ANIMATION_START",pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_START);
		def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
		def.add_static_constant("EVENT_TRANSLATE_ANIMATION",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
		def.add_static_constant("EVENT_TRANSLATE_ACTIVITY",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);

		def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT",pragma::BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
		def.add_static_constant("EVENT_ON_PLAY_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
		def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
		def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
		def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE",pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
		def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START",pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
		def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE",pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
		def.add_static_constant("EVENT_ON_ANIMATION_START",pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_START);
		def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
		def.add_static_constant("EVENT_TRANSLATE_ANIMATION",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
		def.add_static_constant("EVENT_TRANSLATE_ACTIVITY",pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);
		def.add_static_constant("EVENT_MAINTAIN_ANIMATIONS",pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS);
		def.add_static_constant("EVENT_MAINTAIN_ANIMATION",pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATION);
		def.add_static_constant("EVENT_MAINTAIN_ANIMATION_MOVEMENT",pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT);
		def.add_static_constant("EVENT_SHOULD_UPDATE_BONES",pragma::BaseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES);

		def.add_static_constant("EVENT_ON_PLAY_ACTIVITY",pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY);
		def.add_static_constant("EVENT_ON_STOP_LAYERED_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_STOP_LAYERED_ANIMATION);
		def.add_static_constant("EVENT_ON_BONE_TRANSFORM_CHANGED",pragma::BaseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED);
		def.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED",pragma::BaseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED);
		def.add_static_constant("EVENT_ON_BLEND_ANIMATION",pragma::BaseAnimatedComponent::EVENT_ON_BLEND_ANIMATION);
		def.add_static_constant("EVENT_PLAY_ANIMATION",pragma::BaseAnimatedComponent::EVENT_PLAY_ANIMATION);

		def.add_static_constant("FPLAYANIM_NONE",umath::to_integral(pragma::FPlayAnim::None));
		def.add_static_constant("FPLAYANIM_RESET",umath::to_integral(pragma::FPlayAnim::Reset));
		def.add_static_constant("FPLAYANIM_TRANSMIT",umath::to_integral(pragma::FPlayAnim::Transmit));
		def.add_static_constant("FPLAYANIM_SNAP_TO",umath::to_integral(pragma::FPlayAnim::SnapTo));
		def.add_static_constant("FPLAYANIM_DEFAULT",umath::to_integral(pragma::FPlayAnim::Default));
	}

	template<class TLuaClass,class THandle>
		void register_base_time_scale_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("GetTimeScale",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hTimeScale) {
			pragma::Lua::check_component(l,hTimeScale);
			Lua::PushNumber(l,hTimeScale->GetTimeScale());
		}));
		def.def("SetTimeScale",static_cast<void(*)(lua_State*,THandle&,float)>([](lua_State *l,THandle &hTimeScale,float timeScale) {
			pragma::Lua::check_component(l,hTimeScale);
			hTimeScale->SetTimeScale(timeScale);
		}));
		def.def("GetEffectiveTimeScale",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hTimeScale) {
			pragma::Lua::check_component(l,hTimeScale);
			Lua::PushNumber(l,hTimeScale->GetEffectiveTimeScale());
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_parent_component_methods(lua_State *l,TLuaClass &def)
	{
	}

	template<class TLuaClass,class THandle>
		void register_base_ownable_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetOwner",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hOwner) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			//LUA_CHECK_ENTITY(l,hOwner);
			auto *ownerComponent = hEnt.get();
			if(hOwner.IsValid())
				ownerComponent->SetOwner(*hOwner.get());
			else
				ownerComponent->ClearOwner();
		}));
		def.def("SetOwner",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *ownerComponent = hEnt.get();
			ownerComponent->ClearOwner();
		}));
		def.def("GetOwner",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			auto *owner = hEnt.get()->GetOwner();
			if(owner == nullptr)
				return;
			lua_pushentity(l,owner);
		}));
		def.add_static_constant("EVENT_ON_OWNER_CHANGED",pragma::BaseOwnableComponent::EVENT_ON_OWNER_CHANGED);
	}

	template<class TLuaClass,class THandle>
		void register_base_debug_text_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetText",static_cast<void(*)(lua_State*,THandle&,const std::string&)>([](lua_State *l,THandle &hEnt,const std::string &text) {
			if(pragma::Lua::check_component(l,hEnt) == false)
				return;
			hEnt->SetText(text);
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_debug_point_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_line_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_box_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_sphere_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_cone_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_cylinder_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_debug_plane_component_methods(lua_State *l,TLuaClass &def)
	{}

	template<class TLuaClass,class THandle>
		void register_base_point_at_target_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("SetPointAtTarget",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hTarget) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hTarget);
			if(hTarget.IsValid())
				hEnt->SetPointAtTarget(*hTarget.get());
			else
				hEnt->ClearPointAtTarget();
		}));
		def.def("SetPointAtTarget",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearPointAtTarget();
		}));
		def.def("GetPointAtTarget",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *entPointAtTarget = hEnt->GetPointAtTarget();
			if(entPointAtTarget == nullptr)
				return;
			entPointAtTarget->GetLuaObject()->push(l);
		}));
	}

	template<class TLuaClass,class THandle>
		void register_base_attachable_component_methods(lua_State *l,TLuaClass &def)
	{
		def.def("AttachToEntity",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,AttachmentInfo&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,AttachmentInfo &attInfo) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToEntity(hParent.get(),attInfo);
		}));

		def.def("AttachToEntity",static_cast<void(*)(lua_State*,THandle&,EntityHandle&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToEntity(hParent.get());
		}));

		def.def("ClearAttachment",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->ClearAttachment();
		}));

		def.def("AttachToAttachment",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,std::string,AttachmentInfo&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,std::string attachment,AttachmentInfo &attInfo) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToAttachment(hParent.get(),attachment,attInfo);
		}));
		def.def("AttachToAttachment",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,std::string)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,std::string attachment) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToAttachment(hParent.get(),attachment);
		}));
		def.def("AttachToAttachment",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,int,AttachmentInfo&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,int attachment,AttachmentInfo &attInfo) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToAttachment(hParent.get(),attachment,attInfo);
		}));
		def.def("AttachToAttachment",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,int)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,int attachment) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToAttachment(hParent.get(),attachment);
		}));
		def.def("AttachToBone",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,std::string,AttachmentInfo&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,std::string bone,AttachmentInfo &attInfo) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToBone(hParent.get(),bone,attInfo);
		}));
		def.def("AttachToBone",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,std::string)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,std::string bone) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToBone(hParent.get(),bone);
		}));
		def.def("AttachToBone",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,int,AttachmentInfo&)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,int bone,AttachmentInfo &attInfo) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToBone(hParent.get(),bone,attInfo);
		}));
		def.def("AttachToBone",static_cast<void(*)(lua_State*,THandle&,EntityHandle&,int)>([](lua_State *l,THandle &hEnt,EntityHandle &hParent,int bone) {
			pragma::Lua::check_component(l,hEnt);
			LUA_CHECK_ENTITY(l,hParent);
			hEnt->AttachToBone(hParent.get(),bone);
		}));
		def.def("GetLocalPose",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto pose = hEnt->GetLocalPose();
			if(pose.has_value() == false)
				return;
			Lua::Push<umath::Transform>(l,*pose);
		}));
		def.def("SetLocalPose",static_cast<void(*)(lua_State*,THandle&,const umath::Transform&)>([](lua_State *l,THandle &hEnt,const umath::Transform &pose) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetLocalPose(pose);
		}));

		def.def("GetParent",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto *ent = hEnt->GetParent();
			if(ent == NULL)
				return;
			ent->PushLuaObject(l);
		}));

		def.def("SetAttachmentFlags",static_cast<void(*)(lua_State*,THandle&,int)>([](lua_State *l,THandle &hEnt,int flags) {
			pragma::Lua::check_component(l,hEnt);
			hEnt->SetAttachmentFlags(static_cast<FAttachmentMode>(flags));
		}));
		def.def("GetAttachmentFlags",static_cast<void(*)(lua_State*,THandle&)>([](lua_State *l,THandle &hEnt) {
			pragma::Lua::check_component(l,hEnt);
			auto flags = hEnt->GetAttachmentFlags();
			Lua::PushInt(l,umath::to_integral(flags));
		}));

		auto defAttInfo = luabind::class_<AttachmentInfo>("AttachmentInfo");
		defAttInfo.def(luabind::constructor<>());
		defAttInfo.def("SetOffset",static_cast<void(*)(lua_State*,AttachmentInfo&,const Vector3&)>([](lua_State *l,AttachmentInfo &attInfo,const Vector3 &offset) {
			attInfo.offset = offset;
		}));
		defAttInfo.def("SetOffset",static_cast<void(*)(lua_State*,AttachmentInfo&)>([](lua_State *l,AttachmentInfo &attInfo) {
			attInfo.offset.reset();
		}));
		defAttInfo.def("SetRotation",static_cast<void(*)(lua_State*,AttachmentInfo&,const Quat&)>([](lua_State *l,AttachmentInfo &attInfo,const Quat &rotation) {
			attInfo.rotation = rotation;
		}));
		defAttInfo.def("SetRotation",static_cast<void(*)(lua_State*,AttachmentInfo&)>([](lua_State *l,AttachmentInfo &attInfo) {
			attInfo.rotation.reset();
		}));
		defAttInfo.def_readwrite("flags",reinterpret_cast<std::underlying_type_t<decltype(AttachmentInfo::flags)> AttachmentInfo::*>(&AttachmentInfo::flags));
		def.scope[defAttInfo];

		def.add_static_constant("EVENT_ON_ATTACHMENT_UPDATE",pragma::BaseAttachableComponent::EVENT_ON_ATTACHMENT_UPDATE);

		def.add_static_constant("FATTACHMENT_MODE_POSITION_ONLY",umath::to_integral(FAttachmentMode::PositionOnly));
		def.add_static_constant("FATTACHMENT_MODE_BONEMERGE",umath::to_integral(FAttachmentMode::BoneMerge));
		def.add_static_constant("FATTACHMENT_MODE_UPDATE_EACH_FRAME",umath::to_integral(FAttachmentMode::UpdateEachFrame));
		def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW",umath::to_integral(FAttachmentMode::PlayerView));
		def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW_YAW",umath::to_integral(FAttachmentMode::PlayerViewYaw));
		def.add_static_constant("FATTACHMENT_MODE_SNAP_TO_ORIGIN",umath::to_integral(FAttachmentMode::SnapToOrigin));
	}
};

#endif
