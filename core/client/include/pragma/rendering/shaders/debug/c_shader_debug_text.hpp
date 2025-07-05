// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_DEBUG_TEXT_HPP__
#define __C_SHADER_DEBUG_TEXT_HPP__

#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"

namespace pragma {
	class DLLCLIENT ShaderDebugText : public ShaderDebug {
	  public:
		ShaderDebugText(prosper::IPrContext &context, const std::string &identifier);

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IBuffer &vertexBuffer, uint32_t vertexCount, prosper::IDescriptorSet &descSetTexture, const Mat4 &mvp = umat::identity(), const Vector4 &color = Vector4(1.f, 1.f, 1.f, 1.f)) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
