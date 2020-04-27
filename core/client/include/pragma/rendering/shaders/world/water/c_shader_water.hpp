/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_WATER_HPP__
#define __C_SHADER_WATER_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWater
		: public ShaderTextured3DBase
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		enum class MaterialBinding : uint32_t
		{
			DuDvMap = 0u,
			NormalMap,

			Count
		};

		enum class WaterBinding : uint32_t
		{
			ReflectionMap = 0u,
			RefractionMap,
			RefractionDepth,
			WaterSettings,
			WaterFog,

			Count
		};

#pragma pack(push,1)
		struct PushConstants
		{
			float waterFogIntensity;
			uint32_t enableReflection;
			Mat4 reflectionVp;
		};
#pragma pack(pop)

		ShaderWater(prosper::Context &context,const std::string &identifier);
		void SetReflectionEnabled(bool b);
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual void EndDraw() override;
		virtual bool BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
	private:
		bool UpdateBindFogDensity();
		bool m_bReflectionEnabled = false;
		std::weak_ptr<Scene> m_boundScene = {};
		EntityHandle m_boundEntity = {};
	};
};

#endif