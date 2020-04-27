/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_base.hpp"
#include "pragma/console/c_cvar.h"
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticleBase::DESCRIPTOR_SET_ANIMATION) ShaderParticleBase::DESCRIPTOR_SET_ANIMATION = {
	{
		prosper::DescriptorSetInfo::Binding {
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
prosper::IDescriptorSet &ShaderParticleBase::GetAnimationDescriptorSet(const pragma::CParticleSystemComponent &ps)
{
	auto *animDescSet = const_cast<pragma::CParticleSystemComponent&>(ps).GetAnimationDescriptorSet();
	if(animDescSet == nullptr)
	{
		if(m_dummyAnimDescSetGroup == nullptr)
			m_dummyAnimDescSetGroup = c_engine->CreateDescriptorSetGroup(GetAnimationDescriptorSetInfo());
		animDescSet = m_dummyAnimDescSetGroup->GetDescriptorSet();
	}
	return *animDescSet;
}
void ShaderParticleBase::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	auto colorComponents = Anvil::ColorComponentFlagBits::R_BIT | Anvil::ColorComponentFlagBits::G_BIT | Anvil::ColorComponentFlagBits::B_BIT | Anvil::ColorComponentFlagBits::A_BIT;
	auto blendOp = Anvil::BlendOp::ADD;

	auto alphaMode = GetAlphaMode(pipelineIdx);
	switch(alphaMode)
	{
		case AlphaMode::Additive:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE, // color
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE, // alpha
				colorComponents
			);

			break;
		case AlphaMode::Opaque:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ZERO, // color
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ZERO, // alpha
				colorComponents
			);
			break;
		case AlphaMode::Masked:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ZERO, // color
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ZERO, // alpha
				colorComponents
			);
			break;
		case AlphaMode::Translucent:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA, // color
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA, // alpha
				colorComponents
			);
			break;
		case AlphaMode::AdditiveFull:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ONE, // color
				Anvil::BlendFactor::ONE,Anvil::BlendFactor::ONE, // alpha
				colorComponents
			);
			break;
		case AlphaMode::Premultiplied:
			pipelineInfo.set_color_blend_attachment_properties(
				0u,true,blendOp,blendOp,
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA, // color
				Anvil::BlendFactor::SRC_ALPHA,Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA, // alpha
				//static_cast<VkBlendFactor>(Anvil::BlendFactor::ONE),static_cast<VkBlendFactor>(Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA), // color
				//static_cast<VkBlendFactor>(Anvil::BlendFactor::ONE),static_cast<VkBlendFactor>(Anvil::BlendFactor::ONE_MINUS_SRC_ALPHA), // alpha
				colorComponents
			);
			break;
		default:
			throw std::invalid_argument("Unknown alpha mode " +std::to_string(umath::to_integral(alphaMode)) +"!");
	}
}
static auto cvParticleQuality = GetClientConVar("cl_render_particle_quality");
ShaderParticleBase::RenderFlags ShaderParticleBase::GetRenderFlags(const pragma::CParticleSystemComponent &particle) const
{
	auto renderFlags = (particle.IsAnimated() == true) ? RenderFlags::Animated : RenderFlags::None;
	if(cvParticleQuality->GetInt() <= 1)
		renderFlags |= RenderFlags::Unlit;
	if(particle.ShouldUseBlackAsAlpha() == true)
		renderFlags |= RenderFlags::BlackToAlpha;
	if(particle.GetSoftParticles() == true)
		renderFlags |= RenderFlags::SoftParticles;
	if(particle.IsTextureScrollingEnabled())
		renderFlags |= RenderFlags::TextureScrolling;
	return renderFlags;
}
uint32_t ShaderParticleBase::GetBasePipelineIndex(uint32_t pipelineIdx) const {return pipelineIdx /umath::to_integral(AlphaMode::Count);}
pragma::AlphaMode ShaderParticleBase::GetAlphaMode(uint32_t pipelineIdx) const {return static_cast<AlphaMode>(pipelineIdx %umath::to_integral(AlphaMode::Count));}
pragma::AlphaMode ShaderParticleBase::GetRenderAlphaMode(const pragma::CParticleSystemComponent &particle) const
{
	if(particle.IsAlphaPremultiplied())
		return pragma::AlphaMode::Premultiplied;
	return particle.GetAlphaMode();
}

uint32_t ShaderParticleBase::GetParticlePipelineCount() const {return umath::to_integral(AlphaMode::Count) *umath::to_integral(pragma::ShaderScene::Pipeline::Count);}
