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
#include <functional>
#include <mathutil/color.h>

class Scene;
class CBaseEntity;
namespace prosper {class IPrimaryCommandBuffer;};
namespace util
{
	struct DLLCLIENT DrawSceneInfo
	{
		std::shared_ptr<::Scene> scene = nullptr;
		mutable std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer = nullptr;
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		FRender renderFlags = FRender::All;
		std::optional<Color> clearColor = {};
		
		std::function<bool(CBaseEntity&)> prepassFilter = nullptr;
		std::function<bool(CBaseEntity&)> renderFilter = nullptr;

		std::shared_ptr<prosper::IImage> outputImage = nullptr;
		uint32_t outputLayerId = 0u;
	};
};

#endif
