/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LCAMERA_H__
#define __C_LCAMERA_H__

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <mathutil/glmutil.h>

class WorldEnvironment;
class Material;
namespace util {struct DrawSceneInfo;};
class Scene;
namespace Lua
{
	namespace Scene
	{
		DLLCLIENT void UpdateBuffers(lua_State *l,pragma::CSceneComponent &scene,prosper::ICommandBuffer &hCommandBuffer);
		DLLCLIENT void GetWorldEnvironment(lua_State *l,pragma::CSceneComponent &scene);

		DLLCLIENT void RenderPrepass(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode);
		DLLCLIENT void Render(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderFlags renderFlags);
		DLLCLIENT void Render(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode);
	};
	namespace RasterizationRenderer
	{
		DLLCLIENT void GetPrepassDepthTexture(lua_State *l,pragma::CRasterizationRendererComponent &renderer);
		DLLCLIENT void GetPrepassNormalTexture(lua_State *l,pragma::CRasterizationRendererComponent &renderer);

		DLLCLIENT void GetRenderTarget(lua_State *l,pragma::CRasterizationRendererComponent &renderer);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::CRasterizationRendererComponent &renderer,const ::util::DrawSceneInfo &drawSceneInfo);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::CRasterizationRendererComponent &renderer,const ::util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass &rp);
		DLLCLIENT void GetPrepassShader(lua_State *l,pragma::CRasterizationRendererComponent &renderer);

		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,pragma::ShaderGameWorldLightingPass &shader,::Material &mat,BaseEntity &ent,ModelSubMesh &mesh);
		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,const std::string &shaderName,::Material &mat,BaseEntity &ent,ModelSubMesh &mesh);
		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,::Material &mat,BaseEntity &ent,ModelSubMesh &mesh);
	};
};

#endif
