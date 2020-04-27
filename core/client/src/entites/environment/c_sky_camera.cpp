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

	m_renderMeshCollectionHandler.GetRenderMeshData().insert(std::make_pair(RenderMode::World,std::make_shared<rendering::CulledMeshData>()));
	m_cbOnPreRender = c_game->AddCallback("OnPreRender",FunctionCallback<void,rendering::RasterizationRenderer*>::Create([this](rendering::RasterizationRenderer *renderer) {
		auto &posCam = GetEntity().GetPosition();
		m_renderMeshCollectionHandler.PerformOcclusionCulling(*renderer,posCam,false);

		//umath::set_flag(renderFlags,FRender::View | FRender::Skybox,false);
		auto renderFlags = FRender::World | FRender::Static | FRender::Dynamic;
		// TODO: No glow or translucent meshes are supported in 3D skybox for now
		auto resultFlags = m_renderMeshCollectionHandler.GenerateOptimizedRenderObjectStructures(*renderer,posCam,renderFlags,RenderMode::World,false,false);
	}));
}

void CSkyCameraComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_cbOnPreRender.IsValid())
		m_cbOnPreRender.Remove();
}

float CSkyCameraComponent::GetSkyboxScale() const {return m_skyboxScale;}

rendering::RenderMeshCollectionHandler &CSkyCameraComponent::GetRenderMeshCollectionHandler() {return m_renderMeshCollectionHandler;}
const rendering::RenderMeshCollectionHandler &CSkyCameraComponent::GetRenderMeshCollectionHandler() const {return const_cast<CSkyCameraComponent*>(this)->GetRenderMeshCollectionHandler();}

const rendering::CulledMeshData &CSkyCameraComponent::UpdateRenderMeshes(rendering::RasterizationRenderer &renderer,FRender renderFlags)
{
	auto &renderMeshData = m_renderMeshCollectionHandler.GetRenderMeshData();
	auto it = renderMeshData.find(RenderMode::World);
	if(it == renderMeshData.end())
		throw std::logic_error{"Invalid world mesh data!"};
	return *it->second;
}

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
