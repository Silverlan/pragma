// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/core.hpp"

#include "mathutil/umath.h"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.env_sky_camera;
import :client_state;
import :engine;
import :entities.components.game_occlusion_culler;
import :entities.components.rasterization_renderer;
import :entities.components.render;
import :entities.components.renderer;
import :entities.components.toggle;
import :entities.components.transform;
import :entities.components.world;
import :game;


using namespace pragma;

CSkyCameraComponent::SceneData::~SceneData()
{
	if(onRendererChanged.IsValid())
		onRendererChanged.Remove();
	if(onBuildRenderQueue.IsValid())
		onBuildRenderQueue.Remove();
	if(renderSkybox.IsValid())
		renderSkybox.Remove();
	if(updateRenderBuffers.IsValid())
		updateRenderBuffers.Remove();
	if(renderPrepass.IsValid())
		renderPrepass.Remove();
}
void CSkyCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<CTransformComponent>();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "skybox_scale", false))
			m_skyboxScale = ustring::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateScenes(); });
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateToggleState(); });
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateToggleState(); });
}

void CSkyCameraComponent::BuildSkyMeshRenderQueues(const pragma::CSceneComponent &scene, RenderFlags renderFlags, pragma::rendering::RenderMask renderMask, bool enableClipping, rendering::RenderQueue &outRenderQueue, rendering::RenderQueue &outTranslucentRenderQueue,
  pragma::CRasterizationRendererComponent *optRasterizationRenderer, bool waitForRenderQueues) const
{
	auto &pos = GetEntity().GetPosition();

	EntityIterator entItWorld {*pragma::get_cgame()};
	entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
	std::vector<util::BSPTree::Node *> bspLeafNodes;
	bspLeafNodes.reserve(entItWorld.GetCount());
	std::vector<util::BSPTree *> trees;
	trees.reserve(entItWorld.GetCount());
	for(auto *entWorld : entItWorld) {
		if(SceneRenderDesc::ShouldConsiderEntity(*static_cast<CBaseEntity *>(entWorld), scene, renderFlags, renderMask) == false)
			continue;
		auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
		auto &bspTree = worldC->GetBSPTree();
		auto *node = bspTree ? bspTree->FindLeafNode(pos) : nullptr;
		if(node == nullptr)
			continue;
		bspLeafNodes.push_back(node);
		trees.push_back(bspTree.get());

		auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster, false /* translucent */);
		auto *renderQueueTranslucent = worldC->GetClusterRenderQueue(node->cluster, true /* translucent */);
		if(renderQueue)
			outRenderQueue.Merge(*renderQueue);
		if(renderQueueTranslucent)
			outTranslucentRenderQueue.Merge(*renderQueueTranslucent);
	}

	auto &hCam = scene.GetActiveCamera();
	auto *culler = scene.FindOcclusionCuller<COcclusionCullerComponent>();
	if(culler && hCam.valid()) {
		auto vp = hCam->GetProjectionMatrix() * hCam->GetViewMatrix();
		auto &dynOctree = culler->GetOcclusionOctree();
		// TODO: Find out why Enable3dOriginBit specialization constant isn't working properly.
		// (Also see shaders/modules/vs_world.gls)
		// Also take into account that world render queues are built offline, but don't include the flag for the constant -> How to handle?
		SceneRenderDesc::CollectRenderMeshesFromOctree(
		  optRasterizationRenderer, renderFlags, enableClipping, dynOctree, scene, *hCam, vp, renderMask,
		  [&outRenderQueue, &outTranslucentRenderQueue](pragma::rendering::SceneRenderPass renderMode, bool translucent) -> pragma::rendering::RenderQueue * {
			  return (renderMode != pragma::rendering::SceneRenderPass::World) ? nullptr : (translucent ? &outTranslucentRenderQueue : &outRenderQueue);
		  },
		  nullptr, &trees, &bspLeafNodes, 0, nullptr, pragma::GameShaderSpecializationConstantFlag::None //Enable3dOriginBit
		);
		if(waitForRenderQueues)
			pragma::get_cgame()->GetRenderQueueWorkerManager().WaitForCompletion();
	}
}

void CSkyCameraComponent::BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo, SceneData &sceneData)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto *renderer = drawSceneInfo.scene->GetRenderer<pragma::CRendererComponent>();
	auto hRasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(hRasterizer.expired())
		return;
	sceneData.renderQueue->Clear();
	sceneData.renderQueueTranslucent->Clear();
	sceneData.renderQueue->Lock();
	sceneData.renderQueueTranslucent->Lock();

	auto renderMask = drawSceneInfo.GetRenderMask(*pragma::get_cgame());
	auto &rasterizer = *hRasterizer;
	pragma::get_cgame()->GetRenderQueueBuilder().Append(
	  [this, &rasterizer, &drawSceneInfo, &sceneData, renderMask]() {
		  auto &scene = *drawSceneInfo.scene.get();
		  // Build render queues
		  BuildSkyMeshRenderQueues(scene, drawSceneInfo.renderFlags, renderMask, drawSceneInfo.clipPlane.has_value(), *sceneData.renderQueue, *sceneData.renderQueueTranslucent, &rasterizer, false);
	  },
	  [&sceneData]() {
		  // Render queues are complete at this point, we can safely work with them again
		  sceneData.renderQueue->Sort();
		  sceneData.renderQueueTranslucent->Sort();

		  sceneData.renderQueue->Unlock();
		  sceneData.renderQueueTranslucent->Unlock();
	  });
}

void CSkyCameraComponent::UpdateToggleState()
{
	auto &ent = GetEntity();
	auto toggleC = ent.GetComponent<pragma::CToggleComponent>();
	auto isEnabled = toggleC.expired() || toggleC->IsTurnedOn();
	if(!isEnabled) {
		m_sceneData.clear();
		return;
	}
	UpdateScenes();
}

void CSkyCameraComponent::UpdateScenes()
{
	m_sceneData.clear();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto scenes = ent.GetScenes();
	for(auto *scene : scenes) {
		auto idx = scene->GetSceneIndex();
		auto &sceneData = m_sceneData.insert(std::make_pair(idx, std::shared_ptr<SceneData> {new SceneData {}})).first->second;
		auto *pSceneData = sceneData.get();
		sceneData->onBuildRenderQueue = scene->AddEventCallback(pragma::CSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES, [this, pSceneData, &ent, scene](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &drawSceneInfo = static_cast<pragma::CEDrawSceneInfo &>(evData.get()).drawSceneInfo;
			if(!umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::Skybox) || !ent.IsInScene(*scene))
				return util::EventReply::Unhandled;
			BuildRenderQueues(drawSceneInfo, *pSceneData);
			return util::EventReply::Unhandled;
		});
		sceneData->onRendererChanged = scene->AddEventCallback(pragma::CSceneComponent::EVENT_ON_RENDERER_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			UpdateToggleState();
			return util::EventReply::Unhandled;
		});

		sceneData->renderQueue = pragma::rendering::RenderQueue::Create("sky_camera");
		sceneData->renderQueueTranslucent = pragma::rendering::RenderQueue::Create("sky_camera_translucent");

		auto *renderer = scene->GetRenderer<pragma::CRendererComponent>();
		if(!renderer)
			continue;
		auto rasterizationC = renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>();
		if(rasterizationC.expired())
			continue;
		sceneData->renderSkybox = rasterizationC->AddEventCallback(pragma::CRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX, [this, pSceneData, &ent, scene](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &stageData = static_cast<pragma::CELightingStageData &>(evData.get());
			auto &rsys = stageData.renderProcessor;
			auto &drawSceneInfo = rsys.GetRenderPassDrawInfo().drawSceneInfo;
			if(!umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::Skybox) || !ent.IsInScene(*scene))
				return util::EventReply::Unhandled;
			BindToShader(rsys);
			rsys.Render(*pSceneData->renderQueue);
			rsys.Render(*pSceneData->renderQueueTranslucent);
			UnbindFromShader(rsys);
			return util::EventReply::Unhandled;
		});
		sceneData->updateRenderBuffers = rasterizationC->AddEventCallback(pragma::CRasterizationRendererComponent::EVENT_UPDATE_RENDER_BUFFERS, [this, pSceneData, &ent, scene](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &updateRenderBuffersInfo = static_cast<pragma::CEUpdateRenderBuffers &>(evData.get());
			auto &drawSceneInfo = updateRenderBuffersInfo.drawSceneInfo;
			auto &drawCmd = drawSceneInfo.commandBuffer;
			if(!umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::Skybox) || !ent.IsInScene(*scene))
				return util::EventReply::Unhandled;
			// Need to update the render buffers for our render queues
			CSceneComponent::UpdateRenderBuffers(drawCmd, *pSceneData->renderQueue, drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
			CSceneComponent::UpdateRenderBuffers(drawCmd, *pSceneData->renderQueueTranslucent, drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
			return util::EventReply::Unhandled;
		});
		sceneData->renderPrepass = rasterizationC->AddEventCallback(pragma::CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS, [this, pSceneData, &ent, scene](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &stageData = static_cast<pragma::CEPrepassStageData &>(evData.get());
			auto &rsys = stageData.renderProcessor;
			auto &rpDrawInfo = rsys.GetRenderPassDrawInfo();
			auto &drawSceneInfo = rpDrawInfo.drawSceneInfo;
			auto &drawCmd = drawSceneInfo.commandBuffer;
			if(!umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::Skybox) || !ent.IsInScene(*scene))
				return util::EventReply::Unhandled;
			rsys.UnbindShader();
			BindToShader(rsys);
			rsys.BindShader(stageData.shader, umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
			rsys.Render(*pSceneData->renderQueue, pragma::rendering::RenderPass::Prepass);
			rsys.Render(*pSceneData->renderQueueTranslucent, pragma::rendering::RenderPass::Prepass);
			UnbindFromShader(rsys);
			rsys.UnbindShader();
			return util::EventReply::Unhandled;
		});
	}
}

void CSkyCameraComponent::BindToShader(pragma::rendering::BaseRenderProcessor &processor) const
{
	processor.Set3DSky(true);
	auto &ent = GetEntity();
	auto &pos = ent.GetPosition();
	Vector4 drawOrigin {pos.x, pos.y, pos.z, GetSkyboxScale()};
	processor.SetDrawOrigin(drawOrigin);
}
void CSkyCameraComponent::UnbindFromShader(pragma::rendering::BaseRenderProcessor &processor) const
{
	processor.Set3DSky(false);
	processor.SetDrawOrigin({});
}

void CSkyCameraComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

float CSkyCameraComponent::GetSkyboxScale() const { return m_skyboxScale; }

void CSkyCameraComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	UpdateToggleState();
}
void CSkyCameraComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CSkyCamera::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyCameraComponent>();
}
