// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.extract_diffuse_ambinet_occlusion;

export namespace pragma {
	class DLLCLIENT ShaderExtractDiffuseAmbientOcclusion : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderExtractDiffuseAmbientOcclusion(prosper::IPrContext &context, const std::string &identifier);
	};
};
