// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.specular_to_roughness;

export namespace pragma {
	class DLLCLIENT ShaderSpecularToRoughness : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderSpecularToRoughness(prosper::IPrContext &context, const std::string &identifier);
	};
};

pragma::ShaderSpecularToRoughness::ShaderSpecularToRoughness(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing {context, identifier, "programs/util/specular_to_roughness"} {}
