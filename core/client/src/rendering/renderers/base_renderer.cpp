/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

// TODO: Remove this file
#if 0
#include "stdafx_client.h"
#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"

using namespace pragma::rendering;

static std::vector<BaseRenderer*> g_renderers;
const std::vector<BaseRenderer*> &BaseRenderer::GetRenderers() {return g_renderers;}
BaseRenderer::BaseRenderer()
{
	g_renderers.push_back(this);
}
BaseRenderer::~BaseRenderer()
{
	auto it = std::find(g_renderers.begin(),g_renderers.end(),this);
	assert(it != g_renderers.end());
	if(it == g_renderers.end())
		return;
	g_renderers.erase(it);
}
bool BaseRenderer::operator==(const BaseRenderer &other) const {return &other == this;}
bool BaseRenderer::operator!=(const BaseRenderer &other) const {return !operator==(other);}
bool BaseRenderer::RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo) {return true;}
bool BaseRenderer::Render(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || drawSceneInfo.scene->GetRenderer() != this)
		return false;
	BeginRendering(drawSceneInfo);
	return true;
}
void BaseRenderer::BeginRendering(const util::DrawSceneInfo &drawSceneInfo)
{
	drawSceneInfo.scene.get()->UpdateBuffers(drawSceneInfo.commandBuffer);
}
void BaseRenderer::Resize(uint32_t width, uint32_t height) {}
void BaseRenderer::UpdateRenderSettings() {}
void BaseRenderer::UpdateCameraData(pragma::CSceneComponent &scene,pragma::CameraData &cameraData) {}
bool BaseRenderer::IsRasterizationRenderer() const {return false;}
bool BaseRenderer::IsRayTracingRenderer() const {return false;}
prosper::Texture *BaseRenderer::GetPresentationTexture() {return GetSceneTexture();}
const prosper::Texture *BaseRenderer::GetSceneTexture() const {return const_cast<BaseRenderer*>(this)->GetSceneTexture();}
const prosper::Texture *BaseRenderer::GetPresentationTexture() const {return const_cast<BaseRenderer*>(this)->GetPresentationTexture();}
const prosper::Texture *BaseRenderer::GetHDRPresentationTexture() const {return const_cast<BaseRenderer*>(this)->GetHDRPresentationTexture();}

uint32_t BaseRenderer::GetWidth() const
{
	auto *tex = GetSceneTexture();
	return tex ? tex->GetImage().GetWidth() : 0;
}
uint32_t BaseRenderer::GetHeight() const
{
	auto *tex = GetSceneTexture();
	return tex ? tex->GetImage().GetHeight() : 0;
}
#endif
