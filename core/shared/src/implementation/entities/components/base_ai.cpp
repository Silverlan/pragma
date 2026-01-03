// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"

module pragma.shared;

import :entities.components.base_ai;

using namespace pragma;

decltype(BaseAIComponent::s_npcCount) BaseAIComponent::s_npcCount = {0};
decltype(BaseAIComponent::s_navThread) BaseAIComponent::s_navThread = nullptr;

ai::navigation::PathQuery::PathQuery(const Vector3 &_start, const Vector3 &_end) : start(_start), end(_end), complete(false) {}

//////////////////

BaseAIComponent::BaseAIComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_seqIdle(-1), m_navInfo(), m_obstruction()
{
	m_obstruction.nextObstructionCheck = 0.0;
	m_obstruction.pathObstructed = false;
	m_obstruction.resolveDirection = {};

	++s_npcCount;
}

BaseAIComponent::~BaseAIComponent() {}

void BaseAIComponent::OnLookTargetChanged() {}

bool BaseAIComponent::CanMove() const
{
	auto charC = GetEntity().GetCharacterComponent();
	if(charC.expired())
		return false;
	auto *movementC = charC->GetMovementComponent();
	if(!movementC)
		return false;
	return movementC->CanMove();
}
Vector3 BaseAIComponent::GetUpDirection() const
{
	auto upDir = uvec::PRM_UP;
	auto &ent = GetEntity();
	if(ent.IsCharacter()) {
		auto charC = ent.GetCharacterComponent();
		auto *orientC = charC.valid() ? charC->GetOrientationComponent() : nullptr;
		if(orientC)
			upDir = orientC->GetUpDirection();
	}
	return upDir;
}

bool BaseAIComponent::TurnStep(const Vector3 &target, float &turnAngle, const float *turnSpeed)
{
	if(CanMove() == false) {
		turnAngle = 0.f;
		return true;
	}
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return true;
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto dir = target - pTrComponent->GetPosition();
	auto l = uvec::length(dir);
	if(l == 0.f) {
		turnAngle = 0.f;
		return true;
	}
	dir /= l;
	auto charComponent = GetEntity().GetCharacterComponent();
	auto speedMax = static_cast<double>((turnSpeed != nullptr) ? *turnSpeed : (charComponent.valid() ? charComponent->GetTurnSpeed() : 100.f)) * game->DeltaTickTime();
	Vector2 rotAm = {};
	const Vector2 pitchLimit {0.f, 0.f};
	auto newRot = math::approach_direction(pTrComponent->GetRotation(), GetUpDirection(), dir, Vector2(speedMax, speedMax), &rotAm, &pitchLimit);
	pTrComponent->SetRotation(newRot);
	return (math::abs(rotAm.y) <= speedMax) ? true : false;

	// Deprecated if the above code works properly
	/*
	auto *nw = m_entity->GetNetworkState();
	auto *game = nw->GetGameState();
	auto dir = target -m_entity->GetPosition();
	auto rotInv = m_entity->GetOrientation();
	uquat::inverse(rotInv);
	auto rotTgt = uquat::create(uvec::to_angle(dir)) *rotInv;
	auto angTgt = EulerAngles(rotTgt);
	auto speedMax = static_cast<double>((turnSpeed != nullptr) ? *turnSpeed : GetTurnSpeed()) *game->DeltaTickTime();
	auto y = pragma::math::approach_angle(0.f,angTgt.y,speedMax);
	auto diff = pragma::math::get_angle_difference(angTgt.y,y);
	angTgt.y = y;
	m_entity->SetOrientation();
	turnAngle = diff;
	return (fabs(diff) <= speedMax) ? true : false;*/
	/*auto angCur = m_entity->GetAngles();
	auto angTgt = uvec::to_angle(dir);
	angTgt.p = 0;
	angTgt.r = 0;
	auto speedMax = static_cast<double>((turnSpeed != nullptr) ? *turnSpeed : GetTurnSpeed()) *game->DeltaTickTime();
	auto y = pragma::math::approach_angle(angCur.y,angTgt.y,speedMax);
	auto diff = pragma::math::get_angle_difference(angTgt.y,y);
	angTgt.y = y;
	m_entity->SetAngles(angTgt);
	turnAngle = diff;
	return (fabs(diff) <= speedMax) ? true : false;*/
}

bool BaseAIComponent::TurnStep(const Vector3 &target, const float *turnSpeed)
{
	auto turnAngle = 0.f;
	return TurnStep(target, turnAngle, turnSpeed);
}

void BaseAIComponent::ReleaseNavThread()
{
	if(s_navThread == nullptr)
		return;
	s_navThread->running = false;
	if(s_navThread->thread.joinable())
		s_navThread->thread.join();
	if(s_navThread->releaseCallback.IsValid())
		s_navThread->releaseCallback.Remove();
	s_navThread = nullptr;
}

void BaseAIComponent::ReloadNavThread(Game &game)
{
	ReleaseNavThread();

	auto wpNavMesh = std::weak_ptr<nav::Mesh>(game.GetNavMesh());
	if(wpNavMesh.expired() == true)
		return;
	s_navThread = pragma::util::make_shared<ai::navigation::NavThread>();
	auto cb = FunctionCallback<void>::Create(nullptr);
	cb.get<Callback<void>>()->SetFunction([cb]() mutable {
		if(cb.IsValid())
			cb.Remove();
	});
	game.AddCallback("EndGame", cb);
	s_navThread->releaseCallback = cb;

	s_navThread->thread = std::thread([wpNavMesh]() {
		while(s_navThread->running == true) {
			s_navThread->pendingQueueMutex.lock();
			while(s_navThread->pendingQueue.empty() == false) {
				auto query = s_navThread->pendingQueue.front();
				s_navThread->pendingQueue.pop();
				s_navThread->queryQueue.push(query);
			}
			s_navThread->pendingQueueMutex.unlock();

			auto bEmpty = s_navThread->queryQueue.empty();
			if(bEmpty == false) {
				auto item = s_navThread->queryQueue.front();
				auto navMesh = wpNavMesh.lock();
				std::shared_ptr<RcPathResult> path = nullptr;
				if(navMesh != nullptr && (path = navMesh->FindPath(item->start, item->end)) != nullptr)
					item->pathInfo = pragma::util::make_shared<ai::navigation::PathInfo>(path);
				s_navThread->queryQueue.pop();
				item->complete = true;

				bEmpty = s_navThread->queryQueue.empty();
			}
			if(bEmpty == true)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
}

void BaseAIComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetLookTarget = SetupNetEvent("set_look_target");

	BindEventUnhandled(basePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED, [this](std::reference_wrapper<ComponentEvent> eventData) {
		PathStep(static_cast<float>(static_cast<CEPhysicsUpdateData &>(eventData.get()).deltaTime));
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(movementComponent::EVENT_ON_UPDATE_MOVEMENT, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateMovementProperties(); });
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnModelChanged(static_cast<CEOnModelChanged &>(evData.get()).model); });
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_ANIMATION_START, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto animComponent = GetEntity().GetAnimatedComponent();
		if(animComponent.expired())
			return;
		auto *anim = animComponent->GetAnimationObject();
		if(anim == nullptr) {
			m_animMoveInfo.moving = false;
			m_animMoveInfo.blend = false;
			return;
		}
		m_animMoveInfo.blend = !anim->HasFlag(FAnim::NoMoveBlend);
		m_animMoveInfo.moving = (anim->HasFlag(FAnim::MoveX) || anim->HasFlag(FAnim::MoveZ)) ? true : false;
	});
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_BLEND_ANIMATION_MT, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto animComponent = GetEntity().GetAnimatedComponent();
		if(animComponent.expired())
			return;
		auto &evDataBlend = static_cast<CEOnBlendAnimation &>(evData.get());
		auto &animInfo = evDataBlend.slotInfo;
		if(&animInfo == &animComponent->GetBaseAnimationInfo()) // Only apply for base animation, not for gestures
			BlendAnimationMovementMT(evDataBlend.bonePoses, evDataBlend.boneScales);
	});
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEvent(baseCharacterComponent::EVENT_IS_MOVING, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEIsMoving &>(evData.get()).moving = IsMoving();
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	auto *pCharComponent = static_cast<BaseCharacterComponent *>(ent.AddComponent("character").get());
	if(pCharComponent != nullptr)
		pCharComponent->SetTurnSpeed(160.f);

	SetTickPolicy(TickPolicy::Always);
}

void BaseAIComponent::UpdateMovementProperties(MovementComponent &movementC)
{
	movementC.SetSpeed(CalcMovementSpeed());
	movementC.SetAcceleration(CalcMovementAcceleration());
	movementC.SetAirModifier(CalcAirMovementModifier());
	movementC.SetDirection(CalcMovementDirection());
}

void BaseAIComponent::UpdateMovementProperties()
{
	auto charC = GetEntity().GetCharacterComponent();
	auto *movementC = charC.valid() ? charC->GetMovementComponent() : nullptr;
	if(!movementC)
		return;
	UpdateMovementProperties(*movementC);
}

void BaseAIComponent::OnEntitySpawn()
{
	auto &ent = GetEntity();
	auto *game = ent.GetNetworkState()->GetGameState();
	auto *world = game->GetWorld();
	std::vector<EntityHandle> filterEnts {};
	filterEnts.push_back(ent.GetHandle());
	if(world != nullptr)
		filterEnts.push_back(world->GetEntity().GetHandle()); // The navigation mesh should already ensure there's no collision with the world, so we'll exclude it here
	m_obstruction.sweepFilter = std::make_unique<physics::MultiEntityRayCastFilterCallback>(std::move(filterEnts));
}

void BaseAIComponent::OnModelChanged(const std::shared_ptr<asset::Model> &model)
{
	m_seqIdle = -1;
	auto *pObservableComponent = static_cast<BaseObservableComponent *>(GetEntity().FindComponent("observable").get());
	if(pObservableComponent != nullptr)
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::ThirdPerson, false);
	if(model == nullptr)
		return;
	if(pObservableComponent != nullptr) {
		Vector3 min, max;
		model->GetRenderBounds(min, max);
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::ThirdPerson, true);
		pObservableComponent->SetLocalCameraOrigin(BaseObservableComponent::CameraType::ThirdPerson, Vector3 {0.f, (max.y - min.y) * 0.25f, -math::max(math::abs(min.x), math::abs(min.y), math::abs(min.z), math::abs(max.x), math::abs(max.y), math::abs(max.z))});
	}

	// Update animation move speed
	for(auto &pair : m_animMoveSpeed) {
		auto animId = model->LookupAnimation(pair.first);
		if(animId == -1)
			continue;
		m_animIdMoveSpeed[animId] = pair.second;
	}

	// Find idle animation
	std::vector<unsigned int> animations;
	model->GetAnimations(Activity::Idle, animations);
	if(animations.empty())
		return;
	m_seqIdle = animations.front();
	auto it = std::find_if(animations.begin(), animations.end(), [&model](uint32_t animId) {
		auto anim = model->GetAnimation(animId);
		return (anim != nullptr && !anim->HasFlag(FAnim::NoRepeat)) ? true : false; // Prefer an idle animation that can be repeated (More likely to be a generic idle animation)
	});
	if(it != animations.end())
		m_seqIdle = *it;
}

void BaseAIComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->AddCollisionFilter(physics::CollisionMask::NPC);
}

void BaseAIComponent::Spawn() {}

void BaseAIComponent::OnTick(double tDelta)
{
	UpdatePath();
	LookAtStep(tDelta);
}

const char *BaseAIComponent::MoveResultToString(MoveResult result)
{
	switch(result) {
	case MoveResult::TargetUnreachable:
		return "Target unreachable";
	case MoveResult::TargetReached:
		return "Target reached";
	case MoveResult::WaitingForPath:
		return "Waiting for path";
	case MoveResult::MovingToTarget:
		return "Moving to target";
	default:
		return "Unknown";
	}
}
