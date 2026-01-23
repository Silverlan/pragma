// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.view_model;
import :entities.components.animated;
import :entities.components.attachment;
import :entities.components.character;
import :entities.components.player;
import :entities.components.render;
import :entities.components.transform;
import :entities.components.weapon;

using namespace pragma;

void CViewModelComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(cAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &ent = GetEntity();
		auto pAttachableComponent = ent.GetComponent<CAttachmentComponent>();
		auto *parent = pAttachableComponent.valid() ? pAttachableComponent->GetParent() : nullptr;
		if(parent != nullptr && parent->IsCharacter()) {
			auto charComponent = parent->GetCharacterComponent();
			auto *wep = charComponent->GetActiveWeapon();
			if(wep != nullptr && wep->IsWeapon()) {
				if(static_cast<CWeaponComponent &>(*wep->GetWeaponComponent()).HandleViewModelAnimationEvent(this, static_cast<CEHandleAnimationEvent &>(evData.get()).animationEvent))
					return util::EventReply::Handled;
			}
		}
		return util::EventReply::Handled; // Always overwrite
	});
	BindEventUnhandled(cAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<ComponentEvent> evData) {
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
	BindEventUnhandled(cAnimatedComponent::EVENT_ON_ANIMATION_RESET, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto *wepC = static_cast<CWeaponComponent *>(GetWeapon());
		if(wepC)
			wepC->UpdateDeployState();
	});

	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	ent.AddComponent<CTransformComponent>();
	ent.AddComponent<LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
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

static auto cvViewFov = console::get_client_con_var("cl_fov_viewmodel");
void CViewModelComponent::SetViewFOV(float fov)
{
	m_viewFov = fov;
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->IsPlayer() == false)
		return;
	static_cast<CPlayerComponent *>(parent->GetPlayerComponent().get())->UpdateViewFOV();
}
float CViewModelComponent::GetViewFOV() const
{
	if(std::isnan(m_viewFov) == true)
		return cvViewFov->GetFloat();
	return m_viewFov;
}
BasePlayerComponent *CViewModelComponent::GetPlayer()
{
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->IsPlayer() == false)
		return nullptr;
	return static_cast<CPlayerComponent *>(parent->GetPlayerComponent().get());
}
BaseWeaponComponent *CViewModelComponent::GetWeapon()
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
	static_cast<CPlayerComponent *>(pl)->UpdateViewModelTransform();
}
const Vector3 &CViewModelComponent::GetViewModelOffset() const { return m_viewModelOffset; }
void CViewModelComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void CViewModel::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewModelComponent>();
}
