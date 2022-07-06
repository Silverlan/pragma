/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_render_motion_blur_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/rendering/shaders/info/c_shader_velocity_buffer.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_motion_blur.hpp"
#include "pragma/rendering/render_processor.hpp"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_buffer_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;
#pragma optimize("",off)

class DLLCLIENT VelocityStageRenderProcessor
	: public pragma::rendering::DepthStageRenderProcessor
{
public:
	VelocityStageRenderProcessor(
		pragma::ShaderVelocityBuffer &shaderVelocity,const util::RenderPassDrawInfo &drawSceneInfo,const Vector4 &drawOrigin,
		pragma::MotionBlurTemporalData &motionBlurData,prosper::IDescriptorSet &dsMotionData
	);
	virtual bool BindEntity(CBaseEntity &ent) override;
	virtual bool BindShader(prosper::Shader &shader,uint32_t pipelineIdx=0u) override;
private:
	pragma::ShaderVelocityBuffer &m_shaderVelocity;
	pragma::MotionBlurTemporalData &m_motionBlurData;
	prosper::IDescriptorSet &m_dsMotionData;
};
VelocityStageRenderProcessor::VelocityStageRenderProcessor(
	pragma::ShaderVelocityBuffer &shaderVelocity,const util::RenderPassDrawInfo &drawSceneInfo,const Vector4 &drawOrigin,
	pragma::MotionBlurTemporalData &motionBlurData,prosper::IDescriptorSet &dsMotionData
)
	: pragma::rendering::DepthStageRenderProcessor{drawSceneInfo,drawOrigin},m_shaderVelocity{shaderVelocity},
	m_motionBlurData{motionBlurData},m_dsMotionData{dsMotionData}
{}
bool VelocityStageRenderProcessor::BindShader(prosper::Shader &shader,uint32_t pipelineIdx)
{
	if(!DepthStageRenderProcessor::BindShader(shader,pipelineIdx))
		return false;
	auto &cmd = m_shaderProcessor.GetCommandBuffer();
	return cmd.RecordBindDescriptorSets(
		prosper::PipelineBindPoint::Graphics,m_shaderProcessor.GetCurrentPipelineLayout(),
		pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex,m_dsMotionData
	);
}
bool VelocityStageRenderProcessor::BindEntity(CBaseEntity &ent)
{
	auto res = pragma::rendering::DepthStageRenderProcessor::BindEntity(ent);
	if(res == false)
		return false;
	pragma::ShaderVelocityBuffer::MotionBlurPushConstants pushConstants {};

	auto it = m_motionBlurData.prevModelMatrices.find(&ent);
	if(it != m_motionBlurData.prevModelMatrices.end())
		pushConstants.prevPose = it->second;
	else
		pushConstants.prevPose = ent.GetRenderComponent()->GetTransformationMatrix();

	m_motionBlurData.curModelMatrices[&ent] = ent.GetRenderComponent()->GetTransformationMatrix();

	return m_shaderProcessor.GetCommandBuffer().RecordPushConstants(
		m_shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit,
		pragma::ShaderVelocityBuffer::MotionBlurPushConstants::Offset,
		sizeof(pushConstants),&pushConstants
	);
}

#include <wgui/types/wirect.h>
CRenderMotionBlurComponent::CRenderMotionBlurComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{
	static auto g_shadersRegistered = false;
	if(!g_shadersRegistered)
	{
		g_shadersRegistered = true;
		c_engine->GetShaderManager().RegisterShader(
			"velocity_buffer",[](prosper::IPrContext &context,const std::string &identifier) {return new pragma::ShaderVelocityBuffer(context,identifier);}
		);
		c_engine->GetShaderManager().RegisterShader(
			"pp_motion_blur",[](prosper::IPrContext &context,const std::string &identifier) {return new pragma::ShaderPPMotionBlur(context,identifier);}
		);
	}
}
void CRenderMotionBlurComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void CRenderMotionBlurComponent::ReloadVelocityTexture()
{
	if(m_velocityShader.expired())
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
	auto tex = context.CreateTexture({},*img,prosper::util::ImageViewCreateInfo{},prosper::util::SamplerCreateInfo{});
	auto rt = context.CreateRenderTarget(
		{tex,cRenderer->GetPrepass().textureDepth},
		static_cast<prosper::ShaderGraphics*>(m_velocityShader.get())->GetRenderPass()
	);
	m_renderTarget = rt;

	m_velocityTexDsg->GetDescriptorSet()->SetBindingTexture(*tex,0);
	m_valid = true;

	auto *el = WGUI::GetInstance().Create<WITexturedRect>();
	el->SetTexture(*tex);
	el->SetSize(512,512);
	if(m_debugTex.IsValid())
		m_debugTex.Remove();
	m_debugTex = el->GetHandle();
	luabind::globals(c_game->GetLuaState())["_el"] = el->GetHandle();
}

void CRenderMotionBlurComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto *velShader = static_cast<pragma::ShaderVelocityBuffer*>(c_engine->GetShader("velocity_buffer").get());
	auto *shaderMotionBlur = static_cast<pragma::ShaderPPMotionBlur*>(c_engine->GetShader("pp_motion_blur").get());
	if(!velShader || !shaderMotionBlur)
		return;
	m_velocityShader = velShader->GetHandle();

	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(!m_valid)
			return;
		RecordVelocityPass(static_cast<CEDrawSceneInfo&>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_POST_PREPASS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(!m_valid)
			return;
		ExecuteVelocityPass(static_cast<CEDrawSceneInfo&>(evData.get()).drawSceneInfo);
	});
	BindEventUnhandled(pragma::CRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_valid = false;
		auto evDataReloaded = static_cast<CEOnRenderTargetReloaded&>(evData.get());
		if(!evDataReloaded.success)
			return;
		ReloadVelocityTexture();
	});
	
	auto &context = c_engine->GetRenderContext();
	m_swapCmd = context.CreateSwapCommandBufferGroup(context.GetWindow());

	m_velocityTexDsg = shaderMotionBlur->CreateDescriptorSetGroup(pragma::ShaderPPMotionBlur::DESCRIPTOR_SET_TEXTURE_VELOCITY.setIndex);

	prosper::util::BufferCreateInfo motionBlurBufCreateInfo {};
	motionBlurBufCreateInfo.size = sizeof(MotionBlurData);
	motionBlurBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	auto buf = context.CreateBuffer(motionBlurBufCreateInfo);
	m_motionBlurDataDsg = velShader->CreateDescriptorSetGroup(pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_MOTION_BLUR.setIndex);
	m_motionBlurDataDsg->GetDescriptorSet()->SetBindingUniformBuffer(*buf,0);
	m_motionBlurDataBuffer = buf;

	ReloadVelocityTexture();
}

void CRenderMotionBlurComponent::RecordVelocityPass(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_velocityShader.expired())
		return;
	auto *velShader = static_cast<pragma::ShaderVelocityBuffer*>(m_velocityShader.get());
	auto &cmd = GetSwapCommandBuffer();
	auto &rt = GetRenderTarget();
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();

	auto camVel = m_motionBlurData.curCamPose.GetOrigin() -m_motionBlurData.prevCamPose.GetOrigin();
	camVel *= (1.f /60.f);

	auto rot = m_motionBlurData.curCamPose.GetRotation() *uquat::get_inverse(m_motionBlurData.prevCamPose.GetRotation());
	Vector3 axis;
	float angle;
	uquat::to_axis_angle(rot,axis,angle);
	auto angVel = axis *angle;

	MotionBlurData motionBlurData;
	motionBlurData.linearCameraVelocity = {camVel,0.f};
	motionBlurData.angularCameraVelocity = {angVel,0.f};
	drawSceneInfo.commandBuffer->RecordBufferBarrier(
		*m_motionBlurDataBuffer,
		prosper::PipelineStageFlags::VertexShaderBit,prosper::PipelineStageFlags::TransferBit,
		prosper::AccessFlags::UniformReadBit,prosper::AccessFlags::TransferWriteBit
	);
	drawSceneInfo.commandBuffer->RecordUpdateBuffer(*m_motionBlurDataBuffer,0u,motionBlurData);
	drawSceneInfo.commandBuffer->RecordBufferBarrier(
		*m_motionBlurDataBuffer,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::UniformReadBit
	);

	cmd->StartRecording(rt->GetRenderPass(),rt->GetFramebuffer());
	cmd->Record([this,&drawSceneInfo,velShader,&worldRenderQueues,&sceneRenderDesc](prosper::ISecondaryCommandBuffer &cmd) {
		util::RenderPassDrawInfo renderPassDrawInfo {drawSceneInfo,cmd};
		VelocityStageRenderProcessor rsys {
			*velShader,renderPassDrawInfo,{} /* drawOrigin */,
			m_motionBlurData,*m_motionBlurDataDsg->GetDescriptorSet()
		};
		
		auto tick = c_game->GetLastTick();
		auto updatePoses = (tick != m_motionBlurData.lastTick);
		if(updatePoses)
		{
			m_motionBlurData.prevCamPose = m_motionBlurData.curCamPose;
			m_motionBlurData.curCamPose = drawSceneInfo.scene->GetActiveCamera()->GetEntity().GetPose();
		}
		
		rsys.BindShader(*velShader,umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
		// Render static world geometry
		if((renderPassDrawInfo.drawSceneInfo.renderFlags &RenderFlags::World) != RenderFlags::None)
		{
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->SetTime(RenderPassStats::Timer::RenderThreadWait,std::chrono::steady_clock::now() -t);
			for(auto i=decltype(worldRenderQueues.size()){0u};i<worldRenderQueues.size();++i)
				rsys.Render(*worldRenderQueues.at(i),nullptr,i);
		}

		// Note: The non-translucent render queues also include transparent (alpha masked) objects.
		// We don't care about translucent objects here.
		if((renderPassDrawInfo.drawSceneInfo.renderFlags &RenderFlags::World) != RenderFlags::None)
		{
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World,false /* translucent */));

			auto &queueTranslucent = *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World,true /* translucent */);
			queueTranslucent.WaitForCompletion();
			if(queueTranslucent.queue.empty() == false)
			{
				// rsys.BindShader(shaderPrepass,umath::to_integral(pragma::ShaderPrepass::Pipeline::AlphaTest));
				rsys.Render(queueTranslucent);
			}
		}
		rsys.UnbindShader();

		if(updatePoses)
		{
			m_motionBlurData.lastTick = tick;
			std::swap(m_motionBlurData.prevModelMatrices,m_motionBlurData.curModelMatrices);
		}
	});
	cmd->EndRecording();
}
void CRenderMotionBlurComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_debugTex.IsValid())
		m_debugTex.Remove();
}
void CRenderMotionBlurComponent::ExecuteVelocityPass(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_velocityShader.expired())
		return;
	auto *renderer = drawSceneInfo.scene->GetRenderer();
	assert(renderer != nullptr);
	auto motionBlurC = renderer->GetEntity().GetComponent<CRenderMotionBlurComponent>();
	if(motionBlurC.expired())
		return;
	
	auto *velShader = static_cast<pragma::ShaderVelocityBuffer*>(m_velocityShader.get());
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &rt = motionBlurC->GetRenderTarget();
	drawCmd->RecordBeginRenderPass(
		*rt,std::vector<prosper::ClearValue>{prosper::ClearDepthStencilValue{}},
		prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers
	);

		auto &cmd = motionBlurC->GetSwapCommandBuffer();
		cmd->ExecuteCommands(*drawCmd);

	drawCmd->RecordEndRenderPass();
}
void CRenderMotionBlurComponent::PPTest(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto *shaderMotionBlur = static_cast<pragma::ShaderPPMotionBlur*>(c_engine->GetShader("pp_motion_blur").get());

	if(!shaderMotionBlur)
		return;
	auto rasterC = scene.GetRenderer()->GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.expired())
		return;
	auto &hdrInfo = rasterC->GetHDRInfo();
	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(
		*scene.GetCameraBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	drawCmd->RecordBufferBarrier(
		*scene.GetRenderSettingsBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	auto &texVelocity = m_renderTarget->GetTexture();
	drawCmd->RecordImageBarrier(
		texVelocity.GetImage(),
		prosper::ImageLayout::ColorAttachmentOptimal,
		prosper::ImageLayout::ShaderReadOnlyOptimal
	);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true)
	{
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderMotionBlur->RecordBeginDraw(bindState) == true)
		{
			shaderMotionBlur->RecordDraw(
				bindState,
				*hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(),
				*m_velocityTexDsg->GetDescriptorSet()
			);
			shaderMotionBlur->RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(
		texVelocity.GetImage(),
		prosper::ImageLayout::ShaderReadOnlyOptimal,
		prosper::ImageLayout::ColorAttachmentOptimal
	);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}

const std::shared_ptr<prosper::ISwapCommandBufferGroup> &CRenderMotionBlurComponent::GetSwapCommandBuffer() const {return m_swapCmd;}
const std::shared_ptr<prosper::RenderTarget> &CRenderMotionBlurComponent::GetRenderTarget() const {return m_renderTarget;}
#pragma optimize("",on)
