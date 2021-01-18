/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_LIGHT_CONE_HPP__
#define __C_SHADER_LIGHT_CONE_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	namespace rendering {class RasterizationRenderer;};
	class DLLCLIENT ShaderLightCone
		: public ShaderGameWorldLightingPass
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_MAP;

#pragma pack(push,1)
		struct PushConstants
		{
			float coneLength;
			uint32_t boundLightIndex;
		};
#pragma pack(pop)

		ShaderLightCone(prosper::IPrContext &context,const std::string &identifier);
		virtual bool BindSceneCamera(pragma::CSceneComponent &scene,const pragma::rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
	protected:
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		int32_t m_boundLightIndex = -1;
	};
};

#endif
