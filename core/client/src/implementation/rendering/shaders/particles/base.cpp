// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_descriptor_set_group.hpp>

module pragma.client;


import :rendering.shaders.particle_base;

import :engine;


using namespace pragma;

// +1 for depth pass
uint32_t ShaderParticleBase::PIPELINE_COUNT = umath::to_integral(ParticleAlphaMode::Count) + 1;
decltype(ShaderParticleBase::DESCRIPTOR_SET_ANIMATION) ShaderParticleBase::DESCRIPTOR_SET_ANIMATION = {
  "ANIMATION",
  {
    prosper::DescriptorSetInfo::Binding {"ANIMATION_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit},
  },
};
prosper::IDescriptorSet &ShaderParticleBase::GetAnimationDescriptorSet(const pragma::ecs::CParticleSystemComponent &ps)
{
	auto *animDescSet = const_cast<pragma::ecs::CParticleSystemComponent &>(ps).GetAnimationDescriptorSet();
	if(animDescSet == nullptr) {
		if(m_dummyAnimDescSetGroup == nullptr)
			m_dummyAnimDescSetGroup = pragma::get_cengine()->GetRenderContext().CreateDescriptorSetGroup(GetAnimationDescriptorSetInfo());
		animDescSet = m_dummyAnimDescSetGroup->GetDescriptorSet();
	}
	return *animDescSet;
}
struct CustomAlphaBlendMode {
	prosper::BlendOp opColor = prosper::BlendOp::Add;
	prosper::BlendOp opAlpha = prosper::BlendOp::Add;

	prosper::BlendFactor srcColorBlendFactor = prosper::BlendFactor::SrcAlpha;
	prosper::BlendFactor dstColorBlendFactor = prosper::BlendFactor::OneMinusSrcAlpha;

	prosper::BlendFactor srcAlphaBlendFactor = prosper::BlendFactor::One;
	prosper::BlendFactor dstAlphaBlendFactor = prosper::BlendFactor::OneMinusSrcAlpha;
} static g_customAlphaBlendMode {};

static prosper::BlendFactor name_to_blend_factor(const std::string &name)
{
	if(ustring::compare<std::string>(name, "zero", false))
		return prosper::BlendFactor::Zero;
	else if(ustring::compare<std::string>(name, "one", false))
		return prosper::BlendFactor::One;
	else if(ustring::compare<std::string>(name, "src_color", false))
		return prosper::BlendFactor::SrcColor;
	else if(ustring::compare<std::string>(name, "one_minus_src_color", false))
		return prosper::BlendFactor::OneMinusSrcColor;
	else if(ustring::compare<std::string>(name, "dst_color", false))
		return prosper::BlendFactor::DstColor;
	else if(ustring::compare<std::string>(name, "one_minus_dst_color", false))
		return prosper::BlendFactor::OneMinusDstColor;
	else if(ustring::compare<std::string>(name, "src_alpha", false))
		return prosper::BlendFactor::SrcAlpha;
	else if(ustring::compare<std::string>(name, "one_minus_src_alpha", false))
		return prosper::BlendFactor::OneMinusSrcAlpha;
	else if(ustring::compare<std::string>(name, "dst_alpha", false))
		return prosper::BlendFactor::DstAlpha;
	else if(ustring::compare<std::string>(name, "one_minus_dst_alpha", false))
		return prosper::BlendFactor::OneMinusDstAlpha;
	else if(ustring::compare<std::string>(name, "constant_color", false))
		return prosper::BlendFactor::ConstantColor;
	else if(ustring::compare<std::string>(name, "one_minus_constant_color", false))
		return prosper::BlendFactor::OneMinusConstantColor;
	else if(ustring::compare<std::string>(name, "constant_alpha", false))
		return prosper::BlendFactor::ConstantAlpha;
	else if(ustring::compare<std::string>(name, "one_minus_constant_alpha", false))
		return prosper::BlendFactor::OneMinusConstantAlpha;
	else if(ustring::compare<std::string>(name, "src_alpha_saturate", false))
		return prosper::BlendFactor::SrcAlphaSaturate;
	else if(ustring::compare<std::string>(name, "src1_color", false))
		return prosper::BlendFactor::Src1Color;
	else if(ustring::compare<std::string>(name, "one_minus_src1_color", false))
		return prosper::BlendFactor::OneMinusSrc1Color;
	else if(ustring::compare<std::string>(name, "src1_alpha", false))
		return prosper::BlendFactor::Src1Alpha;
	else if(ustring::compare<std::string>(name, "one_minus_src1_alpha", false))
		return prosper::BlendFactor::OneMinusSrc1Alpha;
	Con::cwar << "Invalid blend factor type '" << name << "'!" << Con::endl;
	return prosper::BlendFactor::One;
}

static prosper::BlendOp name_to_blend_op(const std::string &name)
{
	if(ustring::compare<std::string>(name, "add", false))
		return prosper::BlendOp::Add;
	else if(ustring::compare<std::string>(name, "subtract", false))
		return prosper::BlendOp::Subtract;
	else if(ustring::compare<std::string>(name, "reverse_subtract", false))
		return prosper::BlendOp::ReverseSubtract;
	else if(ustring::compare<std::string>(name, "min", false))
		return prosper::BlendOp::Min;
	else if(ustring::compare<std::string>(name, "max", false))
		return prosper::BlendOp::Max;
	Con::cwar << "Invalid blend operation '" << name << "'!" << Con::endl;
	return prosper::BlendOp::Add;
}

void Console::commands::debug_particle_alpha_mode(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	g_customAlphaBlendMode = {};
	if(argv.size() > 0)
		g_customAlphaBlendMode.srcColorBlendFactor = name_to_blend_factor(argv.at(0));
	if(argv.size() > 1)
		g_customAlphaBlendMode.dstColorBlendFactor = name_to_blend_factor(argv.at(1));
	if(argv.size() > 2)
		g_customAlphaBlendMode.srcAlphaBlendFactor = name_to_blend_factor(argv.at(2));
	if(argv.size() > 3)
		g_customAlphaBlendMode.dstAlphaBlendFactor = name_to_blend_factor(argv.at(3));
	if(argv.size() > 4)
		g_customAlphaBlendMode.opColor = name_to_blend_op(argv.at(4));
	if(argv.size() > 5)
		g_customAlphaBlendMode.opAlpha = name_to_blend_op(argv.at(5));

	for(auto &hShader : pragma::get_cengine()->GetShaderManager().GetShaders()) {
		auto *ptShader = dynamic_cast<ShaderParticleBase *>(hShader.get());
		if(ptShader == nullptr)
			continue;
		hShader->ReloadPipelines();
	}
}

void ShaderParticleBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	auto colorComponents = prosper::ColorComponentFlags::RBit | prosper::ColorComponentFlags::GBit | prosper::ColorComponentFlags::BBit | prosper::ColorComponentFlags::ABit;
	auto blendOp = prosper::BlendOp::Add;
	if(pipelineIdx == GetDepthPipelineIndex()) {
		// We only care about depth values
		colorComponents = prosper::ColorComponentFlags::None;
		blendOp = prosper::BlendOp::Add;
		pipelineInfo.SetColorBlendAttachmentProperties(0u, false, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, prosper::BlendFactor::One, prosper::BlendFactor::OneMinusSrcAlpha, colorComponents);
		return;
	}

	auto alphaMode = GetAlphaMode(pipelineIdx);
	switch(alphaMode) {
	case ParticleAlphaMode::Additive:
		{
#if 0
			pipelineInfo.SetColorBlendAttachmentProperties(
				0u,true,blendOp,blendOp,
				prosper::BlendFactor::SrcAlpha,prosper::BlendFactor::One, // color
				prosper::BlendFactor::SrcAlpha,prosper::BlendFactor::One, // alpha
				colorComponents
			);
#endif
			pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, // color
			  prosper::BlendFactor::One, prosper::BlendFactor::OneMinusSrcAlpha,                                                                               // alpha
			  colorComponents);
			break;
		}
	case ParticleAlphaMode::Opaque:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::Zero, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::Zero,                                                                          // alpha
		  colorComponents);
		break;
	case ParticleAlphaMode::Masked:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::Zero, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::Zero,                                                                          // alpha
		  colorComponents);
		break;
	case ParticleAlphaMode::Translucent:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, // color
		  prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha,                                                                          // alpha
		  colorComponents);
		break;
	case ParticleAlphaMode::AdditiveByColor:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::One, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::OneMinusSrcColor,                                                             // alpha
		  colorComponents);
		break;
	case ParticleAlphaMode::Premultiplied:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, // color
		  prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha,                                                                          // alpha
		  //static_cast<VkBlendFactor>(prosper::BlendFactor::ONE),static_cast<VkBlendFactor>(prosper::BlendFactor::ONE_MINUS_SRC_ALPHA), // color
		  //static_cast<VkBlendFactor>(prosper::BlendFactor::ONE),static_cast<VkBlendFactor>(prosper::BlendFactor::ONE_MINUS_SRC_ALPHA), // alpha
		  colorComponents);
		break;
	case ParticleAlphaMode::Custom:
		// For debug purposes only
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, g_customAlphaBlendMode.opColor, g_customAlphaBlendMode.opAlpha, g_customAlphaBlendMode.srcColorBlendFactor, g_customAlphaBlendMode.dstColorBlendFactor, g_customAlphaBlendMode.srcAlphaBlendFactor,
		  g_customAlphaBlendMode.dstAlphaBlendFactor, colorComponents);
		break;
	default:
		throw std::invalid_argument("Unknown alpha mode " + std::to_string(umath::to_integral(alphaMode)) + "!");
	}
}
static auto cvParticleQuality = GetClientConVar("cl_render_particle_quality");
uint32_t ShaderParticleBase::GetDepthPipelineIndex() { return GetParticlePipelineCount() - 1; }
ShaderParticleBase::RenderFlags ShaderParticleBase::GetRenderFlags(const pragma::ecs::CParticleSystemComponent &particle, pragma::ecs::ParticleRenderFlags ptRenderFlags) const
{
	auto renderFlags = (particle.IsAnimated() == true) ? RenderFlags::Animated : RenderFlags::None;
	if(cvParticleQuality->GetInt() <= 1)
		renderFlags |= RenderFlags::Unlit;
	if(particle.GetSoftParticles() == true)
		renderFlags |= RenderFlags::SoftParticles;
	if(particle.IsTextureScrollingEnabled())
		renderFlags |= RenderFlags::TextureScrolling;
	if(particle.GetEffectiveAlphaMode() == ParticleAlphaMode::AdditiveByColor)
		renderFlags |= RenderFlags::AdditiveBlendByColor;

	if(umath::is_flag_set(ptRenderFlags, ecs::ParticleRenderFlags::DepthOnly))
		renderFlags |= RenderFlags::DepthPass;
	return renderFlags;
}
uint32_t ShaderParticleBase::GetBasePipelineIndex(uint32_t pipelineIdx) const { return pipelineIdx / umath::to_integral(ParticleAlphaMode::Count); }
pragma::ParticleAlphaMode ShaderParticleBase::GetAlphaMode(uint32_t pipelineIdx) const { return static_cast<ParticleAlphaMode>(pipelineIdx % umath::to_integral(ParticleAlphaMode::Count)); }
pragma::ParticleAlphaMode ShaderParticleBase::GetRenderAlphaMode(const pragma::ecs::CParticleSystemComponent &particle) const
{
	if(particle.IsAlphaPremultiplied())
		return pragma::ParticleAlphaMode::Premultiplied;
	return particle.GetEffectiveAlphaMode();
}

uint32_t ShaderParticleBase::GetParticlePipelineCount() { return PIPELINE_COUNT; }
