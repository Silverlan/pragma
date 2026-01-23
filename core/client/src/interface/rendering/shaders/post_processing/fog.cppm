// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_fog;

export import :rendering.shaders.base;

export namespace pragma {
	namespace shaderPPFog {
		using namespace shaderPPBase;
	}
	class DLLCLIENT ShaderPPFog : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push, 1)
		struct DLLCLIENT Fog {
			enum class DLLCLIENT Type : uint32_t { Linear = 0, Exponential, Exponential2 };
			enum class DLLCLIENT Flag : uint32_t { None = 0, Enabled = 1 };
			Vector4 color = {0.f, 0.f, 0.f, 0.f};
			float start = 0.f;
			float end = 0.f;
			float density = 0.f;
			Type type = Type::Linear;
			Flag flags = Flag::None;
		};
#pragma pack(pop)

		ShaderPPFog(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetFog) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
