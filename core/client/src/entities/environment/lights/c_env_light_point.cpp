/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/c_cubemapside.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/math/intersection.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_point, CEnvLightPoint);

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static const std::array<Vector3, 6> directions = {Vector3(1.f, 0.f, 0.f), Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector3(0.f, 0.f, -1.f)};

void CLightPointComponent::Initialize()
{
	BaseEnvLightPointComponent::Initialize();
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntity &>(evData.get());
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.expired() || pLightComponent->IsInRange(shouldPassData.entity) == false) {
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		const auto ang = 0.7853982f; // 45 Degree

		auto &ent = shouldPassData.entity;
		auto pRenderComponent = ent.GetRenderComponent();
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentThis = GetEntity().GetTransformComponent();
		if(!pRenderComponent || pTrComponent == nullptr || !pTrComponentThis) {
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		//auto &start = pTrComponentThis->GetPosition();
		auto &sphere = pRenderComponent->GetUpdatedAbsoluteRenderSphere();
		for(auto i = decltype(directions.size()) {0}; i < directions.size(); ++i) {
			//auto &dir = directions[i];
			auto &plane = m_frustumPlanes.at(i);
			if(umath::intersection::sphere_in_plane_mesh(sphere.pos - this->GetEntity().GetPosition(), sphere.radius, plane.begin(), plane.end(), true) != umath::intersection::Intersect::Outside)
				shouldPassData.renderFlags |= 1 << i;
			//if(pLightComponent->IsInCone(shouldPassData.entity,dir,ang) == true)
			//	shouldPassData.renderFlags |= 1<<i;
		}
		shouldPassData.shouldPass = (shouldPassData.renderFlags != 0) ? true : false;
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntityMesh &>(evData.get());
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		shouldPassData.shouldPass = pLightComponent.valid() && pLightComponent->IsInRange(shouldPassData.entity, shouldPassData.mesh);
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix &>(evData.get());
		trData.transformation = &MVPBias<6>::GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) {
		for(auto i = 0; i < 6; i++)
			UpdateTransformationMatrix(i);
	});
	BindEvent(CLightComponent::EVENT_HANDLE_SHADOW_MAP, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto shadowC = GetEntity().AddComponent<CShadowComponent>(true);
		if(shadowC.expired())
			return util::EventReply::Unhandled;
		static_cast<CEHandleShadowMap &>(evData.get()).resultShadow = shadowC.get();
		shadowC->SetType(CShadowComponent::Type::Cube);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CRadiusComponent::EVENT_ON_RADIUS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateFrustumPlanes(); });

	m_bSkipMatrixUpdate = true;
	UpdateProjectionMatrix();
	m_bSkipMatrixUpdate = false;
	//SetProjectionMatrix(glm::perspective<float>(35.f *2.f,1.f,2.f,m_distance));
	//SetViewMatrix(glm::lookAt(GetPosition(),GetPosition() +Vector3(1,0,0),Vector3(0,1,0)));
	for(auto i = 0; i < 6; i++)
		UpdateTransformationMatrix(i);

	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.valid())
		pLightComponent->UpdateTransformationMatrix(GetBiasTransformationMatrix(), GetViewMatrix(), GetProjectionMatrix());
	UpdateFrustumPlanes();
}
void CLightPointComponent::SetShadowDirty()
{
	for(auto &pComponent : GetEntity().GetComponents()) {
		if(pComponent.expired() || typeid(*pComponent) != typeid(CShadowComponent))
			continue;
		static_cast<CShadowComponent &>(*pComponent).SetDirty(true);
	}
}
void CLightPointComponent::UpdateFrustumPlanes()
{
	// Note: Up direction doesn't matter, as long as it's axis-aligned
	// and not the same as the respective directions from the 'directions' array
	static const std::array<Vector3, 6> upDirs = {Vector3(0.f, 1.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector3(0.f, 0.f, 1.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 1.f, 0.f)};
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto radius = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f;
	for(auto i = decltype(directions.size()) {0}; i < directions.size(); ++i) {
		m_frustumPlanes.at(i).clear();
		pragma::CCameraComponent::GetFrustumPlanes(m_frustumPlanes.at(i), 2.f /* nearZ */, radius /* farZ */, 90.f /* fov */, 1.f /* aspectRatio */, Vector3 {}, directions.at(i), upDirs.at(i));
	}
}
void CLightPointComponent::UpdateProjectionMatrix()
{
	auto scale = Vector3(-1.f, -1.f, 1.f); // Vulkan TODO
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto p = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(90.f)), 1.f, 2.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f); // Vulkan TODO
	p = glm::scale(p, scale);
	SetProjectionMatrix(p);
	UpdateViewMatrices();
}
void CLightPointComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightPointComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CRadiusComponent)) {
		static_cast<CRadiusComponent &>(component).GetRadiusProperty()->AddCallback([this](std::reference_wrapper<const float> oldRadius, std::reference_wrapper<const float> radius) {
			SetShadowDirty();
			UpdateProjectionMatrix();
		});
	}
	else if(typeid(component) == typeid(CTransformComponent)) {
		auto &trC = static_cast<CTransformComponent &>(component);
		FlagCallbackForRemoval(trC.AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::PositionChanged) == false)
				                         return util::EventReply::Unhandled;
			                         SetShadowDirty();
			                         UpdateViewMatrices();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CLightComponent))
		static_cast<CLightComponent &>(component).SetLight(*this);
}
void CLightPointComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

const std::array<std::vector<umath::Plane>, 6u> &CLightPointComponent::GetFrustumPlanes() const { return m_frustumPlanes; }
const std::vector<umath::Plane> &CLightPointComponent::GetFrustumPlanes(CubeMapSide side) const { return m_frustumPlanes.at(umath::to_integral(side)); }

/////////////

void CEnvLightPoint::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
	AddComponent<CLightPointComponent>();
}

/////////////

void CLightPointComponent::UpdateViewMatrices()
{
	auto b = m_bSkipMatrixUpdate;
	m_bSkipMatrixUpdate = true;
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Left)], Vector3(0, 1, 0)), umath::to_integral(CubeMapSide::Left)); //umat::look_at(pos,pos +Vector3(1,0,0),Vector3(0,1,0)),1); // Vulkan TODO
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Right)], Vector3(0, 1, 0)), umath::to_integral(CubeMapSide::Right));
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Top)], Vector3(0, 0, -1)), umath::to_integral(CubeMapSide::Top));
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Bottom)], Vector3(0, 0, 1)), umath::to_integral(CubeMapSide::Bottom));
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Front)], Vector3(0, 1, 0)), umath::to_integral(CubeMapSide::Front));
	SetViewMatrix(glm::lookAtRH(pos, pos + directions[umath::to_integral(CubeMapSide::Back)], Vector3(0, 1, 0)), umath::to_integral(CubeMapSide::Back));
	m_bSkipMatrixUpdate = b;
	if(m_bSkipMatrixUpdate == false) {
		for(auto i = 0; i < 6; i++)
			UpdateTransformationMatrix(i);
	}
}

void CLightPointComponent::UpdateTransformationMatrix(unsigned int j) // TODO This is called every time one of the matrices is changed; Only needs to be called once, after they're ALL changed!
{
	if(m_bSkipMatrixUpdate == true)
		return;
	MVPBias<6>::UpdateTransformationMatrix(j);
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.expired())
		return;
	auto &shadowBuffer = pLightComponent->GetShadowBuffer();
	if(shadowBuffer == nullptr)
		return;
	std::array<Mat4, 3> matrices = {GetBiasTransformationMatrix(), GetViewMatrix(4), GetProjectionMatrix()};
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(shadowBuffer, 0ull, matrices);
}
