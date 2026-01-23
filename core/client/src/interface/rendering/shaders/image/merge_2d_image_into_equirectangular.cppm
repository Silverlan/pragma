// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.merge_2d_image_to_equirectangular;

export import :rendering.shaders.cubemap;

export namespace pragma {
	class DLLCLIENT ShaderMerge2dImageIntoEquirectangular : public prosper::ShaderBaseImageProcessing {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE_2D;

		enum class CubeFace : uint32_t {
			PositiveX = 0,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ,

			Count
		};

#pragma pack(push, 1)
		struct PushConstants {
			float xFactor = 1.f;
			CubeFace cubeFace = CubeFace::PositiveX;
		};
#pragma pack(pop)

		ShaderMerge2dImageIntoEquirectangular(prosper::IPrContext &context, const std::string &identifier);
		bool RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTextureEquirect, prosper::IDescriptorSet &descSetTexture2d, CubeFace cubeFace = CubeFace::PositiveX, math::Degree range = 360.f);
	  protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
};
