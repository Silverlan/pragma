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
		DLLCLIENT void Stop(lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately);
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
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod,luabind::class_<CWorldHandle,BaseEntityComponentHandle> &defWorld);
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
		DLLCLIENT void GetModelMatrix(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetTranslationMatrix(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetRotationMatrix(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetTransformationMatrix(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void RenderModel(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void SetRenderMode(lua_State *l,CRenderHandle &hEnt,unsigned int mode);
		DLLCLIENT void GetRenderMode(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetRenderBounds(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetRenderSphereBounds(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void SetRenderBounds(lua_State *l,CRenderHandle &hEnt,Vector3 &min,Vector3 &max);
		DLLCLIENT void UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd,bool bForceBufferUpdate);
		DLLCLIENT void UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::CommandBuffer> &drawCmd);
		DLLCLIENT void GetRenderBuffer(lua_State *l,CRenderHandle &hEnt);
		DLLCLIENT void GetBoneBuffer(lua_State *l,CRenderHandle &hEnt);
	};
	namespace SoundEmitter
	{
		DLLCLIENT void register_class(lua_State *l,luabind::module_ &entsMod);
		DLLCLIENT void CreateSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t type,bool bTransmit);
		DLLCLIENT void EmitSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
	namespace Flex
	{
		DLLCLIENT void GetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId);
		DLLCLIENT void GetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController);
		DLLCLIENT void CalcFlexValue(lua_State *l,CFlexHandle &hEnt,uint32_t flexId);
	};
};

#endif
