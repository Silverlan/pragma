// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_light_cone;

export import :model.mesh;
export import :rendering.shaders.base;

export namespace pragma {
	namespace shaderPPLightCone {
		using namespace shaderPPBase;
	}
	class DLLCLIENT ShaderPPLightCone : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;

		static VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

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
		bool RecordDraw(prosper::ShaderBindState &bindState, const geometry::CModelSubMesh &mesh, prosper::IDescriptorSet &descSetTex, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetInstance, prosper::IDescriptorSet &descSetCam) const;
		bool RecordPushConstants(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
