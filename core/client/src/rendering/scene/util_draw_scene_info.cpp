/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering;

util::DrawSceneInfo::DrawSceneInfo() {}
util::DrawSceneInfo::DrawSceneInfo(const DrawSceneInfo &other)
    : scene {other.scene}, commandBuffer {other.commandBuffer}, renderTarget {other.renderTarget}, renderFlags {other.renderFlags}, clearColor {other.clearColor}, toneMapping {other.toneMapping}, prepassFilter {other.prepassFilter}, renderFilter {other.renderFilter},
      outputImage {other.outputImage}, clipPlane {other.clipPlane}, pvsOrigin {other.pvsOrigin}, outputLayerId {other.outputLayerId}, flags {other.flags}, renderStats {other.renderStats ? std::make_unique<RenderStats>(*other.renderStats) : nullptr}, exclusionMask {other.exclusionMask},
      inclusionMask {other.inclusionMask}, subPasses {other.subPasses ? std::make_unique<std::vector<DrawSceneInfo>>(*other.subPasses) : nullptr}
{
}
util::DrawSceneInfo &util::DrawSceneInfo::operator=(const DrawSceneInfo &other)
{
	scene = other.scene;
	commandBuffer = other.commandBuffer;
	renderTarget = other.renderTarget;
	renderFlags = other.renderFlags;
	clearColor = other.clearColor;
	toneMapping = other.toneMapping;
	clipPlane = other.clipPlane;
	pvsOrigin = other.pvsOrigin;

	prepassFilter = other.prepassFilter;
	renderFilter = other.renderFilter;
	outputImage = other.outputImage;

	exclusionMask = other.exclusionMask;
	inclusionMask = other.inclusionMask;

	outputLayerId = other.outputLayerId;
	flags = other.flags;
	renderStats = other.renderStats ? std::make_unique<RenderStats>(*other.renderStats) : nullptr;

	subPasses = other.subPasses ? std::make_unique<std::vector<DrawSceneInfo>>(*other.subPasses) : nullptr;
	return *this;
}

void util::DrawSceneInfo::AddSubPass(const DrawSceneInfo &drawSceneInfo)
{
	if(!subPasses)
		subPasses = std::make_unique<std::vector<DrawSceneInfo>>();
	subPasses->push_back(drawSceneInfo);
}
const std::vector<util::DrawSceneInfo> *util::DrawSceneInfo::GetSubPasses() const { return subPasses.get(); }

Vector3 util::DrawSceneInfo::GetPvsOrigin() const
{
	if(pvsOrigin)
		return *pvsOrigin;
	auto *cam = scene.valid() ? scene->GetActiveCamera().get() : nullptr;
	if(!cam)
		return {};
	return cam->GetEntity().GetPosition();
}

::pragma::rendering::RenderMask util::DrawSceneInfo::GetRenderMask(CGame &game) const
{
	auto mask = game.GetInclusiveRenderMasks();
	mask |= inclusionMask;
	mask &= ~exclusionMask;
	return mask;
}

util::RenderPassDrawInfo::RenderPassDrawInfo(const DrawSceneInfo &drawSceneInfo, prosper::ICommandBuffer &cmdBuffer) : drawSceneInfo {drawSceneInfo}, commandBuffer {cmdBuffer.shared_from_this()} {}
