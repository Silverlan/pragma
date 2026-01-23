// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.particle_blob;

import :engine;
import :entities.components;
import :game;

using namespace pragma;

decltype(ShaderParticleBlob::VERTEX_BINDING_BLOB_NEIGHBORS) ShaderParticleBlob::VERTEX_BINDING_BLOB_NEIGHBORS = {prosper::VertexInputRate::Instance, MAX_BLOB_NEIGHBORS * sizeof(uint16_t)};
decltype(ShaderParticleBlob::VERTEX_ATTRIBUTE_BLOB_NEIGHBORS) ShaderParticleBlob::VERTEX_ATTRIBUTE_BLOB_NEIGHBORS = {VERTEX_BINDING_BLOB_NEIGHBORS, prosper::Format::R32G32B32A32_UInt};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA) ShaderParticleBlob::DESCRIPTOR_SET_PARTICLE_DATA = {
  "PARTICLES",
  {prosper::DescriptorSetInfo::Binding {"DATA", prosper::DescriptorType::StorageBufferDynamic, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_SCENE) ShaderParticleBlob::DESCRIPTOR_SET_SCENE = {&ShaderParticle2DBase::DESCRIPTOR_SET_SCENE};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_RENDERER) ShaderParticleBlob::DESCRIPTOR_SET_RENDERER = {&ShaderParticle2DBase::DESCRIPTOR_SET_RENDERER};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderParticleBlob::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderParticle2DBase::DESCRIPTOR_SET_RENDER_SETTINGS};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_SHADOWS) ShaderParticleBlob::DESCRIPTOR_SET_SHADOWS = {&ShaderParticle2DBase::DESCRIPTOR_SET_SHADOWS};
decltype(ShaderParticleBlob::DESCRIPTOR_SET_PBR) ShaderParticleBlob::DESCRIPTOR_SET_PBR = {&ShaderPBR::DESCRIPTOR_SET_PBR};

static std::shared_ptr<rendering::shader_material::ShaderMaterial> get_shader_material() { return rendering::shader_material::get_cache().Load("particle_blob"); }

ShaderParticleBlob::ShaderParticleBlob(prosper::IPrContext &context, const std::string &identifier) : ShaderParticle2DBase(context, identifier, "programs/particles/blob/particle_blob", "programs/particles/blob/particle_blob")
{
	SetBaseShader<ShaderParticle>();
	m_shaderMaterial = rendering::shader_material::get_cache().Load("particle_blob");
	m_materialDescSetInfo = ShaderGameWorldLightingPass::CreateMaterialDescriptorSetInfo(*m_shaderMaterial);
}

uint32_t ShaderParticleBlob::GetSceneDescriptorSetIndex() const
{
	return DESCRIPTOR_SET_SCENE.setIndex; // TODO
}

void ShaderParticleBlob::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	ShaderParticle2DBase::GetShaderPreprocessorDefinitions(outDefinitions, outPrefixCode);
	if(m_shaderMaterial)
		outPrefixCode += m_shaderMaterial->ToGlslStruct();
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderParticleBlob::InitializeMaterialDescriptorSet(material::CMaterial &mat)
{
	auto descSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(*m_materialDescSetInfo);
	if(!descSetGroup)
		return nullptr;
	rendering::ShaderInputData materialData {*m_shaderMaterial};
	rendering::shader_material::ShaderMaterial::PopulateShaderInputDataFromMaterial(materialData, mat);
	if(!ShaderGameWorldLightingPass::InitializeMaterialBuffer(*descSetGroup->GetDescriptorSet(), mat, materialData, 0u))
		return nullptr;
	mat.SetDescriptorSetGroup(*this, descSetGroup);
	return descSetGroup;
}

bool ShaderParticleBlob::RecordParticleMaterial(prosper::ShaderBindState &bindState, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps) const
{
	auto *mat = static_cast<material::CMaterial *>(ps.GetMaterial());
	if(mat == nullptr)
		return false;
	auto descSetGroupMat = mat->GetDescriptorSetGroup(const_cast<ShaderParticleBlob &>(*this));
	// if(descSetGroupMat == nullptr)
	// 	descSetGroupMat = const_cast<ShaderParticleBlob*>(this)->InitializeMaterialDescriptorSet(*mat); // Attempt to initialize on the fly
	if(descSetGroupMat == nullptr)
		return false;
	auto &descSetTexture = *descSetGroupMat->GetDescriptorSet();
	auto *descSetDepth = renderer.GetDepthDescriptorSet();
	if(descSetDepth == nullptr)
		return false;
	auto &animDescSet = const_cast<ShaderParticleBlob *>(this)->GetAnimationDescriptorSet(const_cast<ecs::CParticleSystemComponent &>(ps));
	return RecordBindDescriptorSets(bindState, {&descSetTexture}, m_materialDescSetInfo->setIndex);
}

void ShaderParticleBlob::InitializeShaderResources()
{
	ShaderSceneLit::InitializeShaderResources();

	RegisterDefaultGfxPipelineVertexAttributes();
	AddVertexAttribute(VERTEX_ATTRIBUTE_BLOB_NEIGHBORS);

	RegisterDefaultGfxPipelinePushConstantRanges();

	AddDescriptorSetGroup(DESCRIPTOR_SET_PARTICLE_DATA);
	AddDescriptorSetGroup(*m_materialDescSetInfo);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDERER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_RENDER_SETTINGS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SHADOWS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_PBR);
}

void ShaderParticleBlob::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo, basePipelineIdx);

	pipelineInfo.ToggleDepthWrites(pipelineIdx == GetDepthPipelineIndex()); // Last pipeline is depth pipeline

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
}

bool ShaderParticleBlob::RecordBindScene(prosper::ICommandBuffer &cmd, const prosper::IShaderPipelineLayout &layout, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsShadows) const
{
	return ShaderParticle2DBase::RecordBindScene(cmd, layout, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows);
}

bool ShaderParticleBlob::RecordDraw(prosper::ShaderBindState &bindState, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const ecs::CParticleSystemComponent &ps, pts::ParticleOrientationType orientationType,
  pts::ParticleRenderFlags ptRenderFlags, prosper::IBuffer &blobIndexBuffer, prosper::IDescriptorSet &dsParticles, uint32_t particleBufferOffset)
{
	if(RecordParticleMaterial(bindState, renderer, ps) == false)
		return false;
	auto &cam = scene.GetActiveCamera();

	auto colorFactor = scene.GetParticleSystemColorFactor();
	if(math::is_flag_set(ptRenderFlags, pts::ParticleRenderFlags::Bloom)) {
		auto bloomColorFactor = ps.GetEffectiveBloomColorFactor();
		if(bloomColorFactor.has_value())
			colorFactor *= *bloomColorFactor;
	}
	else {
		auto *mat = ps.GetMaterial();
		auto &psColorFactor = ps.GetColorFactor();
		colorFactor *= psColorFactor;
		if(mat) {
			Vector4 matColorFactor;
			if(mat->GetProperty("color_factor", &matColorFactor))
				colorFactor *= matColorFactor;
		}
	}

	auto renderFlags = GetRenderFlags(ps, ptRenderFlags);
	auto width = get_cengine()->GetRenderContext().GetWindowWidth();
	auto height = get_cengine()->GetRenderContext().GetWindowHeight();
	assert(width <= std::numeric_limits<uint16_t>::max() && height <= std::numeric_limits<uint16_t>::max());

	auto viewportSize = static_cast<uint32_t>(width);
	viewportSize <<= 16;
	viewportSize |= height;
	PushConstants pushConstants {colorFactor, Vector3 {}, /* camRightWs */
	  math::to_integral(orientationType), Vector3 {},                          /* camUpWs */
	  0.f,                                                                      /* nearZ */
	  cam.valid() ? cam->GetEntity().GetPosition() : Vector3 {}, 0.f,           /* farZ */
	  viewportSize, math::to_integral(renderFlags), math::to_integral(ps.GetAlphaMode()), ps.GetSimulationTime()};
	Mat4 vp;
	if(cam.valid()) {
		auto &v = cam->GetViewMatrix();
		vp = cam->GetProjectionMatrix() * v;
	}
	GetParticleSystemOrientationInfo(vp, ps, pushConstants.camUpWs, pushConstants.camRightWs, pushConstants.nearZ, pushConstants.farZ, ps.GetMaterial(), cam.get());

	if(RecordPushConstants(bindState, pushConstants) == false)
		return false;
	if(RecordBindDescriptorSet(bindState, dsParticles, DESCRIPTOR_SET_PARTICLE_DATA.setIndex, {particleBufferOffset}) == false)
		return false;
	auto &shaderPbr = get_cgame()->GetGameShader(CGame::GameShader::Pbr);
	assert(shaderPbr.valid());

	float iblStrength = 1.f;
	auto sceneFlags = ShaderGameWorld::SceneFlags::None;
	auto *dsPbr = static_cast<ShaderPBR *>(shaderPbr.get())->GetReflectionProbeDescriptorSet(scene, iblStrength, sceneFlags);
	assert(dsPbr != nullptr);
	if(RecordBindDescriptorSet(bindState, *dsPbr, DESCRIPTOR_SET_PBR.setIndex) == false)
		return false;
	auto bAnimated = ((renderFlags & RenderFlags::Animated) != RenderFlags::None) ? true : false;

	auto ptAnimBuffer = ps.GetParticleAnimationBuffer();
	if(ptAnimBuffer == nullptr)
		ptAnimBuffer = get_cengine()->GetRenderContext().GetDummyBuffer();
	return RecordBindVertexBuffers(bindState, {ps.GetParticleBuffer().get(), ptAnimBuffer.get(), &blobIndexBuffer}) == true && ShaderSceneLit::RecordDraw(bindState, ecs::CParticleSystemComponent::VERTEX_COUNT, ps.GetRenderParticleCount()) == true;
}
