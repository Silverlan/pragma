/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PARTICLE_POLYBOARD_HPP__
#define __C_SHADER_PARTICLE_POLYBOARD_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma
{
	namespace rendering {class RasterizationRenderer;};
	class DLLCLIENT ShaderParticlePolyboard
		: public ShaderParticle2DBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		ShaderParticlePolyboard(prosper::IPrContext &context,const std::string &identifier);

#pragma pack(push,1)
		struct GeometryPushConstants
		{
			Vector3 posCam;
			float radius;
			float curvature;
		};
		struct FragmentPushConstants
		{
			int32_t renderFlags;
			float viewportW;
			float viewportH;
		};
#pragma pack(pop)
		bool Draw(Scene &scene,const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps,prosper::IBuffer &vertexBuffer,prosper::IBuffer &indexBuffer,uint32_t numIndices,float radius,float curvature);
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		bool Draw(Scene &scene,const pragma::CParticleSystemComponent &ps,bool bloom)=delete;
	};
};

#endif