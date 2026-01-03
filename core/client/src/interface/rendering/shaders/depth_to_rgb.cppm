// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.depth_to_rgb;

export import pragma.prosper;

export namespace pragma {
	class DLLCLIENT ShaderDepthToRGB : public prosper::ShaderGraphics {
	  public:
		static VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_UV;

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
