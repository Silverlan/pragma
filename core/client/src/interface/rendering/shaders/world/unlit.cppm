// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_unlit;

export import :rendering.shaders.textured;

export namespace pragma {
	class DLLCLIENT ShaderUnlit : public ShaderGameWorldLightingPass {
	  public:
		ShaderUnlit(prosper::IPrContext &context, const std::string &identifier);
	};
};
