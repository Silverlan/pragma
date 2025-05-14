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
#include <pragma/lua/lentity_components_base_types.hpp>
#include "pragma/clientdefinitions.h"

namespace pragma {
	class CSoundEmitterComponent;
	class CFlexComponent;
};

class CParticleInitializer;
class CParticleOperator;
class CParticleRenderer;
namespace Lua {
	DLLCLIENT void register_cl_ai_component(lua_State *l, luabind::module_ &module);
	DLLCLIENT void register_cl_character_component(lua_State *l, luabind::module_ &module);
	DLLCLIENT void register_cl_player_component(lua_State *l, luabind::module_ &module);
	DLLCLIENT void register_cl_vehicle_component(lua_State *l, luabind::module_ &module);
	DLLCLIENT void register_cl_weapon_component(lua_State *l, luabind::module_ &module);

	namespace ParticleSystem {
		DLLCLIENT void Stop(lua_State *l, pragma::CParticleSystemComponent &hComponent, bool bStopImmediately);
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
		DLLCLIENT CParticleInitializer *AddInitializer(lua_State *l, pragma::CParticleSystemComponent &hComponent, std::string name, const luabind::map<std::string, void> &keyValues);
		DLLCLIENT CParticleOperator *AddOperator(lua_State *l, pragma::CParticleSystemComponent &hComponent, std::string name, const luabind::map<std::string, void> &keyValues);
		DLLCLIENT CParticleRenderer *AddRenderer(lua_State *l, pragma::CParticleSystemComponent &hComponent, std::string name, const luabind::map<std::string, void> &keyValues);
	}
	namespace ModelDef {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace Animated {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace Flex {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace BSP {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod, luabind::class_<pragma::CWorldComponent, pragma::BaseWorldComponent> &defWorld);
	};
	namespace Lightmap {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace VertexAnimated {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace Render {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
		DLLCLIENT void GetTransformationMatrix(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void RenderModel(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetLocalRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetLocalRenderSphereBounds(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetAbsoluteRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetAbsoluteRenderSphereBounds(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void SetLocalRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt, Vector3 &min, Vector3 &max);
		// DLLCLIENT void UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam,bool bForceBufferUpdate);
		// DLLCLIENT void UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam);
		DLLCLIENT void GetRenderBuffer(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void GetBoneBuffer(lua_State *l, pragma::CRenderComponent &hEnt);
		DLLCLIENT void CalcRayIntersection(lua_State *l, pragma::CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir, bool precise);
		DLLCLIENT void CalcRayIntersection(lua_State *l, pragma::CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir);
	};
	namespace SoundEmitter {
		DLLCLIENT void register_class(lua_State *l, luabind::module_ &entsMod);
	};
	namespace Flex {
		DLLCLIENT std::optional<float> GetFlexController(pragma::CFlexComponent &hEnt, uint32_t flexId);
		DLLCLIENT std::optional<float> GetFlexController(pragma::CFlexComponent &hEnt, const std::string &flexController);
		DLLCLIENT std::optional<float> CalcFlexValue(pragma::CFlexComponent &hEnt, uint32_t flexId);
	};
};

#endif
