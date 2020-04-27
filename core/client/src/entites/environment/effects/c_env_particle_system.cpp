/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/baseentity_events.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_particle_system,CEnvParticleSystem);

void CParticleSystemComponent::Initialize()
{
	BaseEnvParticleSystemComponent::Initialize();

	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_ON,[this](std::reference_wrapper<ComponentEvent> evData) {
		Start();
	});
	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_OFF,[this](std::reference_wrapper<ComponentEvent> evData) {
		Stop();
	});
	BindEvent(CIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<pragma::CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setcontinuous",false))
		{
			SetContinuous(util::to_boolean(inputData.data));
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		return HandleKeyValue(kvData.key,kvData.value);
	});
	BindEvent(CAnimatedComponent::EVENT_SHOULD_UPDATE_BONES,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEShouldUpdateBones&>(evData.get()).shouldUpdate = IsActive();
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
	{
		FlagCallbackForRemoval(pTrComponent->GetPosProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
			if(IsActive() == false)
				return;
			for(auto it=m_childSystems.begin();it!=m_childSystems.end();++it)
			{
				auto &hChild = *it;
				if(hChild.valid())
				{
					auto pTrComponent = hChild->GetEntity().GetTransformComponent();
					if(pTrComponent.valid())
						pTrComponent->SetPosition(pos);
				}
			}
		}),CallbackType::Entity);
	}
}
void CParticleSystemComponent::OnEntitySpawn()
{
	CreateParticle();
	BaseEnvParticleSystemComponent::OnEntitySpawn();
}
util::EventReply CParticleSystemComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEnvParticleSystemComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;

	return util::EventReply::Unhandled;
}
void CParticleSystemComponent::ReceiveData(NetPacket &packet)
{
	SetParticleFile(packet->ReadString());
	m_particleName = packet->ReadString();
}
void CParticleSystemComponent::SetParticleFile(const std::string &fileName)
{
	BaseEnvParticleSystemComponent::SetParticleFile(fileName);
	CParticleSystemComponent::Precache(fileName);
}
void CParticleSystemComponent::SetRenderMode(RenderMode mode) {m_renderMode = mode;}
RenderMode CParticleSystemComponent::GetRenderMode() const {return m_renderMode;}

void CParticleSystemComponent::CreateParticle()
{
	if(SetupParticleSystem(m_particleName) == false)
		return;
	/*if(m_hCbRenderCallback.IsValid())
		m_hCbRenderCallback.Remove();
	m_hCbRenderCallback = particle->AddRenderCallback([this]() {
		auto &ent = static_cast<CBaseEntity&>(GetEntity());
		auto pAttComponent = ent.GetComponent<CAttachableComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
	});*/
}
void CParticleSystemComponent::SetRemoveOnComplete(bool b) {BaseEnvParticleSystemComponent::SetRemoveOnComplete(b);}

luabind::object CParticleSystemComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CParticleSystemComponentHandleWrapper>(l);}

///////////////

void CEnvParticleSystem::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CParticleSystemComponent>();
}
