/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"

using namespace pragma::rendering;

BaseRenderer::BaseRenderer()
{}
bool BaseRenderer::operator==(const BaseRenderer &other) const {return &other == this;}
bool BaseRenderer::operator!=(const BaseRenderer &other) const {return !operator==(other);}
bool BaseRenderer::RenderScene(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene == nullptr || drawSceneInfo.scene->GetRenderer() != this)
		return false;
	BeginRendering(drawSceneInfo);
	return true;
}
void BaseRenderer::BeginRendering(const util::DrawSceneInfo &drawSceneInfo)
{
	drawSceneInfo.scene->UpdateBuffers(drawSceneInfo.commandBuffer);
}
void BaseRenderer::Resize(uint32_t width, uint32_t height) {}
void BaseRenderer::UpdateRenderSettings() {}
void BaseRenderer::UpdateCameraData(Scene &scene,pragma::CameraData &cameraData) {}
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
