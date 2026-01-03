// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.shaders.world_raytracing;

export import :entities.components.raytracing;

export namespace pragma {
	class DLLCLIENT ShaderRayTracing : public prosper::ShaderCompute {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_IMAGE_OUTPUT;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_GAME_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_IBL;

		enum class GameSceneBinding : uint32_t {
			TextureArray = 0,
			MaterialInfos,
			SubMeshInfos,
			EntityInstanceData,
			BoneMatrices,

			VertexBuffer,
			IndexBuffer,
			VertexWeightBuffer,
			AlphaBuffer
		};

		enum class RenderFlags : uint32_t {
			None = 0u,
			RenderWorld = math::to_integral(CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWorld),
			RenderView = math::to_integral(CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeView),
			RenderSkybox = math::to_integral(CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeSkybox),
			// RenderWater = pragma::math::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWater),
			NoIBL = RenderSkybox << 1u
		};

#pragma pack(push, 1)
		struct PushConstants {
			uint32_t numMeshes;
			uint32_t lightCount;
			uint32_t width;
			uint32_t height;
			float fov;
			uint32_t pxOffset; // First 16 bits = x-offset, last 16 bits = y-offset of the target image
			RenderFlags renderFlags;
		};
#pragma pack(pop)

		ShaderRayTracing(prosper::IPrContext &context, const std::string &identifier);
		bool RecordCompute(prosper::ShaderBindState &bindState, const PushConstants &pushConstants, prosper::IDescriptorSet &descSetOutputImage, prosper::IDescriptorSet &descSetGameScene, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetLightSources,
		  prosper::IDescriptorSet *descSetIBL, uint32_t workGroupsX, uint32_t workGroupsY) const;
	  protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::ShaderRayTracing::RenderFlags)
};
