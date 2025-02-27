/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_motion_blur_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_motion_blur_data_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/rendering/shaders/info/c_shader_velocity_buffer.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_motion_blur.hpp"
#include "pragma/rendering/render_processor.hpp"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_buffer_create_info.hpp>
#include <buffers/prosper_swap_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
#include <wgui/types/wirect.h>
#endif

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

class DLLCLIENT VelocityStageRenderProcessor : public pragma::rendering::DepthStageRenderProcessor {
  public:
	VelocityStageRenderProcessor(pragma::ShaderVelocityBuffer &shaderVelocity, const util::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin, const pragma::MotionBlurTemporalData &motionBlurData, prosper::IDescriptorSet &dsMotionData, prosper::IDescriptorSet &dsBoneGeneric);
	virtual bool BindEntity(CBaseEntity &ent) override;
	virtual bool BindShader(prosper::Shader &shader, uint32_t pipelineIdx = 0u) override;
  private:
	pragma::ShaderVelocityBuffer &m_shaderVelocity;
	const pragma::MotionBlurTemporalData &m_motionBlurData;
	prosper::IDescriptorSet &m_dsMotionData;
	prosper::IDescriptorSet &m_dsBoneGeneric;
};
VelocityStageRenderProcessor::VelocityStageRenderProcessor(pragma::ShaderVelocityBuffer &shaderVelocity, const util::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin, const pragma::MotionBlurTemporalData &motionBlurData, prosper::IDescriptorSet &dsMotionData,
  prosper::IDescriptorSet &dsBoneGeneric)
    : pragma::rendering::DepthStageRenderProcessor {drawSceneInfo, drawOrigin}, m_shaderVelocity {shaderVelocity}, m_motionBlurData {motionBlurData}, m_dsMotionData {dsMotionData}, m_dsBoneGeneric {dsBoneGeneric}
{
}
bool VelocityStageRenderProcessor::BindShader(prosper::Shader &shader, uint32_t pipelineIdx)
{
	if(!DepthStageRenderProcessor::BindShader(shader, pipelineIdx))
		return false;
	auto &cmd = m_shaderProcessor.GetCommandBuffer();
	return cmd.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, m_shaderProcessor.GetCurrentPipelineLayout(), pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex, m_dsMotionData);
}
bool VelocityStageRenderProcessor::BindEntity(CBaseEntity &ent)
{
	auto res = pragma::rendering::DepthStageRenderProcessor::BindEntity(ent);
	if(res == false)
		return false;
	auto *dsBone = &m_dsBoneGeneric;
	auto it = m_motionBlurData.prevModelMatrices.find(&ent);
	if(it != m_motionBlurData.prevModelMatrices.end() && it->second.boneDsg != nullptr)
		dsBone = it->second.boneDsg->GetDescriptorSet();
	if(!m_shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, m_shaderProcessor.GetCurrentPipelineLayout(), pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex, *dsBone))
		return false;

	pragma::ShaderVelocityBuffer::MotionBlurPushConstants pushConstants {};
	if(it != m_motionBlurData.prevModelMatrices.end())
		pushConstants.prevPose = it->second.matrix;
	else
		pushConstants.prevPose = umat::identity();
	return m_shaderProcessor.GetCommandBuffer().RecordPushConstants(m_shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit, pragma::ShaderVelocityBuffer::MotionBlurPushConstants::Offset, sizeof(pushConstants),
	  &pushConstants);
}

void CRendererPpMotionBlurComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRendererPpMotionBlurComponent;

	{
		using TMotionBlurIntensity = float;
		auto memberInfo = create_component_member_info<T, TMotionBlurIntensity, static_cast<void (T::*)(TMotionBlurIntensity)>(&T::SetMotionBlurIntensity), static_cast<TMotionBlurIntensity (T::*)() const>(&T::GetMotionBlurIntensity)>("motionBlurIntensity", 4.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(10.f);
		registerMember(std::move(memberInfo));
	}
}
CRendererPpMotionBlurComponent::CRendererPpMotionBlurComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
	static auto g_shadersRegistered = false;
	if(!g_shadersRegistered) {
		g_shadersRegistered = true;
		c_engine->GetShaderManager().RegisterShader("velocity_buffer", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderVelocityBuffer(context, identifier); });
		c_engine->GetShaderManager().RegisterShader("pp_motion_blur", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderPPMotionBlur(context, identifier); });
	}
}
void CRendererPpMotionBlurComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererPpMotionBlurComponent::ReloadVelocityTexture()
{
	auto *shader = pragma::get_velocity_buffer_shader();
	if(!shader)
		return;
	auto rendererC = GetEntity().GetComponent<pragma::CRendererComponent>();
	auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rendererC.expired() || cRenderer.expired())
		return;
	c_engine->GetRenderContext().WaitIdle();

	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = rendererC->GetWidth();
	createInfo.height = rendererC->GetHeight();
	createInfo.format = prosper::Format::R32G32B32A32_SFloat;
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;

	auto &context = c_engine->GetRenderContext();
	auto img = context.CreateImage(createInfo);
	auto tex = context.CreateTexture({}, *img, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	auto rt = context.CreateRenderTarget({tex, cRenderer->GetPrepass().textureDepth}, shader->GetRenderPass());
	m_renderTarget = rt;

	m_velocityTexDsg->GetDescriptorSet()->SetBindingTexture(*tex, 0);
	m_valid = true;

#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
	auto *el = WGUI::GetInstance().Create<WITexturedRect>();
	el->SetTexture(*tex);
	el->SetSize(512, 512);
	if(m_debugTex.IsValid())
		m_debugTex.Remove();
	m_debugTex = el->GetHandle();
	luabind::globals(c_game->GetLuaState())["_el"] = el->GetHandle();
#endif
}

void CRendererPpMotionBlurComponent::Initialize()
{
	CRendererPpBaseComponent::Initialize();

	auto *velShader = pragma::get_velocity_buffer_shader();
	auto *shaderMotionBlur = static_cast<pragma::ShaderPPMotionBlur *>(c_engine->GetShader("pp_motion_blur").get());
	if(!velShader || !shaderMotionBlur)
		return;
	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(!m_valid)
			return;
		RecordVelocityPass(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_POST_PREPASS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(!m_valid)
			return;
		ExecuteVelocityPass(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(pragma::CRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_valid = false;
		auto evDataReloaded = static_cast<CEOnRenderTargetReloaded &>(evData.get());
		if(!evDataReloaded.success)
			return;
		ReloadVelocityTexture();
	});

	auto &context = c_engine->GetRenderContext();
	m_swapCmd = context.CreateSwapCommandBufferGroup(context.GetWindow());

	m_velocityTexDsg = shaderMotionBlur->CreateDescriptorSetGroup(pragma::ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE_VELOCITY.setIndex);

	prosper::util::BufferCreateInfo motionBlurBufCreateInfo {};
	motionBlurBufCreateInfo.size = sizeof(MotionBlurCameraData);
	motionBlurBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	auto buf = context.CreateBuffer(motionBlurBufCreateInfo);
	m_motionBlurDataDsg = velShader->CreateDescriptorSetGroup(pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex);
	m_motionBlurDataDsg->GetDescriptorSet()->SetBindingUniformBuffer(*buf, 0);
	m_motionBlurDataBuffer = buf;

	m_genericBoneDsg = velShader->CreateDescriptorSetGroup(pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex);

	ReloadVelocityTexture();
}

void CRendererPpMotionBlurComponent::RecordVelocityPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto *velShader = pragma::get_velocity_buffer_shader();
	if(!velShader)
		return;
	auto &cam = drawSceneInfo.scene->GetActiveCamera();
	if(cam.expired())
		return;
	auto motionBlurDataC = cam->GetEntity().GetComponent<CMotionBlurDataComponent>();
	if(motionBlurDataC.expired())
		return;
	auto &cmd = GetSwapCommandBuffer();
	auto &rt = GetRenderTarget();
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
	auto &motionBlurTempData = motionBlurDataC->GetMotionBlurData();

	drawSceneInfo.commandBuffer->RecordBufferBarrier(*m_motionBlurDataBuffer, prosper::PipelineStageFlags::VertexShaderBit, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::UniformReadBit, prosper::AccessFlags::TransferWriteBit);
	drawSceneInfo.commandBuffer->RecordUpdateBuffer(*m_motionBlurDataBuffer, 0u, motionBlurTempData.cameraData);
	drawSceneInfo.commandBuffer->RecordBufferBarrier(*m_motionBlurDataBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::VertexShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::UniformReadBit);

	DoUpdatePoses(*motionBlurDataC, motionBlurTempData, *drawSceneInfo.commandBuffer);

	cmd->StartRecording(rt->GetRenderPass(), rt->GetFramebuffer());
	cmd->Record([this, &drawSceneInfo, velShader, &worldRenderQueues, &sceneRenderDesc, &motionBlurTempData](prosper::ISecondaryCommandBuffer &cmd) {
		util::RenderPassDrawInfo renderPassDrawInfo {drawSceneInfo, cmd};
		VelocityStageRenderProcessor rsys {*velShader, renderPassDrawInfo, {} /* drawOrigin */, motionBlurTempData, *m_motionBlurDataDsg->GetDescriptorSet(), *m_genericBoneDsg->GetDescriptorSet()};

		rsys.BindShader(*velShader, umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
		// Render static world geometry
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->SetTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);
			for(auto i = decltype(worldRenderQueues.size()) {0u}; i < worldRenderQueues.size(); ++i)
				rsys.Render(*worldRenderQueues.at(i), pragma::rendering::RenderPass::Prepass, nullptr, i);
		}

		// Note: The non-translucent render queues also include transparent (alpha masked) objects.
		// We don't care about translucent objects here.
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, false /* translucent */), pragma::rendering::RenderPass::Prepass);

			auto &queueTranslucent = *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, true /* translucent */);
			queueTranslucent.WaitForCompletion();
			if(queueTranslucent.queue.empty() == false) {
				// rsys.BindShader(shaderPrepass,umath::to_integral(pragma::ShaderPrepass::Pipeline::AlphaTest));
				rsys.Render(queueTranslucent, pragma::rendering::RenderPass::Prepass);
			}
		}
		rsys.UnbindShader();
	});
	cmd->EndRecording();
}
void CRendererPpMotionBlurComponent::DoUpdatePoses(const CMotionBlurDataComponent &motionBlurDataC, const MotionBlurTemporalData &motionBlurData, prosper::IPrimaryCommandBuffer &cmd)
{
	auto lastUpdateIdx = motionBlurDataC.GetLastUpdateIndex();
	if(m_lastMotionDataBufferUpdateIndex == lastUpdateIdx)
		return;
	m_lastMotionDataBufferUpdateIndex = lastUpdateIdx;

	auto *velShader = pragma::get_velocity_buffer_shader();
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *ent : entIt) {
		auto &r = *static_cast<CBaseEntity *>(ent)->GetRenderComponent();
		auto curPose = r.GetTransformationMatrix();
		auto it = motionBlurData.curModelMatrices.find(ent);

		auto *animC = static_cast<CAnimatedComponent *>(ent->GetAnimatedComponent().get());
		auto curBoneBuffer = animC ? animC->GetBoneBuffer() : nullptr;
		if(curBoneBuffer) {
			auto it = motionBlurData.prevModelMatrices.find(ent);
			if(it != motionBlurData.prevModelMatrices.end()) {
				auto &bufferDst = *it->second.boneBuffer;
				auto &bufferSrc = const_cast<prosper::IBuffer &>(*curBoneBuffer);
				cmd.RecordBufferBarrier(bufferSrc, prosper::PipelineStageFlags::VertexShaderBit, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::UniformReadBit, prosper::AccessFlags::TransferReadBit);
				cmd.RecordBufferBarrier(bufferDst, prosper::PipelineStageFlags::VertexShaderBit, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::UniformReadBit, prosper::AccessFlags::TransferWriteBit);
				prosper::util::BufferCopy cpyInfo {};
				cpyInfo.size = bufferSrc.GetSize();
				cmd.RecordCopyBuffer(cpyInfo, bufferSrc, bufferDst);
				cmd.RecordBufferBarrier(bufferSrc, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::VertexShaderBit, prosper::AccessFlags::TransferReadBit, prosper::AccessFlags::UniformReadBit);
				cmd.RecordBufferBarrier(bufferDst, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::VertexShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::UniformReadBit);
			}
		}
	}
}
void CRendererPpMotionBlurComponent::OnRemove()
{
	CRendererPpBaseComponent::OnRemove();
#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
	if(m_debugTex.IsValid())
		m_debugTex.Remove();
#endif
}
void CRendererPpMotionBlurComponent::ExecuteVelocityPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto *velShader = pragma::get_velocity_buffer_shader();
	if(!velShader)
		return;
	auto *renderer = drawSceneInfo.scene->GetRenderer();
	assert(renderer != nullptr);
	auto motionBlurC = renderer->GetEntity().GetComponent<CRendererPpMotionBlurComponent>();
	if(motionBlurC.expired())
		return;

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &rt = motionBlurC->GetRenderTarget();
	drawCmd->RecordBeginRenderPass(*rt, std::vector<prosper::ClearValue> {prosper::ClearDepthStencilValue {}}, prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers);

	auto &cmd = motionBlurC->GetSwapCommandBuffer();
	cmd->ExecuteCommands(*drawCmd);

	drawCmd->RecordEndRenderPass();
}
void CRendererPpMotionBlurComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) { RenderPostProcessing(drawSceneInfo); }
void CRendererPpMotionBlurComponent::RenderPostProcessing(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto *shaderMotionBlur = static_cast<pragma::ShaderPPMotionBlur *>(c_engine->GetShader("pp_motion_blur").get());

	if(!shaderMotionBlur)
		return;
	auto rasterC = scene.GetRenderer()->GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.expired())
		return;
	auto &hdrInfo = rasterC->GetHDRInfo();
	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	drawCmd->RecordBufferBarrier(*scene.GetRenderSettingsBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	auto &texVelocity = m_renderTarget->GetTexture();
	drawCmd->RecordImageBarrier(texVelocity.GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true) {
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderMotionBlur->RecordBeginDraw(bindState) == true) {
			ShaderPPMotionBlur::PushConstants pushConstants {};
			pushConstants.velocityScale = GetMotionBlurIntensity();
			pushConstants.blurQuality = umath::to_integral(GetMotionBlurQuality());
			shaderMotionBlur->RecordDraw(bindState, pushConstants, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *m_velocityTexDsg->GetDescriptorSet());
			shaderMotionBlur->RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texVelocity.GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}

void CRendererPpMotionBlurComponent::UpdateMotionBlurData() { m_motionDataUpdateRequired = true; }

void CRendererPpMotionBlurComponent::SetAutoUpdateMotionData(bool updateMotionPerFrame) { m_autoUpdateMotionData = updateMotionPerFrame; }

void CRendererPpMotionBlurComponent::SetMotionBlurIntensity(float intensity) { m_motionBlurIntensityFactor = intensity; }
float CRendererPpMotionBlurComponent::GetMotionBlurIntensity() const { return m_motionBlurIntensityFactor; }

void CRendererPpMotionBlurComponent::SetMotionBlurQuality(MotionBlurQuality quality) { m_motionBlurQuality = quality; }
MotionBlurQuality CRendererPpMotionBlurComponent::GetMotionBlurQuality() const { return m_motionBlurQuality; }

const std::shared_ptr<prosper::ISwapCommandBufferGroup> &CRendererPpMotionBlurComponent::GetSwapCommandBuffer() const { return m_swapCmd; }
const std::shared_ptr<prosper::RenderTarget> &CRendererPpMotionBlurComponent::GetRenderTarget() const { return m_renderTarget; }
