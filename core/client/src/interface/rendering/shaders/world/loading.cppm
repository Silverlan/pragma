// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_loading;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderLoading : public ShaderGameWorldLightingPass {
	  public:
		ShaderLoading(prosper::IPrContext &context, const std::string &identifier);
	};
};
