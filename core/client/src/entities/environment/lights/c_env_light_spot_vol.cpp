/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/networking/c_nwm_util.h"
#include <sharedutils/alpha_mode.hpp>
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <cmaterial.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_spot_vol,CEnvLightSpotVol);

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent<CModelComponent>();
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid())
	{
		// TODO
		/*FlagCallbackForRemoval(pRenderComponent->BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto *scene = c_game->GetScene();
			auto *renderer = scene ? scene->GetRenderer() : nullptr;
			if(renderer != nullptr && renderer->IsRasterizationRenderer())
				static_cast<pragma::CRasterizationRendererComponent*>(renderer)->SetFrameDepthBufferSamplingRequired();
		}),CallbackType::Entity);*/
	}
}

Bool CLightSpotVolComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetSpotlightTarget)
	{
		auto *ent = nwm::read_entity(packet);
		if(ent != nullptr)
			SetSpotlightTarget(*ent);
		else
			m_hSpotlightTarget = EntityHandle{};
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}

void CLightSpotVolComponent::ReceiveData(NetPacket &packet)
{
	m_coneAngle = packet->Read<float>();
	m_coneStartOffset = packet->Read<float>();
	auto hEnt = GetHandle();
	nwm::read_unique_entity(packet,[this,hEnt](BaseEntity *ent) {
		if(hEnt.expired())
			return;
		if(ent != nullptr)
			SetSpotlightTarget(*ent);
		else
			m_hSpotlightTarget = EntityHandle{};
	});
}

util::EventReply CLightSpotVolComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEnvLightSpotVolComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
	{
		auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
	}
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
	{
		auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::None);
	}
	return util::EventReply::Unhandled;
}

void CLightSpotVolComponent::InitializeVolumetricLight()
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(!mdlComponent)
		return;
	auto mdl = c_game->CreateModel();
	auto group = mdl->AddMeshGroup("reference");

	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto maxDist = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 100.f;
	auto endRadius = maxDist *umath::tan(umath::deg_to_rad(m_coneAngle));

	auto *mat = static_cast<CMaterial*>(client->CreateMaterial("lightcone","light_cone"));
	auto &data = mat->GetDataBlock();
	data->AddValue("int","alpha_mode",std::to_string(umath::to_integral(AlphaMode::Blend)));
	data->AddValue("float","cone_height",std::to_string(maxDist));
	mat->SetTexture("albedo_map","error");

	const uint32_t coneDetail = 64;
	const uint32_t segmentCount = 20;
	const Vector3 dir {0.f,0.f,1.f};
	auto mesh = std::make_shared<CModelMesh>();
	for(auto i=decltype(segmentCount){0};i<segmentCount;++i)
	{
		auto startSc = i /static_cast<float>(segmentCount);
		auto endSc = (i +1) /static_cast<float>(segmentCount);
		
		auto segEndRadius = endRadius *endSc;
		if(segEndRadius < m_coneStartOffset)
			continue; // Skip this segment
		auto segStartRadius = endRadius *startSc;
		if(segStartRadius < m_coneStartOffset)
		{
			// Clamp this segment
			segStartRadius = m_coneStartOffset;
		}

		auto startPos = dir *maxDist *static_cast<float>(segStartRadius /endRadius);
		auto endPos = dir *maxDist *static_cast<float>(segEndRadius /endRadius);
		auto subMesh = std::make_shared<CModelSubMesh>();
		std::vector<Vector3> verts;
		std::vector<Vector3> normals;
		umath::geometry::generate_truncated_cone_mesh(startPos,static_cast<float>(segStartRadius),dir,uvec::distance(startPos,endPos),static_cast<float>(segEndRadius),verts,&subMesh->GetTriangles(),&normals,coneDetail,false);

		auto &meshVerts = subMesh->GetVertices();
		meshVerts.reserve(verts.size());
		for(auto idx=decltype(verts.size()){0};idx<verts.size();++idx)
			meshVerts.push_back(umath::Vertex{verts[idx],normals[idx]});

		subMesh->SetSkinTextureIndex(0);
		mesh->AddSubMesh(subMesh);
	}
	group->AddMesh(mesh);
	mdl->AddMaterial(0,mat);
	mdl->Update(ModelUpdateFlags::All);

	mdlComponent->SetModel(mdl);
}

void CLightSpotVolComponent::OnEntitySpawn()
{
	BaseEnvLightSpotVolComponent::OnEntitySpawn();
	InitializeVolumetricLight();
}

void CLightSpotVolComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

///////

void CEnvLightSpotVol::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightSpotVolComponent>();
}
