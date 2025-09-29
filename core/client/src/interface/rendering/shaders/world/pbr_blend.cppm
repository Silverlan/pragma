// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:rendering.shaders.pbr_blend;

import :rendering.shaders.pbr;

export namespace pragma {
	class DLLCLIENT ShaderPBRBlend : public ShaderPBR {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ALPHA;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ALPHA;

		enum class MaterialBinding : uint32_t { AlbedoMap2 = umath::to_integral(ShaderPBR::MaterialBinding::Count), AlbedoMap3 };

#pragma pack(push, 1)
		struct PushConstants {
			int32_t alphaCount;
		};
#pragma pack(pop)

		ShaderPBRBlend(prosper::IPrContext &context, const std::string &identifier);
		virtual bool GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const override;
	  protected:
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipelineVertexAttributes() override;
		virtual bool OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, CModelSubMesh &mesh) const override;
	};
};
