/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_RAYTRACING_HPP__
#define __C_SHADER_RAYTRACING_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include <shader/prosper_shader.hpp>

namespace pragma {
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
			RenderWorld = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWorld),
			RenderView = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeView),
			RenderSkybox = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeSkybox),
			// RenderWater = umath::to_integral(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags::RenderModeWater),
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
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderRayTracing::RenderFlags)

#endif
