#ifndef __C_SHADER_PARTICLE_HPP__
#define __C_SHADER_PARTICLE_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma
{
	class CParticleSystemComponent;
	class DLLCLIENT ShaderParticle
		: public ShaderParticle2DBase
	{
	public:
		ShaderParticle(prosper::Context &context,const std::string &identifier);
	};

	class DLLCLIENT ShaderParticleRotational
		: public ShaderParticle2DBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_WORLD_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_WORLD_ROTATION;

		enum class VertexAttribute : uint32_t
		{
			WorldRotation = umath::to_integral(ShaderParticle2DBase::VertexAttribute::Count)
		};

		bool BindWorldRotationBuffer(Anvil::Buffer &buffer);
		ShaderParticleRotational(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void GetParticleSystemOrientationInfo(
			const Mat4 &matrix,const CParticleSystemComponent &particle,Vector3 &up,Vector3 &right,
			float &nearZ,float &farZ,const Material *material=nullptr,const pragma::CCameraComponent *cam=nullptr
		) const override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_base.h"

namespace Shader
{
	class DLLCLIENT Particle
		: public ParticleBase
	{
	public:
		Particle();
	};
	class DLLCLIENT ParticleUnlit
		: public ParticleBase
	{
	public:
		ParticleUnlit();
	};
};
#endif

#endif
