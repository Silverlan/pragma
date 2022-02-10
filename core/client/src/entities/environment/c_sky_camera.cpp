/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/baseentity_events.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(sky_camera,CSkyCamera);

CSkyCameraComponent::SceneCallbacks::~SceneCallbacks()
{
	if(onRendererChanged.IsValid())
		onRendererChanged.Remove();
	if(onBuildRenderQueue.IsValid())
		onBuildRenderQueue.Remove();
	if(renderSkybox.IsValid())
		renderSkybox.Remove();
	if(renderPrepass.IsValid())
		renderPrepass.Remove();
}
void CSkyCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<CTransformComponent>();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"skybox_scale",false))
			m_skyboxScale = ustring::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateScenes();
	});
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_ON,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {UpdateToggleState();});
	BindEventUnhandled(CToggleComponent::EVENT_ON_TURN_OFF,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {UpdateToggleState();});

	m_renderQueue = pragma::rendering::RenderQueue::Create("sky_camera");
	m_renderQueueTranslucent = pragma::rendering::RenderQueue::Create("sky_camera_translucent");
}

void CSkyCameraComponent::Render3dSkybox(pragma::rendering::LightingStageRenderProcessor &rsys)
{
	BindToShader(rsys);
		rsys.Render(*m_renderQueue);
		rsys.Render(*m_renderQueueTranslucent);
	UnbindFromShader(rsys);
}

void CSkyCameraComponent::BuildSkyMeshRenderQueues(
	const pragma::CSceneComponent &scene,RenderFlags renderFlags,pragma::rendering::RenderMask renderMask,bool enableClipping,
	rendering::RenderQueue &outRenderQueue,rendering::RenderQueue &outTranslucentRenderQueue,pragma::CRasterizationRendererComponent *optRasterizationRenderer
) const
{
	auto &pos = GetEntity().GetPosition();

	EntityIterator entItWorld {*c_game};
	entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
	std::vector<util::BSPTree::Node*> bspLeafNodes;
	bspLeafNodes.reserve(entItWorld.GetCount());
	std::vector<util::BSPTree*> trees;
	trees.reserve(entItWorld.GetCount());
	for(auto *entWorld : entItWorld)
	{
		if(SceneRenderDesc::ShouldConsiderEntity(*static_cast<CBaseEntity*>(entWorld),scene,renderFlags,renderMask) == false)
			continue;
		auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
		auto &bspTree = worldC->GetBSPTree();
		auto *node = bspTree ? bspTree->FindLeafNode(pos) : nullptr;
		if(node == nullptr)
			continue;
		bspLeafNodes.push_back(node);
		trees.push_back(bspTree.get());
			
		auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster,false /* translucent */);
		auto *renderQueueTranslucent = worldC->GetClusterRenderQueue(node->cluster,true /* translucent */);
		if(renderQueue)
			outRenderQueue.Merge(*renderQueue);
		if(renderQueueTranslucent)
			outTranslucentRenderQueue.Merge(*renderQueueTranslucent);
	}
	
	auto &hCam = scene.GetActiveCamera();
	auto *culler = scene.FindOcclusionCuller();
	if(culler && hCam.valid())
	{
		auto vp = hCam->GetProjectionMatrix() *hCam->GetViewMatrix();
		auto &dynOctree = culler->GetOcclusionOctree();
		// TODO: Find out why Enable3dOriginBit specialization constant isn't working properly.
		// (Also see shaders/modules/vs_world.gls)
		// Also take into account that world render queues are built offline, but don't include the flag for the constant -> How to handle?
		SceneRenderDesc::CollectRenderMeshesFromOctree(
			optRasterizationRenderer,renderFlags,enableClipping,dynOctree,scene,*hCam,vp,renderMask,
			[&outRenderQueue,&outTranslucentRenderQueue](pragma::rendering::SceneRenderPass renderMode,bool translucent) -> pragma::rendering::RenderQueue* {
				return (renderMode != pragma::rendering::SceneRenderPass::World) ? nullptr : (translucent ? &outTranslucentRenderQueue : &outRenderQueue);
			},
			nullptr,&trees,&bspLeafNodes,0,nullptr,pragma::GameShaderSpecializationConstantFlag::None//Enable3dOriginBit
		);
	}
}

void CSkyCameraComponent::BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto *renderer = drawSceneInfo.scene->GetRenderer();
	auto hRasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent>{};
	if(hRasterizer.expired())
		return;
	m_renderQueue->Clear();
	m_renderQueueTranslucent->Clear();
	m_renderQueue->Lock();
	m_renderQueueTranslucent->Lock();

	auto renderMask = drawSceneInfo.GetRenderMask(*c_game);
	auto &rasterizer = *hRasterizer;
	c_game->GetRenderQueueBuilder().Append([this,&rasterizer,&drawSceneInfo,renderMask]() {
		auto &scene = *drawSceneInfo.scene.get();
		BuildSkyMeshRenderQueues(scene,drawSceneInfo.renderFlags,renderMask,drawSceneInfo.clipPlane.has_value(),*m_renderQueue,*m_renderQueueTranslucent,&rasterizer);

		m_renderQueue->Sort();
		m_renderQueueTranslucent->Sort();

		m_renderQueue->Unlock();
		m_renderQueueTranslucent->Unlock();
	});
}

void CSkyCameraComponent::UpdateToggleState()
{
	auto &ent = GetEntity();
	auto toggleC = ent.GetComponent<pragma::CToggleComponent>();
	auto isEnabled = toggleC.expired() || toggleC->IsTurnedOn();
	if(!isEnabled)
	{
		m_sceneCallbacks.clear();
		return;
	}
	UpdateScenes();
}

void CSkyCameraComponent::UpdateScenes()
{
	m_sceneCallbacks.clear();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto scenes = ent.GetScenes();
	for(auto *scene : scenes)
	{
		auto idx = scene->GetSceneIndex();
		auto &callbacks = m_sceneCallbacks.insert(std::make_pair(idx,SceneCallbacks{})).first->second;
		callbacks.onBuildRenderQueue = scene->AddEventCallback(pragma::CSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			BuildRenderQueues(static_cast<pragma::CEDrawSceneInfo&>(evData.get()).drawSceneInfo);
			return util::EventReply::Unhandled;
		});
		callbacks.onRendererChanged = scene->AddEventCallback(pragma::CSceneComponent::EVENT_ON_RENDERER_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			UpdateToggleState();
			return util::EventReply::Unhandled;
		});
		auto *renderer = scene->GetRenderer();
		if(!renderer)
			continue;
		auto rasterizationC = renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>();
		if(rasterizationC.expired())
			continue;
		callbacks.renderSkybox = rasterizationC->AddEventCallback(pragma::CRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &stageData = static_cast<pragma::CELightingStageData&>(evData.get());
			auto &rsys = stageData.renderProcessor;
			BindToShader(rsys);
				rsys.Render(*m_renderQueue);
				rsys.Render(*m_renderQueueTranslucent);
			UnbindFromShader(rsys);
			return util::EventReply::Unhandled;
		});
		callbacks.renderPrepass = rasterizationC->AddEventCallback(pragma::CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &stageData = static_cast<pragma::CEPrepassStageData&>(evData.get());
			auto &rsys = stageData.renderProcessor;
			auto &rpDrawInfo = rsys.GetRenderPassDrawInfo();
			auto &drawSceneInfo = rpDrawInfo.drawSceneInfo;
			auto &drawCmd = drawSceneInfo.commandBuffer;
			// Need to update the render buffers for our render queues
			CSceneComponent::UpdateRenderBuffers(drawCmd,*m_renderQueue,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
			CSceneComponent::UpdateRenderBuffers(drawCmd,*m_renderQueueTranslucent,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);

			rsys.UnbindShader();
			BindToShader(rsys);
			rsys.BindShader(stageData.shader,umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
				rsys.Render(*m_renderQueue);
				rsys.Render(*m_renderQueueTranslucent);
			UnbindFromShader(rsys);
			rsys.UnbindShader();
			return util::EventReply::Unhandled;
		});
	}
}

void CSkyCameraComponent::BindToShader(pragma::rendering::BaseRenderProcessor &processor)
{
	processor.Set3DSky(true);
	auto &ent = GetEntity();
	auto &pos = ent.GetPosition();
	Vector4 drawOrigin {pos.x,pos.y,pos.z,GetSkyboxScale()};
	processor.SetDrawOrigin(drawOrigin);
}
void CSkyCameraComponent::UnbindFromShader(pragma::rendering::BaseRenderProcessor &processor)
{
	processor.Set3DSky(false);
	processor.SetDrawOrigin({});
}

void CSkyCameraComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
}

float CSkyCameraComponent::GetSkyboxScale() const {return m_skyboxScale;}

void CSkyCameraComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	UpdateToggleState();
}
void CSkyCameraComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

////////

void CSkyCamera::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyCameraComponent>();
}
