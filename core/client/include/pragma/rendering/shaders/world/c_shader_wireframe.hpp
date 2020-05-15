/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_WIREFRAME_HPP__
#define __C_SHADER_WIREFRAME_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWireframe
		: public ShaderTextured3DBase
	{
	public:
		ShaderWireframe(prosper::IPrContext &context,const std::string &identifier);
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
