#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_spot,CEnvLightSpot);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
CLightSpotComponent::CLightSpotComponent(BaseEntity &ent)
	: BaseEnvLightSpotComponent(ent)
{}
void CLightSpotComponent::Initialize()
{
	BaseEnvLightSpotComponent::Initialize();

	BindEvent(CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix&>(evData.get());
		trData.transformation = &MVPBias<1>::GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED,[this](std::reference_wrapper<ComponentEvent> evData) {
		UpdateTransformMatrix();
	});
	BindEventUnhandled(CRadiusComponent::EVENT_ON_RADIUS_CHANGED,[this](std::reference_wrapper<ComponentEvent> evData) {
		SetShadowDirty();
		UpdateProjectionMatrix();
	});
	m_angInnerCutoff->AddCallback([this](std::reference_wrapper<const float> oldAng,std::reference_wrapper<const float> newAng) {
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.expired())
			return;
		auto &bufferData = pLightComponent->GetBufferData();
		bufferData.cutoffInner = static_cast<float>(umath::cos(umath::deg_to_rad(newAng.get())));
		auto &renderBuffer = pLightComponent->GetRenderBuffer();
		if(renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(renderBuffer,offsetof(LightBufferData,cutoffInner),bufferData.cutoffInner);
	});
	m_angOuterCutoff->AddCallback([this](std::reference_wrapper<const float> oldAng,std::reference_wrapper<const float> newAng) {
		SetShadowDirty();
		UpdateProjectionMatrix();
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.expired())
			return;
		auto &bufferData = pLightComponent->GetBufferData();
		bufferData.cutoffOuter = static_cast<float>(umath::cos(umath::deg_to_rad(newAng.get())));
		auto &renderBuffer = pLightComponent->GetRenderBuffer();
		if(renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(renderBuffer,offsetof(LightBufferData,cutoffOuter),bufferData.cutoffOuter);
	});

	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.valid())
		pLightComponent->UpdateTransformationMatrix(GetBiasTransformationMatrix(),GetViewMatrix(),GetProjectionMatrix());
}
Bool CLightSpotComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetConeStartOffset)
	{
		auto coneStartOffset = packet->Read<float>();
		SetConeStartOffset(coneStartOffset);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
void CLightSpotComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightSpotComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CLightComponent))
		static_cast<CLightComponent&>(component).SetLight(*this);
	else if(typeid(component) == typeid(CTransformComponent))
	{
		FlagCallbackForRemoval(static_cast<CTransformComponent&>(component).GetPosProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
			SetShadowDirty();
			UpdateViewMatrices();
		}),CallbackType::Component,&component);
		FlagCallbackForRemoval(static_cast<CTransformComponent&>(component).GetOrientationProperty()->AddCallback([this](std::reference_wrapper<const Quat> oldRot,std::reference_wrapper<const Quat> rot) {
			SetShadowDirty();
			UpdateViewMatrices();
		}),CallbackType::Component,&component);
	}
}
void CLightSpotComponent::SetShadowDirty()
{
	for(auto &pComponent : GetEntity().GetComponents())
	{
		if(typeid(*pComponent) != typeid(CShadowComponent))
			continue;
		static_cast<CShadowComponent&>(*pComponent).SetDirty(true);
	}
}
void CLightSpotComponent::UpdateViewMatrices()
{
	//SetViewMatrix(glm::lookAtLH(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0)));
	//SetViewMatrix(umat::look_at(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0))); // Vulkan TODO
	//SetViewMatrix(umat::look_at(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0)));
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.expired())
		return;
	auto dir = pTrComponent->GetForward();
	SetViewMatrix(glm::lookAtRH(pTrComponent->GetPosition(),pTrComponent->GetPosition() +dir,uvec::get_perpendicular(dir)));
	UpdateTransformMatrix();
}
void CLightSpotComponent::ReceiveData(NetPacket &packet)
{
	*m_angOuterCutoff = packet->Read<float>();
	*m_angInnerCutoff = packet->Read<float>();
	auto coneStartOffset = packet->Read<float>();
	SetConeStartOffset(coneStartOffset);
}
void CLightSpotComponent::SetConeStartOffset(float offset)
{
	BaseEnvLightSpotComponent::SetConeStartOffset(offset);
	auto pLightComponent = GetEntity().GetComponent<pragma::CLightComponent>();
	auto &bufferData = pLightComponent->GetBufferData();
	bufferData.direction.w = offset;
	auto &renderBuffer = pLightComponent->GetRenderBuffer();
	if(renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(renderBuffer,offsetof(LightBufferData,direction) +offsetof(Vector4,w),offset);
}
luabind::object CLightSpotComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightSpotComponentHandleWrapper>(l);}
void CLightSpotComponent::UpdateTransformMatrix()
{
	auto pLightComponent = GetEntity().GetComponent<pragma::CLightComponent>();
	if(pLightComponent.expired())
		return;
	auto &shadowBuffer = pLightComponent->GetShadowBuffer();
	if(shadowBuffer == nullptr)
		return;
	std::array<Mat4,3> matrices = {GetBiasTransformationMatrix(),GetViewMatrix(),GetProjectionMatrix()};
	c_engine->ScheduleRecordUpdateBuffer(shadowBuffer,0ull,matrices);
}
void CLightSpotComponent::UpdateProjectionMatrix()
{
	auto scale = Vector3(-1.f,-1.f,1.f); // Vulkan TODO
	//SetProjectionMatrix(glm::perspectiveLH<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,2.f,m_distance));
	//auto p = glm::perspective<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	//p = glm::scale(p,scale); /* Shadow TODO */
	/*static Mat4 transform{
		1.f,0.f,0.f,0.f,
		0.f,-1.f,0.f,0.f,
		0.f,0.f,0.5f,0.5f,
		0.f,0.f,0.f,1.f
	};*/
	//auto p = glm::perspective<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto p = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(GetOuterCutoffAngle() *2.f)),1.f,2.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
	//p = transform *p;
	p = glm::scale(p,scale); /* Shadow TODO */
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
#pragma optimize("",on)
