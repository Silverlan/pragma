// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lcamera.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>

import pragma.client.entities.components;
import pragma.client.scripting.lua;

extern DLLCLIENT CEngine *c_engine;

void Lua::Scene::UpdateBuffers(lua_State *l, pragma::CSceneComponent &scene, prosper::ICommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
		return;
	auto pCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	scene.UpdateBuffers(pCmdBuffer);
}
std::shared_ptr<WorldEnvironment> Lua::Scene::GetWorldEnvironment(lua_State *l, pragma::CSceneComponent &scene)
{
	auto *worldEnv = scene.GetWorldEnvironment();
	if(worldEnv == nullptr)
		return nullptr;
	return worldEnv->shared_from_this();
}

////////////////////////////////

std::shared_ptr<prosper::Texture> Lua::RasterizationRenderer::GetPrepassDepthTexture(lua_State *l, pragma::CRasterizationRendererComponent &renderer) { return renderer.GetPrepass().textureDepth; }
std::shared_ptr<prosper::Texture> Lua::RasterizationRenderer::GetPrepassNormalTexture(lua_State *l, pragma::CRasterizationRendererComponent &renderer) { return renderer.GetPrepass().textureNormals; }
std::shared_ptr<prosper::RenderTarget> Lua::RasterizationRenderer::GetRenderTarget(lua_State *l, pragma::CRasterizationRendererComponent &renderer) { return renderer.GetHDRInfo().sceneRenderTarget; }

bool Lua::RasterizationRenderer::BeginRenderPass(lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo) { return renderer.BeginRenderPass(drawSceneInfo); }
bool Lua::RasterizationRenderer::BeginRenderPass(lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass &rp) { return renderer.BeginRenderPass(drawSceneInfo, &rp); }
