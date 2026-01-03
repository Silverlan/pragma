// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
#endif

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components;
import :engine;
import :entities.components.motion_blur_data;
import :game;
import :rendering.shaders;

using namespace pragma;

class DLLCLIENT VelocityStageRenderProcessor : public rendering::DepthStageRenderProcessor {
  public:
	VelocityStageRenderProcessor(ShaderVelocityBuffer &shaderVelocity, const rendering::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin, const MotionBlurTemporalData &motionBlurData, prosper::IDescriptorSet &dsMotionData, prosper::IDescriptorSet &dsBoneGeneric);
	virtual bool BindEntity(ecs::CBaseEntity &ent) override;
	virtual bool BindShader(prosper::Shader &shader, uint32_t pipelineIdx = 0u) override;
  private:
	ShaderVelocityBuffer &m_shaderVelocity;
	const MotionBlurTemporalData &m_motionBlurData;
	prosper::IDescriptorSet &m_dsMotionData;
	prosper::IDescriptorSet &m_dsBoneGeneric;
};
VelocityStageRenderProcessor::VelocityStageRenderProcessor(ShaderVelocityBuffer &shaderVelocity, const rendering::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin, const MotionBlurTemporalData &motionBlurData, prosper::IDescriptorSet &dsMotionData,
  prosper::IDescriptorSet &dsBoneGeneric)
    : DepthStageRenderProcessor {drawSceneInfo, drawOrigin}, m_shaderVelocity {shaderVelocity}, m_motionBlurData {motionBlurData}, m_dsMotionData {dsMotionData}, m_dsBoneGeneric {dsBoneGeneric}
{
}
bool VelocityStageRenderProcessor::BindShader(prosper::Shader &shader, uint32_t pipelineIdx)
{
	if(!DepthStageRenderProcessor::BindShader(shader, pipelineIdx))
		return false;
	auto &cmd = m_shaderProcessor.GetCommandBuffer();
	return cmd.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, m_shaderProcessor.GetCurrentPipelineLayout(), ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex, m_dsMotionData);
}
bool VelocityStageRenderProcessor::BindEntity(ecs::CBaseEntity &ent)
{
	auto res = DepthStageRenderProcessor::BindEntity(ent);
	if(res == false)
		return false;
	auto *dsBone = &m_dsBoneGeneric;
	auto it = m_motionBlurData.prevModelMatrices.find(&ent);
	if(it != m_motionBlurData.prevModelMatrices.end() && it->second.boneDsg != nullptr)
		dsBone = it->second.boneDsg->GetDescriptorSet();
	if(!m_shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, m_shaderProcessor.GetCurrentPipelineLayout(), ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex, *dsBone))
		return false;

	ShaderVelocityBuffer::MotionBlurPushConstants pushConstants {};
	if(it != m_motionBlurData.prevModelMatrices.end())
		pushConstants.prevPose = it->second.matrix;
	else
		pushConstants.prevPose = umat::identity();
	return m_shaderProcessor.GetCommandBuffer().RecordPushConstants(m_shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit, ShaderVelocityBuffer::MotionBlurPushConstants::Offset, sizeof(pushConstants), &pushConstants);
}

void CRendererPpMotionBlurComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
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
CRendererPpMotionBlurComponent::CRendererPpMotionBlurComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
	static auto g_shadersRegistered = false;
	if(!g_shadersRegistered) {
		g_shadersRegistered = true;
		get_cengine()->GetShaderManager().RegisterShader("velocity_buffer", [](prosper::IPrContext &context, const std::string &identifier) { return new ShaderVelocityBuffer(context, identifier); });
		get_cengine()->GetShaderManager().RegisterShader("pp_motion_blur", [](prosper::IPrContext &context, const std::string &identifier) { return new ShaderPPMotionBlur(context, identifier); });
	}
}
void CRendererPpMotionBlurComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererPpMotionBlurComponent::ReloadVelocityTexture()
{
	auto *shader = get_velocity_buffer_shader();
	if(!shader)
		return;
	auto rendererC = GetEntity().GetComponent<CRendererComponent>();
	auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rendererC.expired() || cRenderer.expired())
		return;
	get_cengine()->GetRenderContext().WaitIdle();

	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = rendererC->GetWidth();
	createInfo.height = rendererC->GetHeight();
	createInfo.format = prosper::Format::R32G32B32A32_SFloat;
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;

	auto &context = get_cengine()->GetRenderContext();
	auto img = context.CreateImage(createInfo);
	auto tex = context.CreateTexture({}, *img, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	auto rt = context.CreateRenderTarget({tex, cRenderer->GetPrepass().textureDepth}, shader->GetRenderPass());
	m_renderTarget = rt;

	m_velocityTexDsg->GetDescriptorSet()->SetBindingTexture(*tex, 0);
	m_valid = true;

#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
	auto *el = pragma::gui::WGUI::GetInstance().Create<WITexturedRect>();
	el->SetTexture(*tex);
	el->SetSize(512, 512);
	if(m_debugTex.IsValid())
		m_debugTex.Remove();
	m_debugTex = el->GetHandle();
	luabind::globals(pragma::get_cgame()->GetLuaState())["_el"] = el->GetHandle();
#endif
}

void CRendererPpMotionBlurComponent::Initialize()
{
	CRendererPpBaseComponent::Initialize();

	auto *velShader = get_velocity_buffer_shader();
	auto *shaderMotionBlur = static_cast<ShaderPPMotionBlur *>(get_cengine()->GetShader("pp_motion_blur").get());
	if(!velShader || !shaderMotionBlur)
		return;
	BindEventUnhandled(cRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(!m_valid)
			return;
		RecordVelocityPass(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(cRasterizationRendererComponent::EVENT_POST_PREPASS, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(!m_valid)
			return;
		ExecuteVelocityPass(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(cRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_valid = false;
		auto evDataReloaded = static_cast<CEOnRenderTargetReloaded &>(evData.get());
		if(!evDataReloaded.success)
			return;
		ReloadVelocityTexture();
	});

	auto &context = get_cengine()->GetRenderContext();
	m_swapCmd = context.CreateSwapCommandBufferGroup(context.GetWindow());

	m_velocityTexDsg = shaderMotionBlur->CreateDescriptorSetGroup(ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE_VELOCITY.setIndex);

	prosper::util::BufferCreateInfo motionBlurBufCreateInfo {};
	motionBlurBufCreateInfo.size = sizeof(MotionBlurCameraData);
	motionBlurBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	auto buf = context.CreateBuffer(motionBlurBufCreateInfo);
	m_motionBlurDataDsg = velShader->CreateDescriptorSetGroup(ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex);
	m_motionBlurDataDsg->GetDescriptorSet()->SetBindingUniformBuffer(*buf, 0);
	m_motionBlurDataBuffer = buf;

	m_genericBoneDsg = velShader->CreateDescriptorSetGroup(ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex);

	ReloadVelocityTexture();
}

void CRendererPpMotionBlurComponent::RecordVelocityPass(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto *velShader = get_velocity_buffer_shader();
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
		rendering::RenderPassDrawInfo renderPassDrawInfo {drawSceneInfo, cmd};
		VelocityStageRenderProcessor rsys {*velShader, renderPassDrawInfo, {} /* drawOrigin */, motionBlurTempData, *m_motionBlurDataDsg->GetDescriptorSet(), *m_genericBoneDsg->GetDescriptorSet()};

		rsys.BindShader(*velShader, math::to_integral(ShaderPrepass::Pipeline::Opaque));
		// Render static world geometry
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & rendering::RenderFlags::World) != rendering::RenderFlags::None) {
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->GetPassStats(rendering::RenderStats::RenderPass::Prepass)->SetTime(rendering::RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);
			for(auto i = decltype(worldRenderQueues.size()) {0u}; i < worldRenderQueues.size(); ++i)
				rsys.Render(*worldRenderQueues.at(i), rendering::RenderPass::Prepass, nullptr, i);
		}

		// Note: The non-translucent render queues also include transparent (alpha masked) objects.
		// We don't care about translucent objects here.
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & rendering::RenderFlags::World) != rendering::RenderFlags::None) {
			rsys.Render(*sceneRenderDesc.GetRenderQueue(rendering::SceneRenderPass::World, false /* translucent */), rendering::RenderPass::Prepass);

			auto &queueTranslucent = *sceneRenderDesc.GetRenderQueue(rendering::SceneRenderPass::World, true /* translucent */);
			queueTranslucent.WaitForCompletion();
			if(queueTranslucent.queue.empty() == false) {
				// rsys.BindShader(shaderPrepass,pragma::math::to_integral(pragma::ShaderPrepass::Pipeline::AlphaTest));
				rsys.Render(queueTranslucent, rendering::RenderPass::Prepass);
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

	auto *velShader = get_velocity_buffer_shader();
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRenderComponent>>();
	for(auto *ent : entIt) {
		auto &r = *static_cast<ecs::CBaseEntity *>(ent)->GetRenderComponent();
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
void CRendererPpMotionBlurComponent::ExecuteVelocityPass(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto *velShader = get_velocity_buffer_shader();
	if(!velShader)
		return;
	auto *renderer = drawSceneInfo.scene->GetRenderer<CRendererComponent>();
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
void CRendererPpMotionBlurComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) { RenderPostProcessing(drawSceneInfo); }
void CRendererPpMotionBlurComponent::RenderPostProcessing(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto *shaderMotionBlur = static_cast<ShaderPPMotionBlur *>(get_cengine()->GetShader("pp_motion_blur").get());

	if(!shaderMotionBlur)
		return;
	auto rasterC = scene.GetRenderer<CRendererComponent>()->GetEntity().GetComponent<CRasterizationRendererComponent>();
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
			pushConstants.blurQuality = math::to_integral(GetMotionBlurQuality());
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
