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
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>

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
