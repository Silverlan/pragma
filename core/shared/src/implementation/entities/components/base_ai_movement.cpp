// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

#define AI_OBSTRUCTION_CHECK_RAYCAST_TYPE_RAY 0
#define AI_OBSTRUCTION_CHECK_RAYCAST_TYPE_SWEEP 1
#define AI_OBSTRUCTION_CHECK_RAYCAST_TYPE AI_OBSTRUCTION_CHECK_RAYCAST_TYPE_SWEEP

import :entities.components.base_ai;

using namespace pragma;

ai::navigation::PathInfo::PathInfo(const std::shared_ptr<RcPathResult> &path) : path(path), pathIdx(0) {}

//////////////////

BaseAIComponent::MoveInfo::MoveInfo(Activity act) : MoveInfo() { activity = act; }
BaseAIComponent::MoveInfo::MoveInfo(Activity act, bool bMoveOnPath) : MoveInfo()
{
	activity = act;
	moveOnPath = bMoveOnPath;
}
BaseAIComponent::MoveInfo::MoveInfo(Activity act, bool bMoveOnPath, const Vector3 &faceTarget, float moveSpeed, float turnSpeed) : MoveInfo()
{
	activity = act;
	moveOnPath = bMoveOnPath;
	this->faceTarget = faceTarget;
	this->moveSpeed = moveSpeed;
	this->turnSpeed = turnSpeed;
}

//////////////////

void BaseAIComponent::UpdatePath()
{
	if(CanMove() == false || m_moveInfo.moveOnPath == false)
		return;
	if(m_navInfo.queuedPath != nullptr) {
		if(s_navThread == nullptr)
			m_navInfo.queuedPath = nullptr;
		else {
			auto bPathChanged = false;
			if(m_navInfo.queuedPath->complete == true) {
				if(m_navInfo.queuedPath->pathInfo != nullptr) {
					m_navInfo.pathInfo = m_navInfo.queuedPath->pathInfo;
					m_navInfo.pathState = PathResult::Success;
				}
				else {
					//Con::CERR<<"Failed to generate path!"<<Con::endl;
					m_navInfo.pathState = PathResult::Failed;
				}
				m_navInfo.queuedPath = nullptr;
				bPathChanged = true;
			}
			if(bPathChanged == true)
				OnPathChanged();
		}
	}
	auto bPathUpdateRequired = (m_navInfo.pathInfo == nullptr || m_navInfo.bPathUpdateRequired == true) ? true : false;
	if(bPathUpdateRequired == false) {
		//auto &pos = m_entity->GetPosition();
		/*Vector3 startPos {};
		if(m_pathInfo->path->GetNode(0,pos,startPos) == true)
		{
			auto d = uvec::distance(pos,startPos);
			
		}*/ // TODO: If our position changed, and we're out of the paths way for whatever reason? (Check Current Node Position)
		/*Vector3 endPos {};
		if(m_pathInfo->path->GetNode(m_pathInfo->path->pathCount -1,pos,endPos) == true)
		{
			auto d = uvec::distance(
		}*/ // Should be checked if end pos actually changed (What if entity?)
	}
	else {
	}
	//
}

void BaseAIComponent::ResetPath()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(CanMove() == false || m_moveInfo.moveOnPath == false || !pTrComponent)
		return;
	m_navInfo.bPathUpdateRequired = true;
	m_navInfo.bTargetReached = false;
	m_navInfo.pathState = PathResult::Updating;
	if(s_navThread != nullptr) {
		m_navInfo.queuedPath = util::make_shared<ai::navigation::PathQuery>(pTrComponent->GetPosition(), GetMoveTarget());
		s_navThread->pendingQueueMutex.lock();
		s_navThread->pendingQueue.push(m_navInfo.queuedPath);
		s_navThread->pendingQueueMutex.unlock();
	}
}

const Vector3 &BaseAIComponent::GetMoveTarget() const { return m_moveInfo.moveTarget; }

bool BaseAIComponent::IsMoving() const
{
	auto animComponent = GetEntity().GetAnimatedComponent();
	return (animComponent.valid() && animComponent->GetActivity() == m_moveInfo.moveActivity) ? true : false;
}

BaseAIComponent::MoveResult BaseAIComponent::MoveTo(const Vector3 &pos, const MoveInfo &info)
{
	if(CanMove() == false)
		return MoveResult::TargetReached;
	auto moveActivity = Activity::Invalid;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.valid()) {
		moveActivity = animComponent->TranslateActivity(info.activity);
		if(moveActivity != m_moveInfo.moveActivity && animComponent->SelectWeightedAnimation(moveActivity) == -1) // Activity doesn't exist
		{
			if(moveActivity == Activity::Walk) // Attempt to select an alternative activity
				moveActivity = Activity::Run;
			else
				moveActivity = Activity::Walk;
		}
	}
	m_moveInfo.moveActivity = moveActivity;
	m_moveInfo.moving = true;
	m_moveInfo.moveSpeed = (std::isnan(info.moveSpeed) == false) ? std::make_unique<float>(info.moveSpeed) : nullptr;
	m_moveInfo.moveOnPath = info.moveOnPath;
	m_moveInfo.moveTarget = pos;
	m_moveInfo.faceTarget = (std::isnan(info.faceTarget.x) == false) ? std::make_unique<Vector3>(info.faceTarget) : nullptr;
	m_moveInfo.turnSpeed = (std::isnan(info.turnSpeed) == false) ? std::make_unique<float>(info.turnSpeed) : nullptr;
	m_moveInfo.destinationTolerance = info.destinationTolerance;
	//auto &start = m_entity->GetPosition();
	auto upDir = GetUpDirection();
	auto d = uvec::planar_distance_sqr(m_moveInfo.moveTarget, m_navInfo.pathTarget, upDir);
	if(d > math::pow2(ai::MAX_NODE_DISTANCE)) // If the new move target is too far away from our old one, we'll probably need a new path
	{
		if(info.moveOnPath == false) // TODO Check if obstructed
			return MoveResult::MovingToTarget;
		m_navInfo.pathTarget = pos;
		ResetPath();
	}
	else if(info.moveOnPath == false)
		return MoveResult::TargetReached;
	if(m_navInfo.bTargetReached == true)
		return MoveResult::TargetReached;
	if(m_navInfo.pathState == PathResult::Failed) {
		m_moveInfo.moving = false;
		return MoveResult::TargetUnreachable;
	}
	if(m_navInfo.pathInfo != nullptr)
		return MoveResult::MovingToTarget;
	assert(m_navInfo.pathState == PathResult::Updating); // If we don't have a valid path and the path generation hasn't failed, then the path must still be generating. Otherwise what happened?
	if(m_navInfo.pathState != PathResult::Updating)      // This shouldn't happen, but if it does, try regenerating the path
	{
		m_navInfo.pathTarget = pos;
		ResetPath();
	}
	return MoveResult::WaitingForPath;
}
void BaseAIComponent::OnPathChanged() {}

float BaseAIComponent::GetMaxSpeed(bool bUseAnimSpeedIfAvailable) const
{
	auto speed = 0.f;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.expired())
		return speed;
	if(GetMoveSpeed(animComponent->GetAnimation(), speed) == true)
		return speed;
	if(bUseAnimSpeedIfAvailable == true) {
		auto *anim = animComponent->GetAnimationObject();
		if(anim != nullptr) {
			auto moveZ = 0.f;
			if(animComponent->CalcAnimationMovementSpeed(nullptr, &moveZ, 0) == true)
				return moveZ;
		}
	}
	return speed;
}

void BaseAIComponent::SetMoveSpeed(int32_t animId, float speed)
{
	if(animId < 0)
		return;
	m_animIdMoveSpeed[animId] = speed;
}
void BaseAIComponent::SetMoveSpeed(const std::string &name, float speed)
{
	m_animMoveSpeed[name] = speed;
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return;
	auto animId = hMdl->LookupAnimation(name);
	if(animId == -1)
		return;
	m_animIdMoveSpeed[animId] = speed;
}
float BaseAIComponent::GetMoveSpeed(int32_t animId) const
{
	auto speed = 0.f;
	GetMoveSpeed(animId, speed);
	return speed;
}
bool BaseAIComponent::GetMoveSpeed(int32_t animId, float &speed) const
{
	auto it = m_animIdMoveSpeed.find(animId);
	if(it == m_animIdMoveSpeed.end()) {
		speed = 0.f;
		return false;
	}
	speed = it->second;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto &scale = pTrComponent->GetScale();
		speed *= math::abs_max(scale.x, scale.y, scale.z);
	}
	return true;
}
void BaseAIComponent::ClearMoveSpeed(int32_t animId)
{
	auto it = m_animIdMoveSpeed.find(animId);
	if(it == m_animIdMoveSpeed.end())
		return;
	m_animIdMoveSpeed.erase(it);
}
void BaseAIComponent::ClearMoveSpeed(const std::string &name)
{
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return;
	auto it = m_animMoveSpeed.find(name);
	if(it != m_animMoveSpeed.end())
		m_animMoveSpeed.erase(name);
	auto animId = hMdl->LookupAnimation(name);
	if(animId == -1)
		return;
	ClearMoveSpeed(animId);
}
Activity BaseAIComponent::GetMoveActivity() const { return m_moveInfo.moveActivity; }

void BaseAIComponent::BlendAnimationMovementMT(std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales)
{
	if(m_seqIdle == -1)
		return;
	//auto act = m_entity->GetActivity();
	//if(act != m_moveActivity) // Doesn't work clientside (Would have to transfer move activity)
	//	return;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	auto &hMdl = GetEntity().GetModel();
	if(animComponent.expired() || hMdl == nullptr /* || animComponent->GetActivity() != m_moveInfo.moveActivity*/)
		return;
	auto *anim = animComponent->GetAnimationObject();
	// Animation movement blending does not work well with special movement animations (e.g. leaping),
	// so we exclude all non-looping movement animations here. (Also see CCharacterComponent::Initialize).
	// If the result isn't satisfactory, alternatively enable the check for the moveActivity above instead.
	// However, this requires sending the moveActivity from the server to the clients (snapshots?)
	if(anim == nullptr || anim->HasFlag(FAnim::Loop) == false)
		return;
	auto moveAnimId = animComponent->GetAnimation();
	auto moveAnim = hMdl->GetAnimation(moveAnimId);
	if(moveAnim == nullptr || m_animMoveInfo.blend == false)
		return;
	auto animIdle = hMdl->GetAnimation(m_seqIdle);
	if(animIdle == nullptr)
		return;
	auto frame = animIdle->GetFrame(0);
	if(frame == nullptr)
		return;
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
	float speed = uvec::length(vel);
	if(m_lastMovementBlendScale == 0.f && speed < 0.5f) // Arbitrary limit; Don't blend animations if speed is below this value
		return;
	float scale = 0.f;
	float speedMax = math::abs(GetMaxSpeed());
	if(speedMax > 0.f) {
		scale = 1.f - (speed / speedMax);
		if(scale < 0.f)
			scale = 0.f;
	}
	m_lastMovementBlendScale = scale = math::approach(m_lastMovementBlendScale, scale, 0.05f);
	auto &dstBonePoses = frame->GetBoneTransforms();
	auto &dstBoneScales = frame->GetBoneScales();
	animComponent->BlendBonePoses(bonePoses, boneScales, dstBonePoses, &dstBoneScales, bonePoses, boneScales, *anim, scale);
}

void BaseAIComponent::OnPathDestinationReached()
{
	m_moveInfo.moving = false; // TODO Do this properly
}
bool BaseAIComponent::HasReachedDestination() const { return m_navInfo.bTargetReached; }
float BaseAIComponent::GetDistanceToMoveTarget() const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!pTrComponent)
		return 0.f;
	return uvec::distance(pTrComponent->GetPosition(), GetMoveTarget());
}

void BaseAIComponent::StopMoving()
{
	if(m_moveInfo.moving == false)
		return;
	m_moveInfo.moveDir = {};
	m_moveInfo.moving = false;
	m_moveInfo.moveSpeed = nullptr;
	auto pAnimComponent = GetEntity().GetAnimatedComponent();
	if(pAnimComponent.valid())
		static_cast<BaseAnimatedComponent *>(pAnimComponent.get())->PlayActivity(Activity::Idle, FPlayAnim::Default);
}

void BaseAIComponent::SetPathNodeIndex(uint32_t nodeIdx, const Vector3 &prevPos)
{
	for(auto i = decltype(m_navInfo.pathInfo->splineNodes.size()) {0}; i < (m_navInfo.pathInfo->splineNodes.size() - 1); ++i)
		m_navInfo.pathInfo->splineNodes[i] = (m_navInfo.pathInfo->splineNodes[i + 1] != nullptr) ? std::make_unique<Vector3>(*m_navInfo.pathInfo->splineNodes[i + 1]) : nullptr;
	m_navInfo.pathInfo->splineNodes.back() = std::make_unique<Vector3>(prevPos);
	m_navInfo.pathInfo->pathIdx = nodeIdx;
	OnPathNodeChanged(m_navInfo.pathInfo->pathIdx);
}

void BaseAIComponent::OnPathNodeChanged(uint32_t nodeIdx) {}

void BaseAIComponent::PathStep(float)
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(CanMove() == false || !pTrComponent)
		return;
	auto &pos = pTrComponent->GetPosition();
	const auto destReachDist = m_moveInfo.destinationTolerance;
	auto upDir = GetUpDirection();
	auto bMoveOnPath = m_moveInfo.moveOnPath == true && m_navInfo.pathInfo != nullptr && m_navInfo.pathInfo->path != nullptr;
	Vector3 tgt;
	if(bMoveOnPath == true) {
		auto &path = *m_navInfo.pathInfo->path;
		if(path.GetNode(m_navInfo.pathInfo->pathIdx, pos, tgt) == false)
			bMoveOnPath = false;
		else {
			tgt.y = 0.f;
			const auto pathNodeReachedDist = math::pow2(10.f);
			while(uvec::planar_distance_sqr(pos, tgt, upDir) <= math::pow2(pathNodeReachedDist)) {
				SetPathNodeIndex(m_navInfo.pathInfo->pathIdx + 1, tgt);
				if(m_navInfo.pathInfo->pathIdx >= CUInt32(path.pathCount)) {
					StopMoving();
					m_navInfo.bTargetReached = true;
					OnPathDestinationReached();
					/*if(GetDistanceToMoveTarget() <= MAX_NODE_DISTANCE)
					{
						m_bTargetReached = true;
						OnPathDestinationReached();
					}*/ // Reached end of path?
					return;
				}
				if(path.GetNode(m_navInfo.pathInfo->pathIdx, pos, tgt) == false) {
					bMoveOnPath = false;
					break;
				}
				tgt.y = 0.f;
			}
		}

		// Calculate spline movement
		// Experimental
		/*
		auto *p2 = &tgt; // Current Node (Move Target)

		auto *p1 = m_pathInfo->splineNodes[1].get(); // Previous node
		if(p1 == nullptr)
			p1 = p2;

		auto *p0 = m_pathInfo->splineNodes[0].get(); // Second previous node
		if(p0 == nullptr)
			p0 = p1;

		Vector3 p3;
		if(path.GetNode(m_pathInfo->pathIdx +1,*p2,p3) == false) // Node after move target
			p3 = *p2;

		p0->y = 0.f;
		p1->y = 0.f;
		p2->y = 0.f;
		p3.y = 0.f;

		auto *game = m_entity->GetNetworkState()->GetGameState();

		auto dirTest = (*p2) -(*p1);
		uvec::normalize(&dirTest);
		auto moveTarget = m_entity->GetPosition() +50.f *dirTest;//(uvec::length(m_entity->GetVelocity()) *tDelta) *dirTest;
		moveTarget = (*p1) +glm::gtx::proj(moveTarget -(*p1),(*p2) -(*p1));

		game->DrawLine(moveTarget,moveTarget +Vector3(0,50,0),colors::Fuchsia,0.1f);

		auto segDist = uvec::distance(*p1,*p2);
		auto s = (segDist > 0.f) ? pragma::math::min(uvec::distance(*p1,moveTarget) /segDist,1.f) : 0.f;
		//Con::COUT<<"s: "<<s<<"; segDist: "<<segDist<<"; Distance to move target: "<<uvec::distance(*p1,moveTarget)<<Con::endl;
		tgt = pragma::util::calc_hermite_spline_position(*p0,*p1,*p2,p3,s,1.f);//0.f);

		game->DrawLine(*p0,*p1,colors::Yellow,0.1f);
		game->DrawLine(*p1,*p2,colors::Aqua,0.1f);
		game->DrawLine(*p2,p3,colors::Lime,0.1f);
		game->DrawLine(tgt,tgt +Vector3(0,100,0),colors::Red,0.1f);
		*/
		//
	}
	if(bMoveOnPath == false) // No navigation info available; Try to move to target in a straight line
	{
		tgt = m_moveInfo.moveTarget;
		if(uvec::planar_distance_sqr(pos, tgt, upDir) <= math::pow2(destReachDist)) {
			StopMoving();
			m_navInfo.bTargetReached = true;
			OnPathDestinationReached();
			return;
		}
	}

	auto dir = tgt - pos;
	dir = uvec::project_to_plane(dir, upDir, 0.f);
	auto l = uvec::length(dir);
	if(l != 0.f)
		dir /= l;
	ResolvePathObstruction(dir);
	m_moveInfo.moveDir = dir;
}
void BaseAIComponent::ResolvePathObstruction(Vector3 &dir)
{
	if(m_moveInfo.moving == false)
		return;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return;
	auto *game = ent.GetNetworkState()->GetGameState();
	auto &t = game->CurTime();
	if(t < m_obstruction.nextObstructionCheck) {
		if(m_obstruction.pathObstructed == true)
			dir = m_obstruction.resolveDirection;
		return;
	}
	auto pos = ent.GetCenter();
	auto pPhysComponent = ent.GetPhysicsComponent();

	auto dstPos = pos + dir * (pPhysComponent ? (pPhysComponent->GetCollisionRadius() * 1.1f) : 0.f);

	const auto fCheckForObstruction = [this, &pTrComponent, &dir, t](physics::TraceResult &r) -> bool {
		if(r.hitType != physics::RayCastHitType::None && (r.entity.valid() == false || IsObstruction(*r.entity.get()))) // Obstructed
		{
			m_obstruction.pathObstructed = true;
			auto aimDir = pTrComponent->GetForward();
			auto newDir = aimDir - glm::gtx::proj(aimDir, r.normal);
			uvec::normalize(&newDir);
			dir = newDir;
			m_obstruction.resolveDirection = dir;
		}
		else
			m_obstruction.pathObstructed = false;
		m_obstruction.nextObstructionCheck = t + ((m_obstruction.pathObstructed == false) ? 0.8 : 0.2);
		return m_obstruction.pathObstructed;
	};

#if AI_OBSTRUCTION_CHECK_RAYCAST_TYPE == AI_OBSTRUCTION_CHECK_RAYCAST_TYPE_RAY
	TraceData data {};
	data.SetFilter(m_obstruction.sweepFilter);
	data.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter);
	if(pPhysComponent != nullptr) {
		data.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		data.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask());
	}

	auto numRays = 3u;
	auto charComponent = ent.GetCharacterComponent();
	auto upDir = charComponent.valid() ? charComponent->GetUpDirection() : uvec::PRM_UP;
	auto height = 0.f;
	if(pPhysComponent != nullptr) {
		Vector3 min, max;
		pPhysComponent->GetCollisionBounds(&min, &max);
		height = min.y + max.y;
	}
	else
		numRays = 0u;
	auto offset = height / 5.f; // 3 raycasts +margin at bottom and top

	// Perform several raycasts near the center of the body of the NPC
	for(auto i = 0u; i < numRays; ++i) {
		auto vOffset = upDir * (offset + i * offset);
		data.SetSource(pos + vOffset);
		data.SetTarget(dstPos + vOffset);
		auto r = game->RayCast(data);
		if(fCheckForObstruction(r) == true)
			return;
	}
#else
	auto *physObj = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(physObj == nullptr || physObj->IsController() == false)
		return;
	auto *physController = static_cast<physics::ControllerPhysObj *>(physObj);
	auto *shape = physController->GetController()->GetShape();
	if(shape == nullptr)
		return;
	// If the NPC's physics object is a capsule shape (which is very likely), we'll have to offset the position
	// so that the capsule sweep isn't inside the ground (since a capsule's origin is at its center, not near the feet, but the NPCs
	// position is always at the feet).
	if(physController->IsCapsule()) {
		auto *capsuleController = static_cast<physics::CapsuleControllerPhysObj *>(physObj);
		auto rot = physObj->GetOrientation();
		auto offset = Vector3(0, capsuleController->GetHeight() * 0.5f, 0);
		uvec::rotate(&offset, rot);
		pos += offset;
		dstPos += offset;
	}

	physics::TraceData data {};
	data.SetSource(pos);
	data.SetShape(*shape);
	data.SetTarget(dstPos);
	data.SetFilter(m_obstruction.sweepFilter);
	data.SetFlags(physics::RayCastFlags::Default | physics::RayCastFlags::InvertFilter);
	if(pPhysComponent) {
		data.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		data.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask());
	}

	auto result = game->Sweep(data);
	fCheckForObstruction(result);
#endif
}

bool BaseAIComponent::IsObstruction(const ecs::BaseEntity &ent) const { return true; }

void BaseAIComponent::OnEntityComponentAdded(BaseEntityComponent &component) { BaseEntityComponent::OnEntityComponentAdded(component); }

Vector2 BaseAIComponent::CalcMovementSpeed() const
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pPhysComponent && (pPhysComponent->GetMoveType() != physics::MoveType::Walk || pPhysComponent->IsOnGround() == false))
		return {pVelComponent.valid() ? uvec::length(pVelComponent->GetVelocity()) : 0.f, 0.f};
	auto speed = 0.f;
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.expired())
		return {speed, 0.f};
	if(GetMoveSpeed(animComponent->GetAnimation(), speed))
		return {speed, 0.f};
	if(m_animMoveInfo.moving == true) {
		Vector2 move {};
		if(animComponent->CalcAnimationMovementSpeed(&move.y, &move.x) == true)
			return move;
	}
	return {speed, 0.f};
}
float BaseAIComponent::CalcAirMovementModifier() const { return 0.f; }
float BaseAIComponent::CalcMovementAcceleration() const { return 80.f; }
Vector3 BaseAIComponent::CalcMovementDirection() const
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pPhysComponent && (pPhysComponent->GetMoveType() != physics::MoveType::Walk || pPhysComponent->IsOnGround() == false)) {
		auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
		uvec::normalize(&vel);
		return vel;
	}
	if(m_animMoveInfo.moving == true) {
		auto bMoveForward = true;
		auto animComponent = ent.GetAnimatedComponent();
		auto &hMdl = GetEntity().GetModel();
		if(hMdl != nullptr && animComponent.valid()) {
			auto anim = hMdl->GetAnimation(animComponent->GetAnimation());
			if(anim != nullptr) {
				auto *bc = anim->GetBlendController();
				if(ent.IsCharacter() && bc && bc->controller == ent.GetCharacterComponent()->GetMoveController())
					bMoveForward = false; // Animation has a move blend-controller, which means it probably allows sideways or backwards movement
			}
		}
		if(bMoveForward == true) {
			auto pTrComponent = ent.GetTransformComponent();
			return pTrComponent ? pTrComponent->GetForward() : uvec::PRM_FORWARD;
		}
	}
	return (m_moveInfo.moving == true) ? m_moveInfo.moveDir : Vector3 {};
}
