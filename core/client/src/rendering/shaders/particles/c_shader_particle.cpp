#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"

using namespace pragma;

ShaderParticle::ShaderParticle(prosper::Context &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/vs_particle","particles/fs_particle")
{}

decltype(ShaderParticleRotational::VERTEX_BINDING_WORLD_ROTATION) ShaderParticleRotational::VERTEX_BINDING_WORLD_ROTATION = {Anvil::VertexInputRate::INSTANCE};
decltype(ShaderParticleRotational::VERTEX_ATTRIBUTE_WORLD_ROTATION) ShaderParticleRotational::VERTEX_ATTRIBUTE_WORLD_ROTATION = {VERTEX_BINDING_WORLD_ROTATION,Anvil::Format::R32G32B32A32_SFLOAT};
ShaderParticleRotational::ShaderParticleRotational(prosper::Context &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/vs_particle_rotational","particles/fs_particle")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticleRotational::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderParticle2DBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_WORLD_ROTATION);
}

bool ShaderParticleRotational::BindWorldRotationBuffer(Anvil::Buffer &buffer) {return RecordBindVertexBuffer(buffer,VERTEX_BINDING_WORLD_ROTATION.GetBindingIndex());}

void ShaderParticleRotational::GetParticleSystemOrientationInfo(
	const Mat4 &matrix,const pragma::CParticleSystemComponent &particle,Vector3 &up,Vector3 &right,
	float &nearZ,float &farZ,const Material *material,const Camera *cam
) const
{
	auto orType = particle.GetOrientationType();
	return ShaderParticle2DBase::GetParticleSystemOrientationInfo(
		matrix,particle,(orType == pragma::CParticleSystemComponent::OrientationType::Aligned) ? orType : pragma::CParticleSystemComponent::OrientationType::World,up,right,
		nearZ,farZ,material,cam
	);
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_particle.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/uniformbinding.h"
#include <textureinfo.h>
#include <cmaterial.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(Particle,particle);
LINK_SHADER_TO_CLASS(ParticleUnlit,particleunlit);

extern DLLCLIENT CGame *c_game;

Particle::Particle()
	: ParticleBase("particle","particles/vs_particle","particles/fs_particle")
{
	m_bEnableLighting = true;
}

ParticleUnlit::ParticleUnlit()
	: ParticleBase("particle","particles/vs_particle_unlit","particles/fs_particle_unlit")
{}
#endif