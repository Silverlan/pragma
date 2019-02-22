#ifndef __UTIL_DRAW_SCENE_INFO_HPP__
#define __UTIL_DRAW_SCENE_INFO_HPP__

#include "pragma/rendering/c_renderflags.h"
#include <memory>
#include <optional>
#include <mathutil/color.h>

class Scene;
namespace prosper {class CommandBuffer;};
namespace util
{
	struct DrawSceneInfo
	{
		std::shared_ptr<::Scene> scene = nullptr;
		std::shared_ptr<prosper::CommandBuffer> commandBuffer = nullptr;
		FRender renderFlags = FRender::All;
		std::optional<Color> clearColor = {};
	};
};

#endif
