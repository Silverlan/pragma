// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_flat;

export import :rendering.shaders.scene;

export namespace pragma {
	class DLLCLIENT ShaderFlat : public ShaderScene {
	  public:
		static VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		ShaderFlat(prosper::IPrContext &context, const std::string &identifier);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override { return std::numeric_limits<uint32_t>::max(); }
		virtual uint32_t GetCameraDescriptorSetIndex() const override { return std::numeric_limits<uint32_t>::max(); }
	};
};
