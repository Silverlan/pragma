/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_EYE_HPP__
#define __C_SHADER_EYE_HPP__

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"

namespace pragma
{
	class DLLCLIENT ShaderEye
		: public ShaderPBR
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
			Vector4 eyeOrigin = {};

			float maxDilationFactor = 1.f;
			float dilationFactor = 0.5f;
			float irisUvRadius = 0.2f;
		};
#pragma pack(pop)

		ShaderEye(prosper::IPrContext &context,const std::string &identifier);
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		bool BindEyeball(uint32_t skinMatIdx);
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
