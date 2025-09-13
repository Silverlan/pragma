// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LENTITY_COMPONENTS_HPP__
#define __C_LENTITY_COMPONENTS_HPP__

#include <alsoundsystem.hpp>
#include "pragma/game/c_game.h"
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include "pragma/clientdefinitions.h"

namespace pragma {
	class CFlexComponent;
};

namespace Lua {
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
