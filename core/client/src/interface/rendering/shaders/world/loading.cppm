// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

export module pragma.client.rendering.shaders:world_loading;

export namespace pragma {
	class DLLCLIENT ShaderLoading : public ShaderGameWorldLightingPass {
	  public:
		ShaderLoading(prosper::IPrContext &context, const std::string &identifier);
	};
};
