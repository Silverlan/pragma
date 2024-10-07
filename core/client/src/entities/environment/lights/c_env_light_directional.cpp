/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_environment, CEnvLightDirectional);

void CLightDirectionalComponent::Initialize()
{
	BaseEnvLightDirectionalComponent::Initialize();

	BindEvent(CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix &>(evData.get());
		trData.transformation = &MVPBias<1>::GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS, [](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEShouldUpdateRenderPass &>(evData.get()).shouldUpdate = true; // CSM Update requirements are determined through ShadowMapCasc::ShouldUpdateLayer
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntity &>(evData.get());
		auto &ent = shouldPassData.entity;
		auto pTrComponent = ent.GetTransformComponent();
		auto pRenderComponent = ent.GetRenderComponent();
		if(pTrComponent == nullptr || !pRenderComponent) {
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		auto &aabb = pRenderComponent->GetUpdatedAbsoluteRenderBounds();

		auto hShadow = GetEntity().GetComponent<CShadowCSMComponent>();
		if(hShadow.expired())
			return util::EventReply::Unhandled;
		auto numLayers = hShadow->GetLayerCount();
		for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
			auto &frustum = *hShadow->GetFrustumSplit(i);
			if(umath::intersection::aabb_aabb(aabb.min, aabb.max, frustum.aabb.min + frustum.obbCenter, frustum.aabb.max + frustum.obbCenter) != umath::intersection::Intersect::Outside)
				shouldPassData.renderFlags |= 1 << i;
		}
		if(shouldPassData.renderFlags == 0) {
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntityMesh &>(evData.get());
		auto hShadow = GetEntity().GetComponent<CShadowCSMComponent>();
		if(hShadow.valid()) {
			auto &entTgt = shouldPassData.entity;
			auto pTrComponent = entTgt.GetTransformComponent();
			if(pTrComponent == nullptr) {
				shouldPassData.shouldPass = false;
				return util::EventReply::Handled;
			}
			auto &pos = pTrComponent->GetPosition();
			Vector3 min;
			Vector3 max;
			shouldPassData.mesh.GetBounds(min, max);
			min += pos;
			max += pos;

			auto numLayers = hShadow->GetLayerCount();
			for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
				auto &frustum = *hShadow->GetFrustumSplit(i);
				if(umath::intersection::aabb_aabb(min, max, frustum.aabb.min + frustum.obbCenter, frustum.aabb.max + frustum.obbCenter) != umath::intersection::Intersect::Outside)
					shouldPassData.renderFlags |= 1 << i;
			}
		}
		if(shouldPassData.renderFlags == 0) {
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(CLightComponent::EVENT_HANDLE_SHADOW_MAP, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto csmC = GetEntity().AddComponent<CShadowCSMComponent>();
		if(csmC.valid())
			csmC->ReloadDepthTextures();
		m_shadowMap = csmC;

		auto lightC = GetEntity().GetComponent<CLightComponent>();
		if(lightC.valid())
			lightC->UpdateShadowTypes();
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr) {
		Vector3 pos = pTrComponent->GetPosition();
		auto dir = pTrComponent->GetForward();
		SetViewMatrix(glm::lookAtRH(pos, pos + dir, uvec::get_perpendicular(dir)));
		auto &trC = static_cast<CTransformComponent &>(*pTrComponent);
		FlagCallbackForRemoval(trC.AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::RotationChanged) == false)
				                         return util::EventReply::Unhandled;
			                         auto &ent = GetEntity();
			                         auto pLightComponent = ent.GetComponent<CLightComponent>();
			                         auto type = pragma::LightType::Undefined;
			                         auto *pLight = pLightComponent.valid() ? pLightComponent->GetLight(type) : nullptr;
			                         if(pLight == nullptr || type != pragma::LightType::Directional)
				                         return util::EventReply::Unhandled;
			                         auto pTrComponent = pLight->GetEntity().GetTransformComponent();
			                         if(pTrComponent != nullptr)
				                         pTrComponent->SetRotation(trC.GetRotation());
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Entity);
	}

	auto pLightComponent = ent.GetComponent<CLightComponent>();
	if(pLightComponent.valid()) {
		pLightComponent->UpdateTransformationMatrix(GetBiasTransformationMatrix(), GetViewMatrix(), GetProjectionMatrix());
		// pLightComponent->SetStateFlag(CLightComponent::StateFlags::UseDualTextureSet,false);
	}

	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(GetEntity().IsSpawned() == false)
			return;
		c_game->UpdateEnvironmentLightSource();
	});
	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(GetEntity().IsSpawned() == false)
			return;
		c_game->UpdateEnvironmentLightSource();
	});
}
void CLightDirectionalComponent::SetShadowDirty()
{
	/*auto shadowMapC = GetEntity().GetComponent<CShadowCSMComponent>();
	if(shadowMapC.valid())
		shadowMapC->SetDirty(true);*/ // TODO
}
void CLightDirectionalComponent::OnEntitySpawn()
{
	BaseEnvLightDirectionalComponent::OnEntitySpawn();
	auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
	if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn())
		c_game->UpdateEnvironmentLightSource();
}
CShadowCSMComponent *CLightDirectionalComponent::GetShadowMap() { return m_shadowMap.get(); }
void CLightDirectionalComponent::ReceiveData(NetPacket &packet)
{
	auto r = packet->Read<short>();
	auto g = packet->Read<short>();
	auto b = packet->Read<short>();
	auto a = packet->Read<short>();
	*m_ambientColor = Color {r, g, b, a};
	m_maxExposure = packet->Read<Float>();
}
Bool CLightDirectionalComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetAmbientColor) {
		Color color;
		color.r = packet->Read<short>();
		color.g = packet->Read<short>();
		color.b = packet->Read<short>();
		color.a = packet->Read<short>();
		SetAmbientColor(color);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
util::EventReply CLightDirectionalComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvLightDirectionalComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
		UpdateAmbientColor();
	return util::EventReply::Unhandled;
}

void CLightDirectionalComponent::SetAmbientColor(const Color &color)
{
	BaseEnvLightDirectionalComponent::SetAmbientColor(color);
	UpdateAmbientColor();
}

void CLightDirectionalComponent::UpdateAmbientColor()
{
	auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
	if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false)
		return;
	// c_game->GetWorldEnvironment().SetAmbientColor((*m_ambientColor)->ToVector4());
	c_game->SetMaxHDRExposure(m_maxExposure);
}
void CLightDirectionalComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

void CEnvLightDirectional::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
	AddComponent<CLightDirectionalComponent>();
}

//////////////

void CLightDirectionalComponent::RenderStaticWorldGeometry()
{
	// prosper TODO
#if 0
	m_cmdShadowBuffers.clear();
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
		return;
	auto hShaderShadow = c_engine->GetShader("shadowcsmstatic");
	if(!hShaderShadow.IsValid())
		return;
	auto hMdl = entWorld->GetModel();
	if(hMdl == nullptr)
		return;
	auto &model = *hMdl.get();
	auto *shadow = GetShadowMap();
	if(shadow == nullptr)
		return;
	auto *tex = shadow->GetDepthTexture(); // TODO
	if(tex == nullptr)
		return;
	auto &shaderShadow = static_cast<Shader::ShadowCSMStatic&>(*hShaderShadow.get());
	//auto &context = c_engine->GetRenderContext();

	//auto &renderPass = shadow->GetRenderPass(CLightBase::RenderPass::Dynamic);
	//auto &img = tex->GetImage();

	CLightBase::EntityInfo info(this,entWorld);
	info.meshes.reserve(model.GetSubMeshCount());
	//auto &materials = model.GetMaterials();
	for(auto &group : model.GetMeshGroups())
	{
		for(auto &mesh : group->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				if(CLightBase::ShouldPass(model,*static_cast<CModelSubMesh*>(subMesh.get())) == false)
					continue;
				info.meshes.push_back(static_cast<CModelSubMesh*>(subMesh.get()));
			}
		}
	}
	shaderShadow.Render(*this,m_cmdShadowBuffers,info);
#endif
}
void CLightDirectionalComponent::ReloadShadowCommandBuffers() { m_bShadowBufferUpdateScheduled = true; }

bool CLightDirectionalComponent::ShouldPass(uint32_t layer, const Vector3 &min, const Vector3 &max)
{
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	auto hShadow = GetEntity().GetComponent<CShadowCSMComponent>();
	if(hShadow.expired())
		return false;
	auto numLayers = hShadow->GetLayerCount();
	auto &frustum = *hShadow->GetFrustumSplit(layer);
	return (umath::intersection::aabb_aabb(min, max, frustum.aabb.min + frustum.obbCenter, frustum.aabb.max + frustum.obbCenter) != umath::intersection::Intersect::Outside) ? true : false;
}

void CLightDirectionalComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightDirectionalComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CTransformComponent)) {
		auto &trC = static_cast<CTransformComponent &>(component);
		FlagCallbackForRemoval(trC.AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::RotationChanged) == false)
				                         return util::EventReply::Unhandled;
			                         auto dir = uquat::forward(trC.GetRotation());
			                         SetViewMatrix(glm::lookAtRH(trC.GetPosition(), trC.GetPosition() + dir, uvec::get_perpendicular(dir)));

			                         //auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
			                         //if(pLightComponent.valid())
			                         //	pLightComponent->SetStaticResolved(false);
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CLightComponent))
		static_cast<CLightComponent &>(component).SetLight(*this);
}

void CLightDirectionalComponent::UpdateFrustum(uint32_t frustumId)
{
	auto &ent = GetEntity();
	auto pLightComponent = ent.GetComponent<CLightComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	auto pToggleComponent = ent.GetComponent<CToggleComponent>();
	if((pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false) || pLightComponent.expired() || pTrComponent == nullptr)
		return;
	auto hShadow = GetEntity().GetComponent<CShadowCSMComponent>();
	if(hShadow.expired())
		return;
	auto *cam = c_game->GetPrimaryCamera();
	hShadow->UpdateFrustum(frustumId, *cam, GetViewMatrix(), pTrComponent->GetForward());
}

void CLightDirectionalComponent::UpdateFrustum()
{
	auto &ent = GetEntity();
	auto pLightComponent = ent.GetComponent<CLightComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	auto pToggleComponent = ent.GetComponent<CToggleComponent>();
	if((pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false) || pLightComponent.expired() || pTrComponent == nullptr)
		return;
	auto hShadow = GetEntity().GetComponent<CShadowCSMComponent>();
	if(hShadow.expired())
		return;
	auto *cam = c_game->GetPrimaryCamera();
	hShadow->UpdateFrustum(*cam, GetViewMatrix(), pTrComponent->GetForward());
}
