/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_BLOB_HPP__
#define __C_SHADER_PARTICLE_BLOB_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderParticleBlob
		: public ShaderParticle2DBase
	{
	public:
		static const uint32_t MAX_BLOB_NEIGHBORS = 8;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BLOB_NEIGHBORS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BLOB_NEIGHBORS;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PARTICLE_DATA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CUBEMAP;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 specularColor;
			float reflectionIntensity;
			float refractionIndexRatio;
			uint32_t debugMode;
		};
#pragma pack(pop)

		ShaderParticleBlob(prosper::IPrContext &context,const std::string &identifier);
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif