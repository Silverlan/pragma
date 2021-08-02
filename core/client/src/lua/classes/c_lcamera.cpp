/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lcamera.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include <pragma/math/plane.h>
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>

extern DLLCLIENT CEngine *c_engine;

void Lua::Scene::UpdateBuffers(lua_State *l,pragma::CSceneComponent &scene,prosper::ICommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
		return;
	auto pCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	scene.UpdateBuffers(pCmdBuffer);
}
void Lua::Scene::GetWorldEnvironment(lua_State *l,pragma::CSceneComponent &scene)
{
	auto *worldEnv = scene.GetWorldEnvironment();
	if(worldEnv == nullptr)
		return;
	Lua::Push<std::shared_ptr<WorldEnvironment>>(l,worldEnv->shared_from_this());
}
void Lua::Scene::RenderPrepass(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode)
{
	auto *renderer = scene.GetRenderer();
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent>{};
	if(raster.expired())
		return;
	auto &shaderPrepass = raster->GetPrepass().GetShader();
	auto curScene = drawSceneInfo.scene;
	drawSceneInfo.scene = scene.GetHandle<pragma::CSceneComponent>();
	shaderPrepass.BindSceneCamera(scene,*raster,false);
	RenderSystem::RenderPrepass(drawSceneInfo,renderMode);
	drawSceneInfo.scene = curScene;
}
void Lua::Scene::Render(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderFlags renderFlags)
{
	auto &cam = scene.GetActiveCamera();
	auto *renderInfo = scene.GetSceneRenderDesc().GetRenderInfo(renderMode);
	if(renderInfo != nullptr && cam.valid())
	{
		auto curScene = drawSceneInfo.scene;
		drawSceneInfo.scene = scene.GetHandle<pragma::CSceneComponent>();

		RenderSystem::Render(drawSceneInfo,renderMode,renderFlags);
		auto *renderInfo = scene.GetSceneRenderDesc().GetRenderInfo(renderMode);
		if(renderInfo != nullptr && cam.valid())
			RenderSystem::Render(drawSceneInfo,*cam,renderMode,renderFlags,renderInfo->translucentMeshes);

		drawSceneInfo.scene = curScene;
	}
}
void Lua::Scene::Render(lua_State *l,pragma::CSceneComponent &scene,::util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode) {Render(l,scene,drawSceneInfo,renderMode,RenderFlags::None);}

////////////////////////////////

void Lua::RasterizationRenderer::GetPrepassDepthTexture(lua_State *l,pragma::CRasterizationRendererComponent &renderer)
{
	auto &depthTex = renderer.GetPrepass().textureDepth;
	if(depthTex == nullptr)
		return;
	Lua::Push(l,depthTex);
}
void Lua::RasterizationRenderer::GetPrepassNormalTexture(lua_State *l,pragma::CRasterizationRendererComponent &renderer)
{
	auto &normalTex = renderer.GetPrepass().textureNormals;
	if(normalTex == nullptr)
		return;
	Lua::Push(l,normalTex);
}
void Lua::RasterizationRenderer::GetRenderTarget(lua_State *l,pragma::CRasterizationRendererComponent &renderer)
{
	auto &rt = renderer.GetHDRInfo().sceneRenderTarget;
	if(rt == nullptr)
		return;
	Lua::Push(l,rt);
}
void Lua::RasterizationRenderer::GetPrepassShader(lua_State *l,pragma::CRasterizationRendererComponent &renderer)
{
	auto &shader = renderer.GetPrepassShader();
	Lua::shader::push_shader(l,shader);
}

void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,pragma::ShaderGameWorldLightingPass &shader,Material &mat,BaseEntity &ent,ModelSubMesh &mesh
)
{
	auto *meshData = scene.GetSceneRenderDesc().GetRenderInfo(static_cast<RenderMode>(renderMode));
	if(meshData == nullptr)
		return;
	auto itShader = std::find_if(meshData->containers.begin(),meshData->containers.end(),[&shader](const std::unique_ptr<ShaderMeshContainer> &c) -> bool {
		return c->shader.get() == &shader;
		});
	if(itShader == meshData->containers.end())
	{
		meshData->containers.emplace_back(std::make_unique<ShaderMeshContainer>(&shader));
		itShader = meshData->containers.end() -1;
		(*itShader)->shader = shader.GetHandle();
	}

	auto &shaderC = **itShader;
	auto itMat = std::find_if(shaderC.containers.begin(),shaderC.containers.end(),[&mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &c) {
		return c->material == &mat;
		});
	if(itMat == shaderC.containers.end())
	{
		shaderC.containers.emplace_back(std::make_unique<RenderSystem::MaterialMeshContainer>(&mat));
		itMat = shaderC.containers.end() -1;
	}

	auto &matC = **itMat;
	auto *cent = static_cast<CBaseEntity*>(&ent);
	auto itEnt = matC.containers.find(cent);
	if(itEnt == matC.containers.end())
		itEnt = matC.containers.insert(std::make_pair(cent,EntityMeshInfo{cent})).first;

	auto &entC = itEnt->second;
	entC.meshes.push_back(&static_cast<CModelSubMesh&>(mesh));
}
void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,const std::string &shaderName,Material &mat,BaseEntity &ent,ModelSubMesh &mesh
)
{
	auto hShader = c_engine->GetShader(shaderName);
	auto *shader = hShader.valid() ? dynamic_cast<pragma::ShaderGameWorldLightingPass*>(hShader.get()) : nullptr;
	if(shader == nullptr)
		return;
	ScheduleMeshForRendering(l,renderer,scene,renderMode,*shader,mat,ent,mesh);
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::CRasterizationRendererComponent &renderer,const ::util::DrawSceneInfo &drawSceneInfo)
{
	Lua::PushBool(l,renderer.BeginRenderPass(drawSceneInfo));
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::CRasterizationRendererComponent &renderer,const ::util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass &rp)
{
	Lua::PushBool(l,renderer.BeginRenderPass(drawSceneInfo,&rp));
}
void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::CRasterizationRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t renderMode,::Material &mat,BaseEntity &ent,ModelSubMesh &mesh
)
{
	auto *shaderInfo = mat.GetShaderInfo();
	auto *shader = shaderInfo ? static_cast<::util::WeakHandle<prosper::Shader>*>(shaderInfo->GetShader().get()) : nullptr;
	if(shader == nullptr || shader->expired() || (*shader)->GetBaseTypeHashCode() != pragma::ShaderGameWorldLightingPass::HASH_TYPE)
		return;
	auto &shaderTex = static_cast<pragma::ShaderGameWorldLightingPass&>(**shader);
	ScheduleMeshForRendering(l,renderer,scene,renderMode,shaderTex,mat,ent,mesh);
}
