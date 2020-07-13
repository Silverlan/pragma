/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lcamera.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include <pragma/math/plane.h>
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>

extern DLLCENGINE CEngine *c_engine;

void Lua::Scene::GetCamera(lua_State *l,::Scene &scene)
{
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return;
	cam->PushLuaObject(l);
}

void Lua::Scene::GetSize(lua_State *l,::Scene &scene)
{
	Lua::Push<Vector2i>(l,Vector2i{scene.GetWidth(),scene.GetHeight()});
}
void Lua::Scene::GetWidth(lua_State *l,::Scene &scene)
{
	Lua::PushInt(l,scene.GetWidth());
}
void Lua::Scene::GetHeight(lua_State *l,::Scene &scene)
{
	Lua::PushInt(l,scene.GetHeight());
}
void Lua::Scene::Resize(lua_State*,::Scene &scene,uint32_t width,uint32_t height)
{
	scene.Resize(width,height);
}
void Lua::Scene::BeginDraw(lua_State *l,::Scene &scene)
{
	/*auto *renderer = scene.GetRenderer();
	if(renderer == nullptr)
		return;
	renderer->BeginRendering();*/
}
void Lua::Scene::UpdateBuffers(lua_State *l,::Scene &scene,prosper::ICommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
		return;
	auto pCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	scene.UpdateBuffers(pCmdBuffer);
}
void Lua::Scene::GetWorldEnvironment(lua_State *l,::Scene &scene)
{
	auto *worldEnv = scene.GetWorldEnvironment();
	if(worldEnv == nullptr)
		return;
	Lua::Push<std::shared_ptr<WorldEnvironment>>(l,worldEnv->shared_from_this());
}
void Lua::Scene::ClearWorldEnvironment(lua_State *l,::Scene &scene)
{
	scene.ClearWorldEnvironment();
}
void Lua::Scene::SetWorldEnvironment(lua_State *l,::Scene &scene,WorldEnvironment &worldEnv)
{
	scene.SetWorldEnvironment(worldEnv);
}
void Lua::Scene::ReloadRenderTarget(lua_State *l,::Scene &scene,uint32_t width,uint32_t height)
{
	scene.ReloadRenderTarget(width,height);
}
void Lua::Scene::GetCameraDescriptorSet(lua_State *l,::Scene &scene,uint32_t bindPoint)
{
	auto &descSet = scene.GetCameraDescriptorSetGroup(static_cast<prosper::PipelineBindPoint>(bindPoint));
	if(descSet == nullptr)
		return;
	Lua::Push(l,descSet);
}
void Lua::Scene::GetCameraDescriptorSet(lua_State *l,::Scene &scene) {GetCameraDescriptorSet(l,scene,umath::to_integral(prosper::PipelineBindPoint::Graphics));}
void Lua::Scene::GetViewCameraDescriptorSet(lua_State *l,::Scene &scene)
{
	auto &descSet = scene.GetViewCameraDescriptorSetGroup();
	if(descSet == nullptr)
		return;
	Lua::Push(l,descSet);
}
void Lua::Scene::GetIndex(lua_State *l,::Scene &scene)
{
	Lua::PushInt(l,scene.GetSceneIndex());
}
void Lua::Scene::GetDebugMode(lua_State *l,::Scene &scene) {return Lua::PushInt(l,scene.GetDebugMode());}
void Lua::Scene::SetDebugMode(lua_State *l,::Scene &scene,uint32_t debugMode) {scene.SetDebugMode(static_cast<::Scene::DebugMode>(debugMode));}

////////////////////////////////

void Lua::RasterizationRenderer::GetPrepassDepthTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &depthTex = renderer.GetPrepass().textureDepth;
	if(depthTex == nullptr)
		return;
	Lua::Push(l,depthTex);
}
void Lua::RasterizationRenderer::GetPrepassNormalTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &normalTex = renderer.GetPrepass().textureNormals;
	if(normalTex == nullptr)
		return;
	Lua::Push(l,normalTex);
}
void Lua::RasterizationRenderer::GetRenderTarget(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &rt = renderer.GetHDRInfo().sceneRenderTarget;
	if(rt == nullptr)
		return;
	Lua::Push(l,rt);
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo)
{
	Lua::PushBool(l,renderer.BeginRenderPass(drawSceneInfo));
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass &rp)
{
	Lua::PushBool(l,renderer.BeginRenderPass(drawSceneInfo,&rp));
}
void Lua::RasterizationRenderer::EndRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const ::util::DrawSceneInfo &drawSceneInfo)
{
	Lua::PushBool(l,renderer.EndRenderPass(drawSceneInfo));
}
void Lua::RasterizationRenderer::GetPrepassShader(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &shader = renderer.GetPrepassShader();
	Lua::shader::push_shader(l,shader);
}
void Lua::RasterizationRenderer::SetShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName,const std::string &dstName) {renderer.SetShaderOverride(srcName,dstName);}
void Lua::RasterizationRenderer::ClearShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName) {renderer.ClearShaderOverride(srcName);}
void Lua::RasterizationRenderer::SetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t mode) {renderer.SetPrepassMode(static_cast<pragma::rendering::RasterizationRenderer::PrepassMode>(mode));}
void Lua::RasterizationRenderer::GetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer) {Lua::PushInt(l,umath::to_integral(renderer.GetPrepassMode()));}

void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderMode,pragma::ShaderTextured3DBase &shader,Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh
)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto *meshData = renderer.GetRenderInfo(static_cast<RenderMode>(renderMode));
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
	auto *ent = static_cast<CBaseEntity*>(hEnt.get());
	auto itEnt = matC.containers.find(ent);
	if(itEnt == matC.containers.end())
		itEnt = matC.containers.insert(std::make_pair(ent,EntityMeshInfo{ent})).first;

	auto &entC = itEnt->second;
	entC.meshes.push_back(&static_cast<CModelSubMesh&>(mesh));
}
void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderMode,const std::string &shaderName,Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh
)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto hShader = c_engine->GetShader(shaderName);
	auto *shader = hShader.valid() ? dynamic_cast<pragma::ShaderTextured3DBase*>(hShader.get()) : nullptr;
	if(shader == nullptr)
		return;
	ScheduleMeshForRendering(l,renderer,renderMode,*shader,mat,hEnt,mesh);
}
void Lua::RasterizationRenderer::ScheduleMeshForRendering(
	lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderMode,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh
)
{
	auto *shaderInfo = mat.GetShaderInfo();
	auto *shader = shaderInfo ? static_cast<::util::WeakHandle<prosper::Shader>*>(shaderInfo->GetShader().get()) : nullptr;
	if(shader == nullptr || shader->expired() || (*shader)->GetBaseTypeHashCode() != pragma::ShaderTextured3DBase::HASH_TYPE)
		return;
	auto &shaderTex = static_cast<pragma::ShaderTextured3DBase&>(**shader);
	ScheduleMeshForRendering(l,renderer,renderMode,shaderTex,mat,hEnt,mesh);
}
