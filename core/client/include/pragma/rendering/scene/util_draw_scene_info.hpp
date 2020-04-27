/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_DRAW_SCENE_INFO_HPP__
#define __UTIL_DRAW_SCENE_INFO_HPP__

#include "pragma/rendering/c_renderflags.h"
#include <memory>
#include <optional>
#include <mathutil/color.h>

class Scene;
namespace prosper {class ICommandBuffer;};
namespace util
{
	struct DrawSceneInfo
	{
		std::shared_ptr<::Scene> scene = nullptr;
		std::shared_ptr<prosper::ICommandBuffer> commandBuffer = nullptr;
		FRender renderFlags = FRender::All;
		std::optional<Color> clearColor = {};
	};
};

#endif
