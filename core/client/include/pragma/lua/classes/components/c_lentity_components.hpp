/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LENTITY_COMPONENTS_HPP__
#define __C_LENTITY_COMPONENTS_HPP__

#include <alsoundsystem.hpp>
#include "pragma/game/c_game.h"
#include <pragma/physics/raytraces.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/lentity_components_base_types.hpp>
#include "pragma/clientdefinitions.h"

namespace Lua
{
	DLLCLIENT void register_cl_ai_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_character_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_player_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_vehicle_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_weapon_component(lua_State *l,luabind::module_ &module);

	namespace ParticleSystem
	{
		DLLCLIENT void Stop(lua_State *l,pragma::CParticleSystemComponent &hComponent,bool bStopImmediately);
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
		DLLCLIENT void AddInitializer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
		DLLCLIENT void AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
		DLLCLIENT void AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
	}
	namespace ModelDef
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
	};
	namespace Animated
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
	};
	namespace Flex
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
	};
	namespace BSP
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod,luabind::class_<pragma::CWorldComponent,pragma::BaseWorldComponent> &defWorld);
	};
	namespace Lightmap
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
	};
	namespace VertexAnimated
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
	};
	namespace Render
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
		DLLCLIENT void GetTransformationMatrix(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void RenderModel(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void SetRenderMode(lua_State *l,pragma::CRenderComponent &hEnt,unsigned int mode);
		DLLCLIENT void GetRenderMode(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetLocalRenderBounds(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetLocalRenderSphereBounds(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetAbsoluteRenderBounds(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetAbsoluteRenderSphereBounds(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void SetLocalRenderBounds(lua_State *l,pragma::CRenderComponent &hEnt,Vector3 &min,Vector3 &max);
		// DLLCLIENT void UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam,bool bForceBufferUpdate);
		// DLLCLIENT void UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam);
		DLLCLIENT void GetRenderBuffer(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetBoneBuffer(lua_State *l,pragma::CRenderComponent &hEnt);
		DLLCLIENT void CalcRayIntersection(lua_State *l,pragma::CRenderComponent &hComponent,const Vector3 &start,const Vector3 &dir,bool precise);
		DLLCLIENT void CalcRayIntersection(lua_State *l,pragma::CRenderComponent &hComponent,const Vector3 &start,const Vector3 &dir);
	};
	namespace SoundEmitter
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
		DLLCLIENT void CreateSound(lua_State *l,pragma::CSoundEmitterComponent &hEnt,std::string sndname,uint32_t type,bool bTransmit);
		DLLCLIENT void EmitSound(lua_State *l,pragma::CSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
	namespace Flex
	{
		DLLCLIENT void GetFlexController(lua_State *l,pragma::CFlexComponent &hEnt,uint32_t flexId);
		DLLCLIENT void GetFlexController(lua_State *l,pragma::CFlexComponent &hEnt,const std::string &flexController);
		DLLCLIENT void CalcFlexValue(lua_State *l,pragma::CFlexComponent &hEnt,uint32_t flexId);
	};
};

#endif
