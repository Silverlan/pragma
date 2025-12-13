// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cstddef>

module pragma.client;

import :entities.components.lights.spot;
import :engine;

using namespace pragma;

CLightSpotComponent::CLightSpotComponent(ecs::BaseEntity &ent) : BaseEnvLightSpotComponent(ent) {}
bool CLightSpotComponent::ShouldTransmitNetData() const { return true; }
void CLightSpotComponent::Initialize()
{
	BaseEnvLightSpotComponent::Initialize();

	BindEvent(cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix &>(evData.get());
		trData.transformation = &GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateTransformMatrix(); });
	BindEventUnhandled(cRadiusComponent::EVENT_ON_RADIUS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		SetShadowDirty();
		UpdateProjectionMatrix();
	});
	m_blendFraction->AddCallback([this](std::reference_wrapper<const float> oldFraction, std::reference_wrapper<const float> newFraction) { UpdateInnerConeAngle(); });

	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.valid())
		pLightComponent->UpdateTransformationMatrix(GetBiasTransformationMatrix(), GetViewMatrix(), GetProjectionMatrix());
}
void CLightSpotComponent::OnEntitySpawn()
{
	BaseEnvLightSpotComponent::OnEntitySpawn();
	UpdateViewMatrices();
}
void CLightSpotComponent::SetFieldAngleComponent(BaseFieldAngleComponent &c)
{
	BaseEnvLightSpotComponent::SetFieldAngleComponent(c);
	c.GetFieldAngleProperty()->AddCallback([this](std::reference_wrapper<const float> oldAng, std::reference_wrapper<const float> newAng) {
		SetShadowDirty();
		UpdateProjectionMatrix();
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.expired())
			return;
		auto &bufferData = pLightComponent->GetBufferData();
		bufferData.outerConeHalfAngle = static_cast<math::Radian>(math::deg_to_rad(newAng.get() / 2.f));
		auto &renderBuffer = pLightComponent->GetRenderBuffer();
		if(renderBuffer != nullptr)
			get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(renderBuffer, offsetof(LightBufferData, outerConeHalfAngle), bufferData.outerConeHalfAngle);
		UpdateInnerConeAngle();

		if(pLightComponent->GetLightIntensityType() == CBaseLightComponent::LightIntensityType::Lumen) {
			// Lumen light intensity has to be converted to candela and the conversion requires knowledge of the cone radius,
			// which means we have to update it here
			pLightComponent->UpdateLightIntensity();
		}
	});
}
void CLightSpotComponent::UpdateInnerConeAngle()
{
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.expired())
		return;
	auto &bufferData = pLightComponent->GetBufferData();
	bufferData.innerConeHalfAngle = math::deg_to_rad(CalcInnerConeAngle(GetOuterConeAngle(), GetBlendFraction()) / 2.f);
	auto &renderBuffer = pLightComponent->GetRenderBuffer();
	if(renderBuffer != nullptr)
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(renderBuffer, offsetof(LightBufferData, innerConeHalfAngle), bufferData.innerConeHalfAngle);
}
Bool CLightSpotComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetConeStartOffset) {
		auto coneStartOffset = packet->Read<float>();
		SetConeStartOffset(coneStartOffset);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
void CLightSpotComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightSpotComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CLightComponent))
		static_cast<CLightComponent &>(component).SetLight(*this);
	else if(typeid(component) == typeid(CTransformComponent)) {
		auto &trC = static_cast<CTransformComponent &>(component);
		FlagCallbackForRemoval(trC.AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			                         if(math::is_flag_set(static_cast<CEOnPoseChanged &>(evData.get()).changeFlags, TransformChangeFlags::PositionChanged | TransformChangeFlags::RotationChanged) == false)
				                         return util::EventReply::Unhandled;
			                         SetShadowDirty();
			                         UpdateViewMatrices();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CFieldAngleComponent))
		SetFieldAngleComponent(static_cast<CFieldAngleComponent &>(component));
}
void CLightSpotComponent::SetShadowDirty()
{
	for(auto &pComponent : GetEntity().GetComponents()) {
		if(pComponent.expired() || typeid(*pComponent) != typeid(CShadowComponent))
			continue;
		static_cast<CShadowComponent &>(*pComponent).SetDirty(true);
	}
}
void CLightSpotComponent::UpdateViewMatrices()
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto dir = pTrComponent->GetForward();
	SetViewMatrix(glm::gtc::lookAtRH(pTrComponent->GetPosition(), pTrComponent->GetPosition() + dir, uvec::get_perpendicular(dir)));
	UpdateTransformMatrix();
}
void CLightSpotComponent::ReceiveData(NetPacket &packet)
{
	*m_blendFraction = packet->Read<float>();
	auto coneStartOffset = packet->Read<float>();
	SetConeStartOffset(coneStartOffset);
}
void CLightSpotComponent::SetConeStartOffset(float offset)
{
	BaseEnvLightSpotComponent::SetConeStartOffset(offset);
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	auto &bufferData = pLightComponent->GetBufferData();
	bufferData.direction.w = offset;
	auto &renderBuffer = pLightComponent->GetRenderBuffer();
	if(renderBuffer != nullptr)
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(renderBuffer, offsetof(LightBufferData, direction) + offsetof(Vector4, w), offset);
}
void CLightSpotComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightSpotComponent::UpdateTransformMatrix()
{
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.expired())
		return;
	auto &shadowBuffer = pLightComponent->GetShadowBuffer();
	if(shadowBuffer == nullptr)
		return;
	std::array<Mat4, 3> matrices = {GetBiasTransformationMatrix(), GetViewMatrix(), GetProjectionMatrix()};
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(shadowBuffer, 0ull, matrices);
}
void CLightSpotComponent::UpdateProjectionMatrix()
{
	auto scale = Vector3(-1.f, -1.f, 1.f); // Vulkan TODO
	//SetProjectionMatrix(glm::gtc::perspectiveLH<float>(CFloat(pragma::math::deg_to_rad(m_angOuterCutoff *2.f)),1.f,2.f,m_distance));
	//auto p = glm::gtc::perspective<float>(CFloat(pragma::math::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	//p = glm::gtc::scale(p,scale); /* Shadow TODO */
	/*static Mat4 transform{
		1.f,0.f,0.f,0.f,
		0.f,-1.f,0.f,0.f,
		0.f,0.f,0.5f,0.5f,
		0.f,0.f,0.f,1.f
	};*/
	//auto p = glm::gtc::perspective<float>(CFloat(pragma::math::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto p = glm::gtc::perspectiveRH<float>(CFloat(math::deg_to_rad(GetOuterConeAngle())), 1.f, 2.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
	//p = transform *p;
	p = glm::gtc::scale(p, scale); /* Shadow TODO */
	SetProjectionMatrix(p);
	UpdateTransformMatrix();
}

/////////////

void CEnvLightSpot::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
	AddComponent<CLightSpotComponent>();
}
