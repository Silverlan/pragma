/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering;

util::RenderPassDrawInfo::RenderPassDrawInfo(const DrawSceneInfo &drawSceneInfo,prosper::ICommandBuffer &cmdBuffer)
	: drawSceneInfo{drawSceneInfo},commandBuffer{cmdBuffer.shared_from_this()}
{}
