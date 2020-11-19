/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include <pragma/entities/baseentity_events.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(sky_camera,CSkyCamera);

void CSkyCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<CTransformComponent>();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"skybox_scale",false))
			m_skyboxScale = ustring::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_renderQueue = pragma::rendering::RenderQueue::Create();
	m_renderQueueTranslucent = pragma::rendering::RenderQueue::Create();

#if 0
	m_cbOnBuildRenderQueue = c_game->AddCallback("BuildRenderQueues",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([this](std::reference_wrapper<const util::DrawSceneInfo> refDrawSceneInfo) {
		if(refDrawSceneInfo.get().scene.expired())
			return;
		m_renderQueue->Clear();
		m_renderQueueTranslucent->Clear();
		m_renderQueue->Lock();
		m_renderQueueTranslucent->Lock();
		
		auto &drawSceneInfo = refDrawSceneInfo.get();
		c_game->GetRenderQueueBuilder().Append([this,&drawSceneInfo]() {
			auto &scene = *drawSceneInfo.scene.get();

			auto &pos = GetEntity().GetPosition();

			EntityIterator entItWorld {*c_game};
			entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
			std::vector<util::BSPTree::Node*> bspLeafNodes;
			bspLeafNodes.reserve(entItWorld.GetCount());
			for(auto *entWorld : entItWorld)
			{
				if(SceneRenderDesc::ShouldConsiderEntity(*static_cast<CBaseEntity*>(entWorld),scene,pos,drawSceneInfo.renderFlags) == false)
					continue;
				auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
				auto &bspTree = worldC->GetBSPTree();
				auto *node = bspTree ? bspTree->FindLeafNode(pos) : nullptr;
				if(node == nullptr)
					continue;
				bspLeafNodes.push_back(node);
			
				auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster,false /* translucent */);
				auto *renderQueueTranslucent = worldC->GetClusterRenderQueue(node->cluster,true /* translucent */);
				if(renderQueue)
					m_renderQueue->Merge(*renderQueue);
				if(renderQueueTranslucent)
					m_renderQueueTranslucent->Merge(*renderQueueTranslucent);
			}
		
			auto &hCam = scene.GetActiveCamera();
			auto *culler = scene.FindOcclusionCuller();
			if(culler && hCam.valid())
			{
				auto vp = hCam->GetProjectionMatrix() *hCam->GetViewMatrix();
				auto &dynOctree = culler->GetOcclusionOctree();
				SceneRenderDesc::CollectRenderMeshesFromOctree(
					drawSceneInfo,dynOctree,scene,*hCam,vp,drawSceneInfo.renderFlags,
					[this](RenderMode renderMode,bool translucent) -> pragma::rendering::RenderQueue* {
						return (renderMode != RenderMode::World) ? nullptr : (translucent ? m_renderQueueTranslucent.get() : m_renderQueue.get());
					},
					nullptr,&bspLeafNodes
				);
			}

			m_renderQueue->Sort();
			m_renderQueueTranslucent->Sort();

			m_renderQueue->Unlock();
			m_renderQueueTranslucent->Unlock();
		});
	}));
	m_cbPostRenderSkybox = c_game->AddCallback("PostRenderSkybox",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>,std::reference_wrapper<pragma::rendering::LightingStageRenderProcessor>>::Create(
		[this](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo,std::reference_wrapper<pragma::rendering::LightingStageRenderProcessor> rsys) {
		BindToShader(rsys.get());
			rsys.get().Render(*m_renderQueue);
			rsys.get().Render(*m_renderQueueTranslucent);
		UnbindFromShader(rsys.get());
	}));
#endif
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
	if(m_cbOnBuildRenderQueue.IsValid())
		m_cbOnBuildRenderQueue.Remove();
	if(m_cbPostRenderSkybox.IsValid())
		m_cbPostRenderSkybox.Remove();
}

float CSkyCameraComponent::GetSkyboxScale() const {return m_skyboxScale;}

void CSkyCameraComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
}
luabind::object CSkyCameraComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSkyCameraComponentHandleWrapper>(l);}

////////

void CSkyCamera::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyCameraComponent>();
}
