// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LCAMERA_H__
#define __C_LCAMERA_H__

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <image/prosper_texture.hpp>
#include <mathutil/glmutil.h>

class WorldEnvironment;
class Material;
namespace util {
	struct DrawSceneInfo;
};
class Scene;
namespace pragma {
	class CSceneComponent;
	class CRasterizationRendererComponent;
	class ShaderGameWorldLightingPass;
};
namespace prosper {
	class IRenderPass;
	class ICommandBuffer;
};
class ModelSubMesh;
namespace Lua {
	namespace Scene {
		DLLCLIENT void UpdateBuffers(lua_State *l, pragma::CSceneComponent &scene, prosper::ICommandBuffer &hCommandBuffer);
		DLLCLIENT std::shared_ptr<WorldEnvironment> GetWorldEnvironment(lua_State *l, pragma::CSceneComponent &scene);
	};
	namespace RasterizationRenderer {
		DLLCLIENT std::shared_ptr<prosper::Texture> GetPrepassDepthTexture(lua_State *l, pragma::CRasterizationRendererComponent &renderer);
		DLLCLIENT std::shared_ptr<prosper::Texture> GetPrepassNormalTexture(lua_State *l, pragma::CRasterizationRendererComponent &renderer);

		DLLCLIENT std::shared_ptr<prosper::RenderTarget> GetRenderTarget(lua_State *l, pragma::CRasterizationRendererComponent &renderer);
		DLLCLIENT bool BeginRenderPass(lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo);
		DLLCLIENT bool BeginRenderPass(lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass &rp);
	};
};

#endif
