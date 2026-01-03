// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.weapon;
import :client_state;
import :game;

using namespace pragma;

ComponentEventId cWeaponComponent::EVENT_ATTACH_TO_OWNER = INVALID_COMPONENT_ID;
void CWeaponComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseWeaponComponent::RegisterEvents(componentManager, registerEvent);
	cWeaponComponent::EVENT_ATTACH_TO_OWNER = registerEvent("ATTACH_TO_OWNER", ComponentEventInfo::Type::Explicit);
}

std::vector<CWeaponComponent *> CWeaponComponent::s_weapons;
const std::vector<CWeaponComponent *> &CWeaponComponent::GetAll() { return s_weapons; }
unsigned int CWeaponComponent::GetWeaponCount() { return CUInt32(s_weapons.size()); }

CWeaponComponent::CWeaponComponent(ecs::BaseEntity &ent) : BaseWeaponComponent(ent), CBaseNetComponent() { s_weapons.push_back(this); }

CWeaponComponent::~CWeaponComponent()
{
	auto it = std::find(s_weapons.begin(), s_weapons.end(), this);
	if(it != s_weapons.end())
		s_weapons.erase(it);
	ClearOwnerCallbacks();
}

void CWeaponComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CWeaponComponent::ReceiveData(NetPacket &packet)
{
	auto primAmmoType = packet->Read<UInt32>();
	auto secAmmoType = packet->Read<UInt32>();
	auto primClipSize = packet->Read<UInt16>();
	auto secClipSize = packet->Read<UInt16>();
	auto primMaxClipSize = packet->Read<UInt16>();
	auto secMaxClipSize = packet->Read<UInt16>();
	SetPrimaryAmmoType(primAmmoType);
	SetSecondaryAmmoType(secAmmoType);
	SetPrimaryClipSize(primClipSize);
	SetSecondaryClipSize(secClipSize);
	SetMaxPrimaryClipSize(primMaxClipSize);
	SetMaxSecondaryClipSize(secMaxClipSize);
}

bool CWeaponComponent::HandleViewModelAnimationEvent(CViewModelComponent *, const AnimationEvent &) { return false; }

void CWeaponComponent::UpdateViewModel()
{
	if(IsDeployed() == false)
		return;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	auto &vmEnt = static_cast<ecs::CBaseEntity &>(vm->GetEntity());
	auto pRenderComponentVm = vmEnt.GetRenderComponent();
	auto mdlComponentVm = vmEnt.GetModelComponent();
	if(m_viewModel.has_value()) {
		if(m_viewModel->empty() == true) {
			if(pRenderComponentVm)
				pRenderComponentVm->SetSceneRenderPass(rendering::SceneRenderPass::None);
			vm->SetViewModelOffset({});
			return;
		}
		if(mdlComponentVm)
			mdlComponentVm->SetModel(*m_viewModel);
	}
	if(pRenderComponentVm)
		pRenderComponentVm->SetSceneRenderPass(rendering::SceneRenderPass::View);
	vm->SetViewModelOffset(GetViewModelOffset());
	vm->SetViewFOV(GetViewFOV());
}

static auto cvViewFov = console::get_client_con_var("cl_fov_viewmodel");
void CWeaponComponent::SetViewModelOffset(const Vector3 &offset)
{
	m_viewModelOffset = offset;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	vm->SetViewModelOffset(offset);
}
const Vector3 &CWeaponComponent::GetViewModelOffset() const { return m_viewModelOffset; }
void CWeaponComponent::SetViewFOV(math::Degree fov)
{
	m_viewFov = fov;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	vm->SetViewFOV(fov);
}
void CWeaponComponent::UpdateObserver(BaseObserverComponent *observer)
{
	if(m_cbOnOwnerObserverModeChanged.IsValid())
		m_cbOnOwnerObserverModeChanged.Remove();
	if(observer) {
		m_cbOnOwnerObserverModeChanged = observer->GetObserverModeProperty()->AddCallback([this](const std::reference_wrapper<const ObserverMode> oldObserverMode, const std::reference_wrapper<const ObserverMode> observerMode) { UpdateOwnerAttachment(); });
		FlagCallbackForRemoval(m_cbOnOwnerObserverModeChanged, CallbackType::Component);
	}
}
void CWeaponComponent::Initialize()
{
	BaseWeaponComponent::Initialize();

	BindEvent(cRenderComponent::EVENT_SHOULD_DRAW, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw &>(evData.get());
		auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
		auto pRenderComponent = ent.GetComponent<CRenderComponent>();
		auto renderMode = pRenderComponent.valid() ? pRenderComponent->GetSceneRenderPass() : rendering::SceneRenderPass::None;
		if(renderMode != rendering::SceneRenderPass::None) {
			if(renderMode == rendering::SceneRenderPass::View) {
				auto *pl = get_cgame()->GetLocalPlayer();
				auto *plComponent = static_cast<CPlayerComponent *>(pl->GetEntity().GetPlayerComponent().get());
				if(pl->IsInFirstPersonMode() == false) {
					shouldDrawData.shouldDraw = false;
					return util::EventReply::Handled;
				}
				return util::EventReply::Unhandled;
			}
			auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
			if(owner != nullptr) {
				auto charComponent = owner->GetCharacterComponent();
				if(charComponent.valid()) {
					if(charComponent->GetActiveWeapon() != &GetEntity()) {
						shouldDrawData.shouldDraw = false;
						return util::EventReply::Handled;
					}
					return util::EventReply::Unhandled;
				}
			}
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(cRenderComponent::EVENT_SHOULD_DRAW_SHADOW, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		if(m_bDeployed == false) {
			static_cast<CEShouldDraw &>(evData.get()).shouldDraw = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(cOwnableComponent::EVENT_ON_OWNER_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		UpdateOwnerAttachment();
		ClearOwnerCallbacks();
		auto &ownerChangedData = static_cast<CEOnOwnerChanged &>(evData.get());
		if(ownerChangedData.newOwner != nullptr && ownerChangedData.newOwner->IsPlayer() == true && static_cast<CPlayerComponent *>(ownerChangedData.newOwner->GetPlayerComponent().get())->IsLocalPlayer() == true) {
			auto plComponent = ownerChangedData.newOwner->GetPlayerComponent();
			auto *observableC = plComponent->GetObservableComponent();
			if(observableC) {
				m_cbOnObserverChanged = observableC->AddEventCallback(cObservableComponent::EVENT_ON_OBSERVER_CHANGED, [this, observableC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
					UpdateObserver(observableC->GetObserver());
					return util::EventReply::Unhandled;
				});
				FlagCallbackForRemoval(m_cbOnObserverChanged, CallbackType::Component);
				UpdateObserver(observableC->GetObserver());
			}
		}
	});
	BindEventUnhandled(cWeaponComponent::EVENT_ON_DEPLOY, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto *renderC = static_cast<ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
		if(renderC)
			renderC->UpdateShouldDrawState();
	});
	BindEventUnhandled(cWeaponComponent::EVENT_ON_HOLSTER, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto *renderC = static_cast<ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
		if(renderC)
			renderC->UpdateShouldDrawState();
	});
}
math::Degree CWeaponComponent::GetViewFOV() const
{
	if(m_viewFov.has_value() == false)
		return cvViewFov->GetFloat();
	return *m_viewFov;
}

bool CWeaponComponent::IsInFirstPersonMode() const
{
	if(IsDeployed() == false)
		return false;
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr || owner->IsPlayer() == false)
		return false;
	auto *plComponent = static_cast<CPlayerComponent *>(owner->GetPlayerComponent().get());
	return plComponent->IsLocalPlayer() && plComponent->IsInFirstPersonMode();
}

void CWeaponComponent::UpdateWorldModel()
{
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(!pRenderComponent)
		return;
	pRenderComponent->SetSceneRenderPass(IsInFirstPersonMode() ? ((math::is_flag_set(m_stateFlags, StateFlags::HideWorldModelInFirstPerson) == true) ? rendering::SceneRenderPass::None : rendering::SceneRenderPass::View) : rendering::SceneRenderPass::World);
}

void CWeaponComponent::SetViewModel(const std::string &mdl)
{
	m_viewModel = mdl;
	UpdateViewModel();
}
const std::optional<std::string> &CWeaponComponent::GetViewModelName() const { return m_viewModel; }

void CWeaponComponent::OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin)
{
	BaseWeaponComponent::OnFireBullets(bulletInfo, bulletOrigin, bulletDir, effectsOrigin);
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	auto &ent = GetEntity();
	if(owner != nullptr && owner->IsPlayer()) {
		auto *plComponent = static_cast<CPlayerComponent *>(owner->GetPlayerComponent().get());
		if(plComponent->IsLocalPlayer()) {
			auto charComponent = owner->GetCharacterComponent();
			auto pTrComponent = owner->GetTransformComponent();
			bulletDir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::PRM_FORWARD;
			bulletOrigin = plComponent->GetViewPos();
		}
	}
	if(effectsOrigin == nullptr)
		return;
	if(std::isnan(bulletInfo.effectOrigin.x) == false) {
		*effectsOrigin = bulletInfo.effectOrigin;
		return;
	}
	auto pMdlC = ent.GetModelComponent();
	if(pMdlC && pMdlC->GetAttachment(m_attMuzzle, effectsOrigin, static_cast<Quat *>(nullptr)) == true) {
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->LocalToWorld(effectsOrigin);
	}
}

void CWeaponComponent::ClearOwnerCallbacks()
{
	if(m_cbOnObserverChanged.IsValid())
		m_cbOnObserverChanged.Remove();
	if(m_cbOnOwnerObserverModeChanged.IsValid())
		m_cbOnOwnerObserverModeChanged.Remove();
}

void CWeaponComponent::UpdateOwnerAttachment()
{
	m_hTarget = EntityHandle {};
	UpdateWorldModel();
	auto &ent = GetEntity();
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr) {
		auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
		if(pAttComponent.valid())
			pAttComponent->ClearAttachment();
		return;
	}
	auto *game = get_client_state()->GetGameState();
	CViewModelComponent *cVm = nullptr;
	if(owner->IsPlayer()) {
		auto *plComponent = static_cast<CPlayerComponent *>(owner->GetPlayerComponent().get());
		auto charComponent = owner->GetCharacterComponent();
		if(plComponent->IsLocalPlayer() && charComponent.valid() && charComponent->GetActiveWeapon() == &ent && IsInFirstPersonMode() == true) {
			auto *vm = game->GetViewModel<CViewModelComponent>();
			if(vm == nullptr)
				return;
			cVm = vm;
		}
	}

	CEAttachToOwner evData {*owner, cVm};
	if(BroadcastEvent(cWeaponComponent::EVENT_ATTACH_TO_OWNER, evData) == util::EventReply::Unhandled) {
		auto *parent = cVm ? &cVm->GetEntity() : owner;
		auto pTransformComponent = ent.GetTransformComponent();
		auto pTransformComponentParent = parent->GetTransformComponent();
		if(pTransformComponent && pTransformComponentParent) {
			pTransformComponent->SetPosition(pTransformComponentParent->GetPosition());
			pTransformComponent->SetRotation(pTransformComponentParent->GetRotation());
		}

		auto pAttComponent = ent.AddComponent<CAttachmentComponent>();
		if(pAttComponent.valid()) {
			auto pMdlComponent = parent->GetModelComponent();
			auto attId = pMdlComponent ? pMdlComponent->LookupAttachment("weapon") : -1;
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::SnapToOrigin | FAttachmentMode::UpdateEachFrame;
			if(attId != -1)
				pAttComponent->AttachToAttachment(parent, "weapon", attInfo);
			else
				pAttComponent->AttachToEntity(parent, attInfo);
		}
	}

	auto attC = GetEntity().GetComponent<CAttachmentComponent>();
	if(attC.valid()) {
		auto *parent = attC->GetParent();
		m_hTarget = parent ? parent->GetHandle() : EntityHandle {};
	}
	//SetParent(parent,FPARENT_BONEMERGE | FPARENT_UPDATE_EACH_FRAME);
	//SetAnimated(true);
}

void CWeaponComponent::SetHideWorldModelInFirstPerson(bool b)
{
	math::set_flag(m_stateFlags, StateFlags::HideWorldModelInFirstPerson, b);
	UpdateWorldModel();
}
bool CWeaponComponent::GetHideWorldModelInFirstPerson() const { return math::is_flag_set(m_stateFlags, StateFlags::HideWorldModelInFirstPerson); }

void CWeaponComponent::UpdateDeployState()
{
	if(IsDeployed() == false)
		return;

	if(math::is_flag_set(m_stateFlags, StateFlags::UpdatingDeployState))
		return; // Prevent infinite recursion
	math::set_flag(m_stateFlags, StateFlags::UpdatingDeployState);
	util::ScopeGuard sg {[this]() { math::set_flag(m_stateFlags, StateFlags::UpdatingDeployState, false); }};

	UpdateOwnerAttachment();
	UpdateViewModel();
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	PlayViewActivity(Activity::VmIdle);
}

void CWeaponComponent::Deploy()
{
	BaseWeaponComponent::Deploy();
	UpdateOwnerAttachment();
	UpdateViewModel();
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	if(PlayViewActivity(Activity::VmDeploy) == false)
		PlayViewActivity(Activity::VmIdle);
}

void CWeaponComponent::Holster()
{
	BaseWeaponComponent::Holster();
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	CGame *game = get_client_state()->GetGameState();
	PlayViewActivity(Activity::VmHolster);
}

Activity CWeaponComponent::TranslateViewActivity(Activity act) { return act; }

CViewModelComponent *CWeaponComponent::GetViewModel()
{
	ecs::BaseEntity *parent = m_hTarget.get();
	if(parent == nullptr)
		return nullptr;
	CGame *game = get_client_state()->GetGameState();
	auto *vm = game->GetViewModel<CViewModelComponent>();
	if(vm == nullptr || &vm->GetEntity() != parent)
		return nullptr;
	return vm;
}

bool CWeaponComponent::PlayViewActivity(Activity activity, FPlayAnim flags)
{
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return false;
	activity = TranslateViewActivity(activity);
	auto pAnimComponent = vm->GetEntity().GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->PlayActivity(activity, flags);
	return true;
}

void CWeaponComponent::PrimaryAttack()
{
	if(!CanPrimaryAttack())
		return;
	BaseWeaponComponent::PrimaryAttack();
}
void CWeaponComponent::SecondaryAttack()
{
	if(!CanSecondaryAttack())
		return;
	BaseWeaponComponent::SecondaryAttack();
}
void CWeaponComponent::TertiaryAttack() { BaseWeaponComponent::TertiaryAttack(); }
void CWeaponComponent::Attack4() { BaseWeaponComponent::Attack4(); }
void CWeaponComponent::Reload() { BaseWeaponComponent::Reload(); }

CEAttachToOwner::CEAttachToOwner(ecs::BaseEntity &owner, CViewModelComponent *optViewmodel) : owner {owner}, viewModel {optViewmodel} {}
void CEAttachToOwner::PushArguments(lua::State *l)
{
	owner.GetLuaObject().push(l);
	if(viewModel)
		viewModel->PushLuaObject(l);
}

void CWeaponComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseWeaponComponent::RegisterLuaBindings(l, modEnts);
	auto def = pragma::LuaCore::create_entity_component_class<CWeaponComponent, BaseWeaponComponent>("WeaponComponent");
	def.def("PlayViewActivity", &CWeaponComponent::PlayViewActivity);
	def.def("PlayViewActivity", static_cast<bool (*)(CWeaponComponent &wepComponent, Activity)>([](CWeaponComponent &wepComponent, Activity activity) { return wepComponent.PlayViewActivity(activity); }));
	def.def("SetViewModel", &CWeaponComponent::SetViewModel);
	def.def("GetViewModel", &CWeaponComponent::GetViewModel);
	def.def("SetHideWorldModelInFirstPerson", &CWeaponComponent::SetHideWorldModelInFirstPerson);
	def.def("GetHideWorldModelInFirstPerson", &CWeaponComponent::GetHideWorldModelInFirstPerson);
	def.def("IsInFirstPersonMode", &CWeaponComponent::IsInFirstPersonMode);
	def.def("SetViewModelOffset", &CWeaponComponent::SetViewModelOffset);
	def.def("GetViewModelOffset", &CWeaponComponent::GetViewModelOffset);
	def.def("SetViewFOV", &CWeaponComponent::SetViewFOV);
	def.def("GetViewFOV", &CWeaponComponent::GetViewFOV);
	def.add_static_constant("EVENT_ATTACH_TO_OWNER", cWeaponComponent::EVENT_ATTACH_TO_OWNER);
	modEnts[def];
}
