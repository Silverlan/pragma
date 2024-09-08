/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PP_LIGHT_CONE_HPP__
#define __C_SHADER_PP_LIGHT_CONE_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma {
	class CModelComponent;
	class DLLCLIENT ShaderPPLightCone : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		enum class TextureBinding : uint32_t { SceneTexturePostToneMapping = 0 };

#pragma pack(push, 1)
		struct DLLCLIENT PushConstants {
			void SetResolution(uint32_t w, uint32_t h) { resolution = w << 16 | (static_cast<uint16_t>(h)); }
			Vector4 color;
			Vector3 coneOrigin;
			float coneLength;
			uint32_t resolution;
			float nearZ;
			float farZ;
		};
#pragma pack(pop)

		ShaderPPLightCone(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, const CModelSubMesh &mesh, prosper::IDescriptorSet &descSetTex, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetInstance, prosper::IDescriptorSet &descSetCam) const;
		bool RecordPushConstants(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};

#endif
