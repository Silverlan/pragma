// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.lights.point;
import :engine;
import :entities.components.lights.shadow;
import :entities.components.radius;
import :entities.components.render;
import :entities.components.transform;

using namespace pragma;

static const std::array<Vector3, 6> directions = {Vector3(1.f, 0.f, 0.f), Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector3(0.f, 0.f, -1.f)};

CLightPointComponent::CLightPointComponent(ecs::BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
void CLightPointComponent::Initialize()
{
	BaseEnvLightPointComponent::Initialize();
	BindEvent(cLightComponent::EVENT_SHOULD_PASS_ENTITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
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
			if(math::intersection::sphere_in_plane_mesh(sphere.pos - this->GetEntity().GetPosition(), sphere.radius, plane.begin(), plane.end(), true) != math::intersection::Intersect::Outside)
				shouldPassData.renderFlags |= 1 << i;
			//if(pLightComponent->IsInCone(shouldPassData.entity,dir,ang) == true)
			//	shouldPassData.renderFlags |= 1<<i;
		}
		shouldPassData.shouldPass = (shouldPassData.renderFlags != 0) ? true : false;
		return util::EventReply::Handled;
	});
	BindEvent(cLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntityMesh &>(evData.get());
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		shouldPassData.shouldPass = pLightComponent.valid() && pLightComponent->IsInRange(shouldPassData.entity, shouldPassData.mesh);
		return util::EventReply::Handled;
	});
	BindEvent(cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix &>(evData.get());
		trData.transformation = &GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) {
		for(auto i = 0; i < 6; i++)
			UpdateTransformationMatrix(i);
	});
	BindEvent(cLightComponent::EVENT_HANDLE_SHADOW_MAP, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto shadowC = GetEntity().AddComponent<CShadowComponent>(true);
		if(shadowC.expired())
			return util::EventReply::Unhandled;
		static_cast<CEHandleShadowMap &>(evData.get()).resultShadow = shadowC.get();
		shadowC->SetType(CShadowComponent::Type::Cube);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(cRadiusComponent::EVENT_ON_RADIUS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateFrustumPlanes(); });

	m_bSkipMatrixUpdate = true;
	UpdateProjectionMatrix();
	m_bSkipMatrixUpdate = false;
	//SetProjectionMatrix(glm::gtc::perspective<float>(35.f *2.f,1.f,2.f,m_distance));
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
		CCameraComponent::GetFrustumPlanes(m_frustumPlanes.at(i), 2.f /* nearZ */, radius /* farZ */, 90.f /* fov */, 1.f /* aspectRatio */, Vector3 {}, directions.at(i), upDirs.at(i));
	}
}
void CLightPointComponent::UpdateProjectionMatrix()
{
	auto scale = Vector3(-1.f, -1.f, 1.f); // Vulkan TODO
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto p = glm::gtc::perspectiveRH<float>(CFloat(math::deg_to_rad(90.f)), 1.f, 2.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f); // Vulkan TODO
	p = glm::gtc::scale(p, scale);
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
		FlagCallbackForRemoval(trC.AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			                         if(math::is_flag_set(static_cast<CEOnPoseChanged &>(evData.get()).changeFlags, TransformChangeFlags::PositionChanged) == false)
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
void CLightPointComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

const std::array<std::vector<math::Plane>, 6u> &CLightPointComponent::GetFrustumPlanes() const { return m_frustumPlanes; }
const std::vector<math::Plane> &CLightPointComponent::GetFrustumPlanes(rendering::CubeMapSide side) const { return m_frustumPlanes.at(math::to_integral(side)); }

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
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Left)], Vector3(0, 1, 0)), math::to_integral(rendering::CubeMapSide::Left)); //umat::look_at(pos,pos +Vector3(1,0,0),Vector3(0,1,0)),1); // Vulkan TODO
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Right)], Vector3(0, 1, 0)), math::to_integral(rendering::CubeMapSide::Right));
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Top)], Vector3(0, 0, -1)), math::to_integral(rendering::CubeMapSide::Top));
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Bottom)], Vector3(0, 0, 1)), math::to_integral(rendering::CubeMapSide::Bottom));
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Front)], Vector3(0, 1, 0)), math::to_integral(rendering::CubeMapSide::Front));
	SetViewMatrix(glm::gtc::lookAtRH(pos, pos + directions[math::to_integral(rendering::CubeMapSide::Back)], Vector3(0, 1, 0)), math::to_integral(rendering::CubeMapSide::Back));
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
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(shadowBuffer, 0ull, matrices);
}
