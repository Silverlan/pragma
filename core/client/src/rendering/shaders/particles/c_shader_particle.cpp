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
	float &nearZ,float &farZ,const Material *material,const pragma::CCameraComponent *cam
) const
{
	auto orType = particle.GetOrientationType();
	return ShaderParticle2DBase::GetParticleSystemOrientationInfo(
		matrix,particle,(orType == pragma::CParticleSystemComponent::OrientationType::Aligned) ? orType : pragma::CParticleSystemComponent::OrientationType::World,up,right,
		nearZ,farZ,material,cam
	);
}
