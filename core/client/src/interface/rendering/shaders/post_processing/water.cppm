// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.pp_water;

export import :rendering.shaders.base;
export import pragma.cmaterialsystem;

export namespace pragma {
	class DLLCLIENT ShaderPPWater : public ShaderPPBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_REFRACTION_MAP;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push, 1)
		struct PushConstants {
			Vector4 clipPlane;
		};
#pragma pack(pop)

		ShaderPPWater(prosper::IPrContext &context, const std::string &identifier);
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat);
		bool RecordRefractionMaterial(prosper::ShaderBindState &bindState, material::CMaterial &mat) const;
		bool RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetTime, prosper::IDescriptorSet &descSetFog, const Vector4 &clipPlane) const;
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
