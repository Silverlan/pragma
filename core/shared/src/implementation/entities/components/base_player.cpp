// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.base_player;

using namespace pragma;

void BasePlayerComponent::SetStandHeight(float height) { m_standHeight = height; }
void BasePlayerComponent::SetCrouchHeight(float height) { m_crouchHeight = height; }
void BasePlayerComponent::OnTakenDamage(game::DamageInfo &info, unsigned short oldHealth, unsigned short newHealth)
{
	if(oldHealth == 0 || newHealth != 0)
		return;
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->Kill(&info);
}
void BasePlayerComponent::OnKilled(game::DamageInfo *dmgInfo)
{
	auto observerC = GetEntity().FindComponent("observer");
	if(observerC.valid())
		static_cast<BaseObserverComponent *>(observerC.get())->SetObserverMode(ObserverMode::ThirdPerson);
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();

	game->CallCallbacks<void, BasePlayerComponent *, game::DamageInfo *>("OnPlayerDeath", this, dmgInfo);
	for(auto *gmC : game->GetGamemodeComponents())
		gmC->OnPlayerDeath(*this, dmgInfo);

	auto charComponent = ent.GetCharacterComponent();
	charComponent->RemoveWeapons();
}
void BasePlayerComponent::SetViewRotation(const Quat &rot)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->SetViewOrientation(rot);
}
util::EventReply BasePlayerComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseActorComponent::EVENT_ON_KILLED)
		OnKilled(static_cast<const CEOnCharacterKilled &>(evData).damageInfo);
	else if(eventId == baseActorComponent::EVENT_ON_RESPAWN)
		OnRespawn();
	else if(eventId == baseHealthComponent::EVENT_ON_TAKEN_DAMAGE) {
		auto &healthInfo = static_cast<CEOnTakenDamage &>(evData);
		OnTakenDamage(healthInfo.damageInfo, healthInfo.oldHealth, healthInfo.newHealth);
	}
	return util::EventReply::Unhandled;
}
bool BasePlayerComponent::CanUnCrouch() const
{
	if(m_shapeStand == nullptr)
		return true;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr)
		return true;
	auto *col = phys->GetCollisionObject();
	if(col == nullptr)
		return false;
	if(ent.IsCharacter() == false)
		return false;
	auto pTrComponent = ent.GetTransformComponent();
	auto &charComponent = *ent.GetCharacterComponent();
	auto colPos = col->GetPos();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto data = charComponent.GetAimTraceData();
	data.SetSource(colPos);
	data.SetTarget(colPos + (pTrComponent ? pTrComponent->GetUp() : uvec::PRM_UP) * 0.001f); // Target position musn't be the same as the source position, otherwise the trace will never detect a hit
	data.SetShape(*m_shapeStand);
	return game->Sweep(data).hitType == physics::RayCastHitType::None; // Overlap only works with collision objects, not with individual shapes, so we have to use Sweep instead
}
ActionInputControllerComponent *BasePlayerComponent::GetActionInputController() { return m_actionController; }
void BasePlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ActionInputControllerComponent))
		m_actionController = &static_cast<ActionInputControllerComponent &>(component);
}
void BasePlayerComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(ActionInputControllerComponent))
		m_actionController = nullptr;
}
void BasePlayerComponent::OnTick(double tDelta)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(m_bCrouching || m_crouchTransition != CrouchTransition::None) {
		if((!m_actionController || m_actionController->GetActionInput(Action::Crouch) == false) && m_crouchTransition != CrouchTransition::Uncrouching)
			UnCrouch();
		else if(m_crouchTransition != CrouchTransition::None) {
			auto *state = ent.GetNetworkState();
			Game *game = state->GetGameState();
			if(game->CurTime() >= m_tCrouch) {
				m_tCrouch = 0.f;
				if(m_crouchTransition == CrouchTransition::Crouching) {
					m_bCrouching = true;
					if(phys != nullptr && phys->IsController()) {
						auto *controller = static_cast<physics::ControllerPhysObj *>(phys);
						if(controller->IsCapsule()) {
							auto *capsule = static_cast<physics::CapsuleControllerPhysObj *>(controller);
							capsule->SetHeight(m_crouchHeight);
						}
						OnFullyCrouched();
					}
				}
				else {
					m_bCrouching = false;
					if(phys != nullptr && phys->IsController()) {
						auto *controller = static_cast<physics::ControllerPhysObj *>(phys);
						if(controller->IsCapsule()) {
							auto *capsule = static_cast<physics::CapsuleControllerPhysObj *>(controller);
							capsule->SetHeight(m_standHeight);
						}
						OnFullyUnCrouched();
					}
				}
				m_crouchTransition = CrouchTransition::None;
			}
		}
	}

	// Update animation
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.valid() && (phys == nullptr || phys->IsController())) // Only run this if not in ragdoll mode
	{
		auto pTrComponent = ent.GetTransformComponent();
		auto pVelComponent = ent.GetComponent<VelocityComponent>();
		auto &vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
		auto speed = uvec::length(vel);
		auto charComponent = ent.GetCharacterComponent();
		const auto movementSpeedThreshold = 0.04f;
		if(speed >= movementSpeedThreshold) {
			if((m_bForceAnimationUpdate == true || animComponent->GetActivity() == animComponent->TranslateActivity(Activity::Idle)) && PlaySharedActivity(IsWalking() ? Activity::Walk : Activity::Run) == true)
				m_movementActivity = animComponent->GetActivity();
			m_bForceAnimationUpdate = false;

			if(pTrComponent) {
				auto rot = pTrComponent->GetRotation();
				auto rotView = charComponent.valid() ? charComponent->GetViewOrientation() : rot;
				auto rotRef = charComponent.valid() ? charComponent->GetOrientationAxesRotation() : rot;
				rotView = rotRef * rotView;
				rot = rotRef * rot;
				auto ang = EulerAngles(rot);
				ang.y = EulerAngles(rotView).y;
				rot = uquat::get_inverse(rotRef) * uquat::create(ang);
				pTrComponent->SetRotation(rot);
			}
		}
		else {
			auto bMoving = IsMoving();
			if(m_bForceAnimationUpdate == true || bMoving == true) {
				m_bForceAnimationUpdate = false;
				PlaySharedActivity(Activity::Idle);
				if(bMoving == true) {
					auto moveBlendScale = 0.f;
					if(charComponent.valid()) {
						auto *movementC = charComponent->GetMovementComponent();
						if(movementC)
							moveBlendScale = movementC->GetMovementBlendScale();
					}
					animComponent->SetLastAnimationBlendScale(1.f - moveBlendScale);
				}
			}
		}
	}
}

bool BasePlayerComponent::IsMoving() const
{
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	return (animComponent.valid() && animComponent->GetActivity() == m_movementActivity && m_movementActivity != Activity::Invalid) ? true : false;
}
bool BasePlayerComponent::IsWalking() const { return m_actionController && m_actionController->GetActionInput(Action::Walk); }
bool BasePlayerComponent::IsSprinting() const { return (!IsWalking() && m_actionController && m_actionController->GetActionInput(Action::Sprint)) ? true : false; }

Con::c_cout &BasePlayerComponent::print(Con::c_cout &os)
{
	auto &ent = GetEntity();
	auto nameComponent = ent.GetNameComponent();
	os << "Player[" << (nameComponent.valid() ? nameComponent->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		os << "NULL";
	else
		os << hMdl->GetName();
	os << "]";
	return os;
}

std::ostream &BasePlayerComponent::print(std::ostream &os)
{
	auto &ent = GetEntity();
	auto nameComponent = ent.GetNameComponent();
	os << "Player[" << (nameComponent.valid() ? nameComponent->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		os << "NULL";
	else
		os << hMdl->GetName();
	os << "]";
	return os;
}

BasePlayerComponent::BasePlayerComponent(ecs::BaseEntity &ent)
    : BaseEntityComponent(ent), m_portUDP(0), m_speedWalk(63.33f), m_speedRun(190), m_speedSprint(320), m_speedCrouchWalk(63.33f), m_bCrouching(false), m_standHeight(72), m_crouchHeight(36), m_standEyeLevel(64), m_crouchEyeLevel(28), m_tCrouch(0), m_bFlashlightOn(false)
{
	m_bLocalPlayer = false;
	m_timeConnected = 0;
}

BasePlayerComponent::~BasePlayerComponent()
{
	//if(m_sprite != nullptr)
	//	ServerState::Get()->RemoveSprite(m_sprite); // WEAVETODO
	if(m_entFlashlight.valid())
		m_entFlashlight->Remove();
}

void BasePlayerComponent::OnRespawn()
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	game->CallCallbacks<void, BasePlayerComponent *>("OnPlayerSpawned", this);
	for(auto *gmC : game->GetGamemodeComponents())
		gmC->OnPlayerSpawned(*this);
}

BasePlayer *BasePlayerComponent::GetBasePlayer() const { return dynamic_cast<BasePlayer *>(m_hBasePlayer.get()); }

void BasePlayerComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvApplyViewRotationOffset = SetupNetEvent("apply_view_rotation_offset");
	m_netEvPrintMessage = SetupNetEvent("print_message");
	m_netEvRespawn = SetupNetEvent("respawn");
	m_netEvSetViewOrientation = SetupNetEvent("set_view_orientation");

	auto &ent = GetEntity();
	ent.AddComponent("character");
	ent.AddComponent("name");
	ent.AddComponent("score");
	ent.AddComponent("observable");
	auto actionInputControllerC = ent.AddComponent<ActionInputControllerComponent>();
	auto movementControllerC = ent.AddComponent<InputMovementControllerComponent>();
	if(movementControllerC.valid())
		movementControllerC->SetActionInputController(actionInputControllerC.get());
	m_hBasePlayer = ent.GetHandle();

	BindEventUnhandled(movementComponent::EVENT_ON_UPDATE_MOVEMENT, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateMovementProperties(); });
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &hMdl = GetEntity().GetModel();
		if(hMdl == nullptr)
			return util::EventReply::Unhandled;
		auto anim = hMdl->GetAnimation(static_cast<CEOnAnimationComplete &>(evData.get()).animation);
		if(anim == nullptr)
			return util::EventReply::Unhandled;
		if(anim->HasFlag(FAnim::Loop) == false)
			PlaySharedActivity(Activity::Idle); // A non-looping animation has completed; Switch back to idle
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_ANIMATION_RESET, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		PlaySharedActivity(Activity::Idle); // A non-looping animation has completed; Switch back to idle
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_ANIMATION_START, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		m_movementActivity = Activity::Invalid;
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(baseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		if((IsCrouching() == true && m_crouchTransition != CrouchTransition::Uncrouching) || m_crouchTransition == CrouchTransition::Crouching) {
			auto &activity = static_cast<CETranslateActivity &>(evData.get()).activity;
			switch(activity) {
			case Activity::Idle:
				activity = Activity::CrouchIdle;
				return util::EventReply::Handled;
			case Activity::Walk:
			case Activity::Run:
				activity = Activity::CrouchWalk;
				return util::EventReply::Handled;
			}
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		OnPhysicsInitialized();
		return util::EventReply::Unhandled;
	});
	BindEvent(baseCharacterComponent::EVENT_IS_MOVING, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEIsMoving &>(evData.get()).moving = IsMoving();
		return util::EventReply::Handled;
	});
	BindEventUnhandled(baseCharacterComponent::EVENT_ON_JUMP, [this](std::reference_wrapper<ComponentEvent> evData) { PlaySharedActivity(Activity::Jump); });
	BindEventUnhandled(actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &evAction = static_cast<CEOnActionInputChanged &>(evData.get());
		HandleActionInput(evAction.action, evAction.pressed);
		return util::EventReply::Unhandled;
	});

	auto whObservableComponent = ent.FindComponent("observable");
	if(whObservableComponent.valid()) {
		auto *pObservableComponent = static_cast<BaseObservableComponent *>(whObservableComponent.get());
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::FirstPerson, true);
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::ThirdPerson, true);
		pObservableComponent->SetLocalCameraOffset(BaseObservableComponent::CameraType::ThirdPerson, {0.f, 10.f, -80.f});
	}

	SetTickPolicy(TickPolicy::Always);
}

void BasePlayerComponent::UpdateMovementProperties()
{
	auto charC = GetEntity().GetCharacterComponent();
	auto *movementC = charC.valid() ? charC->GetMovementComponent() : nullptr;
	if(!movementC)
		return;
	movementC->SetSpeed(CalcMovementSpeed());
	float rampUpTime;
	auto acc = CalcMovementAcceleration(rampUpTime);
	movementC->SetAcceleration(acc);
	movementC->SetAccelerationRampUpTime(rampUpTime);
	movementC->SetAirModifier(CalcAirMovementModifier());
}

void BasePlayerComponent::HandleActionInput(Action action, bool pressed)
{
	if(!pressed)
		return;
	auto charComponent = GetEntity().GetCharacterComponent();
	switch(action) {
	case Action::Jump:
		{
			if(charComponent.valid())
				charComponent->Jump();
			break;
		}
	case Action::Crouch:
		{
			Crouch();
			break;
		}
	case Action::Attack:
		{
			if(charComponent.valid())
				charComponent->PrimaryAttack();
			break;
		}
	case Action::Attack2:
		{
			if(charComponent.valid())
				charComponent->SecondaryAttack();
			break;
		}
	case Action::Attack3:
		{
			if(charComponent.valid())
				charComponent->TertiaryAttack();
			break;
		}
	case Action::Attack4:
		{
			if(charComponent.valid())
				charComponent->Attack4();
			break;
		}
	case Action::Reload:
		{
			if(charComponent.valid())
				charComponent->ReloadWeapon();
			break;
		}
	case Action::Use:
		{
			Use();
			break;
		}
	}
}

void BasePlayerComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->AddCollisionFilter(physics::CollisionMask::Player);
}

ecs::BaseEntity *BasePlayerComponent::FindUseEntity() const
{
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	Vector3 origin = charComponent.valid() ? charComponent->GetEyePosition() : (pTrComponent ? pTrComponent->GetPosition() : Vector3 {});
	Vector3 viewDir = charComponent.valid() ? charComponent->GetViewForward() : (pTrComponent ? pTrComponent->GetForward() : Vector3 {});
	const auto dotMin = 0.6f;
	float dotClosest = 1.f;
	float maxDist = 96.f;
	float distClosest = std::numeric_limits<float>::max();
	ecs::BaseEntity *entClosest = nullptr;
	ecs::EntityIterator it {*game};
	it.AttachFilter<TEntityIteratorFilterComponent<UsableComponent>>();
	for(auto *entOther : it) {
		auto pUsableComponent = entOther->GetComponent<UsableComponent>();
		if(pUsableComponent->CanUse(const_cast<ecs::BaseEntity *>(&ent))) {
			auto pTrComponentOther = entOther->GetTransformComponent();
			if(!pTrComponentOther)
				continue;
			auto pPhysComponentOther = entOther->GetPhysicsComponent();
			auto &posEnt = pTrComponentOther->GetPosition();
			Vector3 min {};
			Vector3 max {};
			if(pPhysComponentOther)
				pPhysComponentOther->GetCollisionBounds(&min, &max);
			min += posEnt;
			max += posEnt;
			Vector3 res;
			math::geometry::closest_point_on_aabb_to_point(min, max, origin, &res);

			float dist = glm::distance(origin, res);
			if(dist <= maxDist) {
				Vector3 dir = res - origin;
				uvec::normalize(&dir);
				auto dot = uvec::dot(viewDir, dir);
				if(dot >= dotMin && ((dot - dotClosest) >= 0.2f || (distClosest - dist) >= 20.f)) {
					physics::TraceData data;
					data.SetSource(origin);
					data.SetTarget(origin + dir * dist);
					data.SetFilter(*entOther);
					auto result = game->RayCast(data);
					if(result.hitType == physics::RayCastHitType::None || result.entity.get() == entOther) {
						dotClosest = dot;
						entClosest = entOther;
					}
				}
			}
		}
	}
	return entClosest;
}

void BasePlayerComponent::Use()
{
	ecs::BaseEntity *ent = FindUseEntity();
	if(ent == nullptr)
		return;
	auto pUsableComponent = ent->GetComponent<UsableComponent>();
	if(pUsableComponent.valid())
		pUsableComponent->OnUse(&GetEntity());
}

void BasePlayerComponent::SetFlashlight(bool b)
{
	if(m_entFlashlight.valid() == false)
		return;
	auto *toggleComponent = static_cast<BaseToggleComponent *>(m_entFlashlight.get()->FindComponent("toggle").get());
	if(toggleComponent != nullptr)
		toggleComponent->SetTurnedOn(b);
}
void BasePlayerComponent::ToggleFlashlight()
{
	if(IsFlashlightOn())
		SetFlashlight(false);
	else
		SetFlashlight(true);
}
bool BasePlayerComponent::IsFlashlightOn() const
{
	if(m_entFlashlight.valid() == false)
		return false;
	auto *toggleComponent = static_cast<BaseToggleComponent *>(m_entFlashlight.get()->FindComponent("toggle").get());
	return (toggleComponent != nullptr) ? toggleComponent->IsTurnedOn() : false;
}

Vector2 BasePlayerComponent::CalcMovementSpeed() const
{
	float speed;
	auto physComponent = GetEntity().GetPhysicsComponent();
	if(physComponent && physComponent->GetMoveType() == physics::MoveType::Noclip) {
		speed = GetEntity().GetNetworkState()->GetGameState()->GetConVarFloat("sv_noclip_speed");
		if(IsWalking())
			speed *= 0.5f;
		else if(IsSprinting())
			speed *= 2.f;
	}
	else if(IsCrouching())
		speed = GetCrouchedWalkSpeed();
	else if(IsWalking())
		speed = GetWalkSpeed();
	else if(IsSprinting())
		speed = GetSprintSpeed();
	else
		speed = GetRunSpeed();
	return {speed, 0.f};
}
float BasePlayerComponent::CalcAirMovementModifier() const { return GetEntity().GetNetworkState()->GetGameState()->GetConVarFloat("sv_player_air_move_scale"); }
float BasePlayerComponent::CalcMovementAcceleration(float &optOutRampUpTime) const
{
	auto *game = GetEntity().GetNetworkState()->GetGameState();
	optOutRampUpTime = game->GetConVarFloat("sv_acceleration_ramp_up_time");
	return game->GetConVarFloat("sv_acceleration");
}

void BasePlayerComponent::SetUDPPort(unsigned short port) { m_portUDP = port; }

void BasePlayerComponent::SetLocalPlayer(bool b) { m_bLocalPlayer = b; }

unsigned short BasePlayerComponent::GetUDPPort() const { return m_portUDP; }

std::string BasePlayerComponent::GetClientIP() { return "[::]:0"; }
unsigned short BasePlayerComponent::GetClientPort() { return 0; }

void BasePlayerComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	m_timeConnected = state->RealTime();

	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetCollisionBounds(Vector3(-16, 0, -16), Vector3(16, m_standHeight, 16));
	PlaySharedActivity(Activity::Idle);
}

void BasePlayerComponent::GetConVars(std::unordered_map<std::string, std::string> **convars) { *convars = &m_conVars; }

bool BasePlayerComponent::GetConVar(std::string cvar, std::string *val)
{
	std::unordered_map<std::string, std::string>::iterator i = m_conVars.find(cvar);
	if(i == m_conVars.end())
		return false;
	*val = i->second;
	return true;
}

std::string BasePlayerComponent::GetConVarString(std::string cvar) const
{
	std::unordered_map<std::string, std::string>::iterator i = const_cast<BasePlayerComponent *>(this)->m_conVars.find(cvar);
	if(i == m_conVars.end())
		return "";
	return i->second;
}

int BasePlayerComponent::GetConVarInt(std::string cvar) const
{
	std::unordered_map<std::string, std::string>::iterator i = const_cast<BasePlayerComponent *>(this)->m_conVars.find(cvar);
	if(i == m_conVars.end())
		return 0;
	return string::to_int(i->second);
}

float BasePlayerComponent::GetConVarFloat(std::string cvar) const
{
	std::unordered_map<std::string, std::string>::iterator i = const_cast<BasePlayerComponent *>(this)->m_conVars.find(cvar);
	if(i == m_conVars.end())
		return 0;
	return util::to_float(i->second);
}

bool BasePlayerComponent::GetConVarBool(std::string cvar) const
{
	std::unordered_map<std::string, std::string>::iterator i = const_cast<BasePlayerComponent *>(this)->m_conVars.find(cvar);
	if(i == m_conVars.end())
		return false;
	return i->second != "0";
}

double BasePlayerComponent::TimeConnected() const { return GetEntity().GetNetworkState()->RealTime() - m_timeConnected; }

double BasePlayerComponent::ConnectionTime() const { return m_timeConnected; }

void BasePlayerComponent::SetViewPos(const std::optional<Vector3> &pos) { m_viewPos = pos; }
Vector3 BasePlayerComponent::GetViewPos() const
{
	if(m_viewPos)
		return *m_viewPos;
	if(!m_observableComponent)
		return GetEntity().GetPosition();
	auto viewOffset = m_observableComponent->GetViewOffset();
	auto charComponent = GetEntity().GetCharacterComponent();
	auto upDir = uvec::PRM_UP;
	if(charComponent.valid()) {
		auto *orientC = charComponent->GetOrientationComponent();
		if(orientC)
			upDir = orientC->GetUpDirection();
	}
	viewOffset = Vector3(viewOffset.x, 0, viewOffset.z) + upDir * viewOffset.y;
	return GetEntity().GetPosition() + viewOffset;
}

bool BasePlayerComponent::PlaySharedActivity(Activity activity)
{
	auto animComponent = GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->PlayActivity(activity) : false;
}

float BasePlayerComponent::GetStandHeight() const { return m_standHeight; }
float BasePlayerComponent::GetCrouchHeight() const { return m_crouchHeight; }
float BasePlayerComponent::GetStandEyeLevel() const { return m_standEyeLevel; }
float BasePlayerComponent::GetCrouchEyeLevel() const { return m_crouchEyeLevel; }
void BasePlayerComponent::SetStandEyeLevel(float eyelevel) { m_standEyeLevel = eyelevel; }
void BasePlayerComponent::SetCrouchEyeLevel(float eyelevel) { m_crouchEyeLevel = eyelevel; }

void BasePlayerComponent::OnCrouch() {}
void BasePlayerComponent::OnUnCrouch() {}
void BasePlayerComponent::OnFullyCrouched() {}
void BasePlayerComponent::OnFullyUnCrouched() {}
void BasePlayerComponent::Crouch()
{
	if((m_bCrouching && m_crouchTransition != CrouchTransition::Uncrouching) || m_crouchTransition == CrouchTransition::Crouching)
		return;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	m_shapeStand = nullptr;
	auto *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	if(phys != nullptr && phys->IsController()) {
		auto *controllerPhys = static_cast<physics::ControllerPhysObj *>(phys);
		assert(controllerPhys->IsCapsule());
		if(!controllerPhys->IsCapsule())
			spdlog::warn("Box-controller crouching not implemented!");
		auto *controller = controllerPhys->GetController();
		auto shape = controller->GetShape();
		if(shape != nullptr && controllerPhys->IsCapsule()) {
			auto *capsuleShape = dynamic_cast<physics::ICapsuleShape *>(shape);
			if(capsuleShape) {
				auto radius = capsuleShape->GetRadius();
				auto halfHeight = capsuleShape->GetHalfHeight();
				auto *physEnv = game->GetPhysicsEnvironment();
				m_shapeStand = physEnv->CreateCapsuleShape(radius, halfHeight, physEnv->GetGenericMaterial()); // TODO: Cache this shape
			}
		}
	}

	m_crouchTransition = CrouchTransition::Crouching;
	m_bForceAnimationUpdate = true;
	m_tCrouch = CFloat(game->CurTime()) + 0.2f;
	OnCrouch();
}

void BasePlayerComponent::UnCrouch(bool bForce)
{
	if((!m_bCrouching && m_crouchTransition != CrouchTransition::Crouching) || m_crouchTransition == CrouchTransition::Uncrouching)
		return;
	if(!bForce && !CanUnCrouch())
		return;
	m_shapeStand = nullptr;
	m_crouchTransition = CrouchTransition::Uncrouching;
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	m_bForceAnimationUpdate = true;
	m_tCrouch = CFloat(game->CurTime()) + 0.4f;
	OnUnCrouch();
}

bool BasePlayerComponent::IsCrouching() const { return m_bCrouching; }

bool BasePlayerComponent::IsLocalPlayer() const { return m_bLocalPlayer; }

bool BasePlayerComponent::IsKeyDown(int key) { return m_keysPressed[key]; }

float BasePlayerComponent::GetWalkSpeed() const
{
	auto r = m_speedWalk;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto &scale = pTrComponent->GetScale();
		r *= math::abs_max(scale.x, scale.y, scale.z);
	}
	return r;
}
float BasePlayerComponent::GetRunSpeed() const
{
	auto r = m_speedRun;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto &scale = pTrComponent->GetScale();
		r *= math::abs_max(scale.x, scale.y, scale.z);
	}
	return r;
}
float BasePlayerComponent::GetSprintSpeed() const
{
	auto r = m_speedSprint;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto &scale = pTrComponent->GetScale();
		r *= math::abs_max(scale.x, scale.y, scale.z);
	}
	return r;
}
void BasePlayerComponent::SetWalkSpeed(float speed) { m_speedWalk = speed; }
void BasePlayerComponent::SetRunSpeed(float speed) { m_speedRun = speed; }
void BasePlayerComponent::SetSprintSpeed(float speed) { m_speedSprint = speed; }
float BasePlayerComponent::GetCrouchedWalkSpeed() const
{
	auto r = m_speedCrouchWalk;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto &scale = pTrComponent->GetScale();
		r *= math::abs_max(scale.x, scale.y, scale.z);
	}
	return r;
}
void BasePlayerComponent::SetCrouchedWalkSpeed(float speed) { m_speedCrouchWalk = speed; }

void BasePlayerComponent::PrintMessage(std::string message, console::MESSAGE) {}
