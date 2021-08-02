/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/shared_spawnflags.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/basetoggle.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_camera,CEnvCamera);

extern DLLCLIENT CGame *c_game;

CCameraComponent::~CCameraComponent()
{
	if(m_cbCameraUpdate.IsValid())
		m_cbCameraUpdate.Remove();
}
void CCameraComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEnvCameraComponent::Save(udm);
}
void CCameraComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEnvCameraComponent::Load(udm,version);
}
void CCameraComponent::OnEntitySpawn()
{
	BaseEnvCameraComponent::OnEntitySpawn();
	UpdateState();
}
void CCameraComponent::UpdateState()
{
	if(GetEntity().IsSpawned() == false)
		return;
	auto toggleC = GetEntity().GetComponent<CToggleComponent>();
	if(toggleC.expired() || toggleC->IsTurnedOn())
	{
		if(m_cbCameraUpdate.IsValid())
			m_cbCameraUpdate.Remove();
		auto *renderScene = c_game->GetRenderScene();
		if(renderScene && static_cast<CBaseEntity&>(GetEntity()).IsInScene(*renderScene))
		{
			auto *pl = c_game->GetLocalPlayer();
			if(pl != nullptr)
				pl->SetObserverMode(OBSERVERMODE::THIRDPERSON);
			m_cbCameraUpdate = c_game->AddCallback("CalcView",FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>::Create(
				[this](std::reference_wrapper<Vector3> refPos,std::reference_wrapper<Quat> refRot,
					std::reference_wrapper<Quat> rotMod
				) {
				auto &ent = GetEntity();
				auto *pToggleComponent = static_cast<pragma::BaseToggleComponent*>(ent.FindComponent("toggle").get());
				if(pToggleComponent != nullptr && pToggleComponent->IsTurnedOn() == false)
					return;
				auto pTrComponent = ent.GetTransformComponent();
				auto &pos = refPos.get();
				auto &rot = refRot.get();
				if(pTrComponent == nullptr)
				{
					pos = {};
					rot = uquat::identity();
				}
				else
				{
					pos = pTrComponent->GetPosition();
					rot = pTrComponent->GetRotation();
				}
			}));
			renderScene->SetActiveCamera(*this);
		}
		return;
	}

	auto *renderScene = c_game->GetRenderScene();
	if(renderScene)
	{
		if(static_cast<CBaseEntity&>(GetEntity()).IsInScene(*renderScene))
		{
			auto *pl = c_game->GetLocalPlayer();
			if(pl != nullptr)
				pl->SetObserverMode(OBSERVERMODE::FIRSTPERSON);
			if(m_cbCameraUpdate.IsValid())
				m_cbCameraUpdate.Remove();
		}

		if(renderScene->GetActiveCamera().get() == this)
		{
			EntityIterator entIt {*c_game};
			entIt.AttachFilter<TEntityIteratorFilterComponent<CCameraComponent>>();
			for(auto *ent : entIt)
			{
				auto toggleC = ent->GetComponent<CToggleComponent>();
				if((toggleC.valid() && toggleC->IsTurnedOn() == false) || static_cast<CBaseEntity*>(ent)->IsInScene(*renderScene) == false)
					continue;
				auto camC = ent->GetComponent<CCameraComponent>();
				renderScene->SetActiveCamera(*camC);
				break;
			}
		}
	}
}
util::EventReply CCameraComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEnvCameraComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON || eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
		UpdateState();
	return util::EventReply::Unhandled;
}
void CCameraComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

/////////

void CEnvCamera::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CCameraComponent>();
}
