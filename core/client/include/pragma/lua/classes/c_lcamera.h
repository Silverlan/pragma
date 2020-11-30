/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
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
		DLLCLIENT void GetCamera(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void GetSize(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void GetWidth(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void GetHeight(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void Resize(lua_State *l,CSceneHandle &scene,uint32_t width,uint32_t height);

		DLLCLIENT void BeginDraw(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void UpdateBuffers(lua_State *l,CSceneHandle &scene,prosper::ICommandBuffer &hCommandBuffer);
		DLLCLIENT void GetWorldEnvironment(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void ClearWorldEnvironment(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void SetWorldEnvironment(lua_State *l,CSceneHandle &scene,WorldEnvironment &worldEnv);

		DLLCLIENT void ReloadRenderTarget(lua_State *l,CSceneHandle &scene,uint32_t width,uint32_t height);

		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,CSceneHandle &scene,uint32_t bindPoint);
		DLLCLIENT void GetViewCameraDescriptorSet(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void GetIndex(lua_State *l,const CSceneHandle &scene);

		DLLCLIENT void GetDebugMode(lua_State *l,CSceneHandle &scene);
		DLLCLIENT void SetDebugMode(lua_State *l,CSceneHandle &scene,uint32_t debugMode);
		
		DLLCLIENT void Link(lua_State *l,CSceneHandle &scene,CSceneHandle &sceneOther);
		DLLCLIENT void BuildRenderQueue(lua_State *l,CSceneHandle &scene,::util::DrawSceneInfo &drawSceneInfo);
		DLLCLIENT void RenderPrepass(lua_State *l,CSceneHandle &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode);
		DLLCLIENT void Render(lua_State *l,CSceneHandle &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderFlags renderFlags);
		DLLCLIENT void Render(lua_State *l,CSceneHandle &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode);
	};
	namespace RasterizationRenderer
	{
		DLLCLIENT void GetPrepassDepthTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);
		DLLCLIENT void GetPrepassNormalTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void GetRenderTarget(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass &rp);
		DLLCLIENT void EndRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo);
		DLLCLIENT void GetPrepassShader(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void SetShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName,const std::string &dstName);
		DLLCLIENT void ClearShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName);

		DLLCLIENT void SetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t mode);
		DLLCLIENT void GetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,CSceneHandle &scene,uint32_t renderMode,pragma::ShaderTextured3DBase &shader,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh);
		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,CSceneHandle &scene,uint32_t renderMode,const std::string &shaderName,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh);
		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,CSceneHandle &scene,uint32_t renderMode,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh);
	};
};

#endif
