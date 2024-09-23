/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_DEPTH_TO_RGB_H__
#define __C_SHADER_DEPTH_TO_RGB_H__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma {
	class DLLCLIENT ShaderDepthToRGB : public prosper::ShaderGraphics {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET;

#pragma pack(push, 1)
		struct PushConstants {
			float nearZ;
			float farZ;
			float contrastFactor;
		};
#pragma pack(pop)

		ShaderDepthToRGB(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader);
		ShaderDepthToRGB(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, float contrastFactor = 1.f) const;
	  protected:
		template<class TPushConstants>
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, const TPushConstants &pushConstants) const;
		virtual uint32_t GetPushConstantSize() const;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};

	//////////////////////

	class DLLCLIENT ShaderCubeDepthToRGB : public ShaderDepthToRGB {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			ShaderDepthToRGB::PushConstants basePushConstants;
			int32_t cubeSide;
		};
#pragma pack(pop)

		ShaderCubeDepthToRGB(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, uint32_t cubeSide, float contrastFactor = 1.f) const;
	  protected:
		virtual uint32_t GetPushConstantSize() const override;
	};

	//////////////////////

	class DLLCLIENT ShaderCSMDepthToRGB : public ShaderDepthToRGB {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			ShaderDepthToRGB::PushConstants basePushConstants;
			int32_t layer;
		};
#pragma pack(pop)

		ShaderCSMDepthToRGB(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetDepthTex, float nearZ, float farZ, uint32_t layer, float contrastFactor = 1.f) const;
	  protected:
		virtual uint32_t GetPushConstantSize() const override;
	};
};
#endif
