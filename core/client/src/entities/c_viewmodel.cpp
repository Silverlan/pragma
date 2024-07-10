/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_attachment_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/model/model.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/logic_component.hpp>
#include <pragma/entities/components/parent_component.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(viewmodel, CViewModel);

extern DLLCLIENT CGame *c_game;

void CViewModelComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(CAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ent = GetEntity();
		auto pAttachableComponent = ent.GetComponent<CAttachmentComponent>();
		auto *parent = pAttachableComponent.valid() ? pAttachableComponent->GetParent() : nullptr;
		if(parent != nullptr && parent->IsCharacter()) {
			auto charComponent = parent->GetCharacterComponent();
			auto *wep = charComponent->GetActiveWeapon();
			if(wep != nullptr && wep->IsWeapon()) {
				if(static_cast<pragma::CWeaponComponent &>(*wep->GetWeaponComponent()).HandleViewModelAnimationEvent(this, static_cast<CEHandleAnimationEvent &>(evData.get()).animationEvent))
					return util::EventReply::Handled;
			}
		}
		return util::EventReply::Handled; // Always overwrite
	});
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto &hMdl = ent.GetModel();
		if(hMdl != nullptr) {
			auto anim = hMdl->GetAnimation(static_cast<CEOnAnimationComplete &>(evData.get()).animation);
			if(anim != nullptr && anim->HasFlag(FAnim::Loop) == true)
				return;
		}
		auto animComponent = ent.GetAnimatedComponent();
		if(animComponent.valid())
			animComponent->PlayActivity(Activity::VmIdle);
	});
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_ANIMATION_RESET, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto *wepC = GetWeapon();
		if(wepC)
			wepC->UpdateDeployState();
	});

	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	ent.AddComponent<pragma::CTransformComponent>();
	ent.AddComponent<pragma::LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->AddToRenderGroup("firstperson");
		pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::None);
		pRenderComponent->SetCastShadows(false);
	}
	auto pMdlComponent = ent.AddComponent<CModelComponent>();
	if(pMdlComponent.valid())
		pMdlComponent->SetModel("weapons/v_soldier.wmd");
	ent.AddComponent<CAnimatedComponent>();
}

static auto cvViewFov = GetClientConVar("cl_fov_viewmodel");
void CViewModelComponent::SetViewFOV(float fov)
{
	m_viewFov = fov;
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->IsPlayer() == false)
		return;
	static_cast<pragma::CPlayerComponent *>(parent->GetPlayerComponent().get())->UpdateViewFOV();
}
float CViewModelComponent::GetViewFOV() const
{
	if(std::isnan(m_viewFov) == true)
		return cvViewFov->GetFloat();
	return m_viewFov;
}
CPlayerComponent *CViewModelComponent::GetPlayer()
{
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->IsPlayer() == false)
		return nullptr;
	return static_cast<pragma::CPlayerComponent *>(parent->GetPlayerComponent().get());
}
CWeaponComponent *CViewModelComponent::GetWeapon()
{
	auto *pl = GetPlayer();
	if(pl == nullptr)
		return nullptr;
	auto charC = pl->GetEntity().GetComponent<CCharacterComponent>();
	if(charC.expired())
		return nullptr;
	auto *weapon = charC->GetActiveWeapon();
	auto weaponC = weapon ? weapon->GetComponent<CWeaponComponent>() : pragma::ComponentHandle<CWeaponComponent> {};
	return weaponC.get();
}

void CViewModelComponent::SetViewModelOffset(const Vector3 &offset)
{
	m_viewModelOffset = offset;
	auto *pl = GetPlayer();
	if(pl == nullptr)
		return;
	pl->UpdateViewModelTransform();
}
const Vector3 &CViewModelComponent::GetViewModelOffset() const { return m_viewModelOffset; }
void CViewModelComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void CViewModel::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewModelComponent>();
}
