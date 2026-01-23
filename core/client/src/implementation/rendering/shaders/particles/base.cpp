// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.particle_base;

import :engine;

using namespace pragma;

// +1 for depth pass
uint32_t ShaderParticleBase::PIPELINE_COUNT = math::to_integral(rendering::ParticleAlphaMode::Count) + 1;
decltype(ShaderParticleBase::DESCRIPTOR_SET_ANIMATION) ShaderParticleBase::DESCRIPTOR_SET_ANIMATION = {
  "ANIMATION",
  {
    prosper::DescriptorSetInfo::Binding {"ANIMATION_DATA", prosper::DescriptorType::StorageBuffer, prosper::ShaderStageFlags::FragmentBit},
  },
};
prosper::IDescriptorSet &ShaderParticleBase::GetAnimationDescriptorSet(const ecs::CParticleSystemComponent &ps)
{
	auto *animDescSet = const_cast<ecs::CParticleSystemComponent &>(ps).GetAnimationDescriptorSet();
	if(animDescSet == nullptr) {
		if(m_dummyAnimDescSetGroup == nullptr)
			m_dummyAnimDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(GetAnimationDescriptorSetInfo());
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
	if(pragma::string::compare<std::string>(name, "zero", false))
		return prosper::BlendFactor::Zero;
	else if(pragma::string::compare<std::string>(name, "one", false))
		return prosper::BlendFactor::One;
	else if(pragma::string::compare<std::string>(name, "src_color", false))
		return prosper::BlendFactor::SrcColor;
	else if(pragma::string::compare<std::string>(name, "one_minus_src_color", false))
		return prosper::BlendFactor::OneMinusSrcColor;
	else if(pragma::string::compare<std::string>(name, "dst_color", false))
		return prosper::BlendFactor::DstColor;
	else if(pragma::string::compare<std::string>(name, "one_minus_dst_color", false))
		return prosper::BlendFactor::OneMinusDstColor;
	else if(pragma::string::compare<std::string>(name, "src_alpha", false))
		return prosper::BlendFactor::SrcAlpha;
	else if(pragma::string::compare<std::string>(name, "one_minus_src_alpha", false))
		return prosper::BlendFactor::OneMinusSrcAlpha;
	else if(pragma::string::compare<std::string>(name, "dst_alpha", false))
		return prosper::BlendFactor::DstAlpha;
	else if(pragma::string::compare<std::string>(name, "one_minus_dst_alpha", false))
		return prosper::BlendFactor::OneMinusDstAlpha;
	else if(pragma::string::compare<std::string>(name, "constant_color", false))
		return prosper::BlendFactor::ConstantColor;
	else if(pragma::string::compare<std::string>(name, "one_minus_constant_color", false))
		return prosper::BlendFactor::OneMinusConstantColor;
	else if(pragma::string::compare<std::string>(name, "constant_alpha", false))
		return prosper::BlendFactor::ConstantAlpha;
	else if(pragma::string::compare<std::string>(name, "one_minus_constant_alpha", false))
		return prosper::BlendFactor::OneMinusConstantAlpha;
	else if(pragma::string::compare<std::string>(name, "src_alpha_saturate", false))
		return prosper::BlendFactor::SrcAlphaSaturate;
	else if(pragma::string::compare<std::string>(name, "src1_color", false))
		return prosper::BlendFactor::Src1Color;
	else if(pragma::string::compare<std::string>(name, "one_minus_src1_color", false))
		return prosper::BlendFactor::OneMinusSrc1Color;
	else if(pragma::string::compare<std::string>(name, "src1_alpha", false))
		return prosper::BlendFactor::Src1Alpha;
	else if(pragma::string::compare<std::string>(name, "one_minus_src1_alpha", false))
		return prosper::BlendFactor::OneMinusSrc1Alpha;
	Con::CWAR << "Invalid blend factor type '" << name << "'!" << Con::endl;
	return prosper::BlendFactor::One;
}

static prosper::BlendOp name_to_blend_op(const std::string &name)
{
	if(pragma::string::compare<std::string>(name, "add", false))
		return prosper::BlendOp::Add;
	else if(pragma::string::compare<std::string>(name, "subtract", false))
		return prosper::BlendOp::Subtract;
	else if(pragma::string::compare<std::string>(name, "reverse_subtract", false))
		return prosper::BlendOp::ReverseSubtract;
	else if(pragma::string::compare<std::string>(name, "min", false))
		return prosper::BlendOp::Min;
	else if(pragma::string::compare<std::string>(name, "max", false))
		return prosper::BlendOp::Max;
	Con::CWAR << "Invalid blend operation '" << name << "'!" << Con::endl;
	return prosper::BlendOp::Add;
}

static void debug_particle_alpha_mode(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
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

	for(auto &hShader : get_cengine()->GetShaderManager().GetShaders()) {
		auto *ptShader = dynamic_cast<ShaderParticleBase *>(hShader.get());
		if(ptShader == nullptr)
			continue;
		hShader->ReloadPipelines();
	}
}
namespace {
	auto UVN = console::client::register_command("debug_particle_alpha_mode", &debug_particle_alpha_mode, console::ConVarFlags::None, "Specifies the blend mode arguments for particle systems that use the \
	'custom' alpha mode. Argument order: <srcColorBlendFactor> <dstColorBlendFactor> <srcAlphaBlendFactor> <dstAlphaBlendFactor> <opColor> <opAlpha>.\n\
	Blend factor options: zero, one, src_color, one_minus_src_color, dst_color, one_minus_dst_color, src_alpha, one_minus_src_alpha, dst_alpha, one_minus_dst_alpha, constant_color, \
	one_minus_constant_color, constant_alpha, one_minus_constant_alpha, src_alpha_saturate, src1_color, one_minus_src1_color, src1_alpha, one_minus_src1_alpha\n\
	Operation options: add, subtract, reverse_subtract, min, max");
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
	case rendering::ParticleAlphaMode::Additive:
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
	case rendering::ParticleAlphaMode::Opaque:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::Zero, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::Zero,                                                                          // alpha
		  colorComponents);
		break;
	case rendering::ParticleAlphaMode::Masked:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::Zero, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::Zero,                                                                          // alpha
		  colorComponents);
		break;
	case rendering::ParticleAlphaMode::Translucent:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, // color
		  prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha,                                                                          // alpha
		  colorComponents);
		break;
	case rendering::ParticleAlphaMode::AdditiveByColor:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::One, prosper::BlendFactor::One, // color
		  prosper::BlendFactor::One, prosper::BlendFactor::OneMinusSrcColor,                                                             // alpha
		  colorComponents);
		break;
	case rendering::ParticleAlphaMode::Premultiplied:
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, blendOp, blendOp, prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha, // color
		  prosper::BlendFactor::SrcAlpha, prosper::BlendFactor::OneMinusSrcAlpha,                                                                          // alpha
		  //static_cast<VkBlendFactor>(prosper::BlendFactor::ONE),static_cast<VkBlendFactor>(prosper::BlendFactor::ONE_MINUS_SRC_ALPHA), // color
		  //static_cast<VkBlendFactor>(prosper::BlendFactor::ONE),static_cast<VkBlendFactor>(prosper::BlendFactor::ONE_MINUS_SRC_ALPHA), // alpha
		  colorComponents);
		break;
	case rendering::ParticleAlphaMode::Custom:
		// For debug purposes only
		pipelineInfo.SetColorBlendAttachmentProperties(0u, true, g_customAlphaBlendMode.opColor, g_customAlphaBlendMode.opAlpha, g_customAlphaBlendMode.srcColorBlendFactor, g_customAlphaBlendMode.dstColorBlendFactor, g_customAlphaBlendMode.srcAlphaBlendFactor,
		  g_customAlphaBlendMode.dstAlphaBlendFactor, colorComponents);
		break;
	default:
		throw std::invalid_argument("Unknown alpha mode " + std::to_string(math::to_integral(alphaMode)) + "!");
	}
}
static auto cvParticleQuality = console::get_client_con_var("cl_render_particle_quality");
uint32_t ShaderParticleBase::GetDepthPipelineIndex() { return GetParticlePipelineCount() - 1; }
ShaderParticleBase::RenderFlags ShaderParticleBase::GetRenderFlags(const ecs::CParticleSystemComponent &particle, pts::ParticleRenderFlags ptRenderFlags) const
{
	auto renderFlags = (particle.IsAnimated() == true) ? RenderFlags::Animated : RenderFlags::None;
	if(cvParticleQuality->GetInt() <= 1)
		renderFlags |= RenderFlags::Unlit;
	if(particle.GetSoftParticles() == true)
		renderFlags |= RenderFlags::SoftParticles;
	if(particle.IsTextureScrollingEnabled())
		renderFlags |= RenderFlags::TextureScrolling;
	if(particle.GetEffectiveAlphaMode() == rendering::ParticleAlphaMode::AdditiveByColor)
		renderFlags |= RenderFlags::AdditiveBlendByColor;

	if(math::is_flag_set(ptRenderFlags, pts::ParticleRenderFlags::DepthOnly))
		renderFlags |= RenderFlags::DepthPass;
	return renderFlags;
}
uint32_t ShaderParticleBase::GetBasePipelineIndex(uint32_t pipelineIdx) const { return pipelineIdx / math::to_integral(rendering::ParticleAlphaMode::Count); }
rendering::ParticleAlphaMode ShaderParticleBase::GetAlphaMode(uint32_t pipelineIdx) const { return static_cast<rendering::ParticleAlphaMode>(pipelineIdx % math::to_integral(rendering::ParticleAlphaMode::Count)); }
rendering::ParticleAlphaMode ShaderParticleBase::GetRenderAlphaMode(const ecs::CParticleSystemComponent &particle) const
{
	if(particle.IsAlphaPremultiplied())
		return rendering::ParticleAlphaMode::Premultiplied;
	return particle.GetEffectiveAlphaMode();
}

uint32_t ShaderParticleBase::GetParticlePipelineCount() { return PIPELINE_COUNT; }
