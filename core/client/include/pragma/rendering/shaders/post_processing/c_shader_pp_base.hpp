/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PP_BASE_HPP__
#define __C_SHADER_PP_BASE_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPPBase
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		ShaderPPBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPPBase(prosper::IPrContext &context,const std::string &identifier,const std::string &fsShader);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
