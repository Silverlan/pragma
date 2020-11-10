/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_DRAW_SCENE_INFO_HPP__
#define __UTIL_DRAW_SCENE_INFO_HPP__

#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/c_settings.hpp"
#include <memory>
#include <optional>
#include <functional>
#include <mathutil/color.h>

class CBaseEntity;
namespace prosper {class IPrimaryCommandBuffer;};
namespace pragma {class CSceneComponent;};
namespace util
{
	struct DLLCLIENT DrawSceneInfo
	{
		util::WeakHandle<::pragma::CSceneComponent> scene = {};
		mutable std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer = nullptr;
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		FRender renderFlags = FRender::All;
		std::optional<Color> clearColor = {};
		std::optional<::pragma::rendering::ToneMapping> toneMapping {};
		
		std::function<bool(CBaseEntity&)> prepassFilter = nullptr;
		std::function<bool(CBaseEntity&)> renderFilter = nullptr;

		std::shared_ptr<prosper::IImage> outputImage = nullptr;
		uint32_t outputLayerId = 0u;
		bool flipVertically = false;
	};
};

#endif
