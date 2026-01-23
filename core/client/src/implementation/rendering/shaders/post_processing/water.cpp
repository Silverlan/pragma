// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.pp_water;

import :engine;

using namespace pragma;

decltype(ShaderPPWater::DESCRIPTOR_SET_TEXTURE) ShaderPPWater::DESCRIPTOR_SET_TEXTURE = {&ShaderPPFog::DESCRIPTOR_SET_TEXTURE};
decltype(ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER) ShaderPPWater::DESCRIPTOR_SET_DEPTH_BUFFER = {&ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER};
decltype(ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP) ShaderPPWater::DESCRIPTOR_SET_REFRACTION_MAP = {
  "REFRACTION",
  {prosper::DescriptorSetInfo::Binding {"MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderPPWater::DESCRIPTOR_SET_SCENE) ShaderPPWater::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderPPWater::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderPPWater::DESCRIPTOR_SET_FOG) ShaderPPWater::DESCRIPTOR_SET_FOG = {&ShaderPPFog::DESCRIPTOR_SET_FOG};
ShaderPPWater::ShaderPPWater(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBase(context, identifier, "programs/post_processing/water") { SetBaseShader<prosper::ShaderCopyImage>(); }

void ShaderPPWater::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();
	AddDescriptorSetGroup(DESCRIPTOR_SET_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_DEPTH_BUFFER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_REFRACTION_MAP);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_FOG);
	AddDefaultVertexAttributes();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}
void ShaderPPWater::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderPPWater::InitializeMaterialDescriptorSet(material::CMaterial &mat)
{
	auto *dudvMap = mat.GetTextureInfo(material::ematerial::DUDV_MAP_IDENTIFIER);
	if(dudvMap == nullptr || dudvMap->texture == nullptr)
		return nullptr;
	auto descSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_REFRACTION_MAP);
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	auto texture = std::static_pointer_cast<material::Texture>(dudvMap->texture);
	if(texture->HasValidVkTexture())
		descSet.SetBindingTexture(*texture->GetVkTexture(), 0u);
	return descSetGroup;
}

bool ShaderPPWater::RecordRefractionMaterial(prosper::ShaderBindState &bindState, material::CMaterial &mat) const
{
	auto descSetGroup = mat.GetDescriptorSetGroup(const_cast<ShaderPPWater &>(*this));
	//if(descSetGroup == nullptr)
	//	descSetGroup = InitializeMaterialDescriptorSet(mat); // Attempt to initialize on the fly
	if(descSetGroup == nullptr)
		return false;
	return RecordBindDescriptorSet(bindState, *descSetGroup->GetDescriptorSet(), DESCRIPTOR_SET_REFRACTION_MAP.setIndex);
}

bool ShaderPPWater::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, prosper::IDescriptorSet &descSetDepth, prosper::IDescriptorSet &descSetCamera, prosper::IDescriptorSet &descSetTime, prosper::IDescriptorSet &descSetFog,
  const Vector4 &clipPlane) const
{
	return RecordBindDescriptorSet(bindState, descSetDepth, DESCRIPTOR_SET_DEPTH_BUFFER.setIndex) && RecordBindDescriptorSets(bindState, {&descSetCamera, &descSetTime, &descSetFog}, DESCRIPTOR_SET_SCENE.setIndex) && RecordPushConstants(bindState, clipPlane)
	  && ShaderPPBase::RecordDraw(bindState, descSetTexture);
}
