#include "stdafx_shared.h"
#if 0
#include "pragma/entities/basenpc.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/raytraces.h"
#include <pragma/physics/movetypes.h>
#include <glm/gtx/projection.hpp>

BaseNPC::PathInfo::PathInfo(const std::shared_ptr<RcPathResult> &path)
	: path(path),pathIdx(0)
{}

//////////////////

BaseNPC::MoveInfo::MoveInfo(Activity act)
	: MoveInfo()
{
	activity = act;
}
BaseNPC::MoveInfo::MoveInfo(Activity act,bool bMoveOnPath)
	: MoveInfo()
{
	activity = act;
	moveOnPath = bMoveOnPath;
}
BaseNPC::MoveInfo::MoveInfo(Activity act,bool bMoveOnPath,const Vector3 &faceTarget,float moveSpeed,float turnSpeed)
	: MoveInfo()
{
	activity = act;
	moveOnPath = bMoveOnPath;
	this->faceTarget = faceTarget;
	this->moveSpeed = moveSpeed;
	this->turnSpeed = turnSpeed;
}

//////////////////

decltype(BaseNPC::s_tAiNav) BaseNPC::s_tAiNav {};
decltype(BaseNPC::s_aiQueueMutex) BaseNPC::s_aiQueueMutex {};
decltype(BaseNPC::s_aiNavQueue) BaseNPC::s_aiNavQueue {};

void BaseNPC::UpdatePath()
{
	if(CanMove() == false || m_moveInfo.moveOnPath == false)
		return;
	if(m_navInfo.queuedPath != nullptr)
	{
		auto bPathChanged = false;
		s_aiQueueMutex.lock();
			if(m_navInfo.queuedPath->complete == true)
			{
				if(m_navInfo.queuedPath->pathInfo != nullptr)
				{
					m_navInfo.pathInfo = m_navInfo.queuedPath->pathInfo;
					m_navInfo.pathState = PathResult::Success;
				}
				else
				{
					//Con::cerr<<"Failed to generate path!"<<Con::endl;
					m_navInfo.pathState = PathResult::Failed;
				}
				m_navInfo.queuedPath = nullptr;
				bPathChanged = true;
			}
		s_aiQueueMutex.unlock();
		if(bPathChanged == true)
			OnPathChanged();
	}
	auto bPathUpdateRequired = (m_navInfo.pathInfo == nullptr || m_navInfo.bPathUpdateRequired == true) ? true : false;
	if(bPathUpdateRequired == false)
	{
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
	else
	{
		

	}
	//
}

void BaseNPC::ResetPath()
{
	if(CanMove() == false || m_moveInfo.moveOnPath == false)
		return;
	m_navInfo.bPathUpdateRequired = true;
	m_navInfo.bTargetReached = false;
	m_navInfo.pathState = PathResult::Updating;
	s_aiQueueMutex.lock();
		m_navInfo.queuedPath = std::shared_ptr<PathQuery>(new PathQuery(m_entity->GetPosition(),GetMoveTarget()));
		s_aiNavQueue.push(m_navInfo.queuedPath);
	s_aiQueueMutex.unlock();
}

const Vector3 &BaseNPC::GetMoveTarget() const {return m_moveInfo.moveTarget;}

bool BaseNPC::IsMoving() const
{
	return (m_entity->GetModelComponent().GetActivity() == m_moveInfo.moveActivity) ? true : false;
}

BaseNPC::MoveResult BaseNPC::MoveTo(const Vector3 &pos,const MoveInfo &info)
{
	if(CanMove() == false)
		return BaseNPC::MoveResult::TargetReached;
	auto moveActivity = m_entity->GetModelComponent().TranslateActivity(info.activity);
	if(moveActivity != m_moveInfo.moveActivity && m_entity->GetModelComponent().SelectWeightedAnimation(moveActivity) == -1) // Activity doesn't exist
	{
		if(moveActivity == Activity::Walk) // Attempt to select an alternative activity
			moveActivity = Activity::Run;
		else
			moveActivity = Activity::Walk;
	}
	m_moveInfo.moveActivity = moveActivity;
	m_moveInfo.moving = true;
	m_moveInfo.moveSpeed = (std::isnan(info.moveSpeed) == false) ? std::make_unique<float>(info.moveSpeed) : nullptr;
	m_moveInfo.moveOnPath = info.moveOnPath;
	m_moveInfo.moveTarget = pos;
	m_moveInfo.faceTarget = (std::isnan(info.faceTarget.x) == false) ? std::make_unique<Vector3>(info.faceTarget) : nullptr;
	m_moveInfo.turnSpeed = (std::isnan(info.turnSpeed) == false) ? std::make_unique<float>(info.turnSpeed) : nullptr;
	//auto &start = m_entity->GetPosition();
	auto d = uvec::planar_distance_sqr(m_moveInfo.moveTarget,m_navInfo.pathTarget,GetUpDirection());
	if(d > umath::pow2(MAX_NODE_DISTANCE)) // If the new move target is too far away from our old one, we'll probably need a new path
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
	if(m_navInfo.pathState == PathResult::Failed)
	{
		m_moveInfo.moving = false;
		return MoveResult::TargetUnreachable;
	}
	if(m_navInfo.pathInfo != nullptr)
		return MoveResult::MovingToTarget;
	assert(m_navInfo.pathState == PathResult::Updating); // If we don't have a valid path and the path generation hasn't failed, then the path must still be generating. Otherwise what happened?
	if(m_navInfo.pathState != PathResult::Updating) // This shouldn't happen, but if it does, try regenerating the path
	{
		m_navInfo.pathTarget = pos;
		ResetPath();
	}
	return MoveResult::WaitingForPath;
}
void BaseNPC::OnPathChanged() {}

float BaseNPC::GetMaxSpeed(bool bUseAnimSpeedIfAvailable) const
{
	auto speed = 0.f;
	if(GetMoveSpeed(m_entity->GetModelComponent().GetAnimation(),speed) == true)
		return speed;
	if(bUseAnimSpeedIfAvailable == true)
	{
		auto *anim = m_entity->GetModelComponent().GetAnimationObject();
		if(anim != nullptr)
		{
			auto moveZ = 0.f;
			if(m_entity->CalcAnimationMovementSpeed(nullptr,&moveZ,-1) == true)
				return moveZ;
		}
	}
	return speed;
}

void BaseNPC::SetMoveSpeed(int32_t animId,float speed)
{
	if(animId < 0)
		return;
	m_animIdMoveSpeed[animId] = speed;
}
void BaseNPC::SetMoveSpeed(const std::string &name,float speed)
{
	m_animMoveSpeed[name] = speed;
	auto &mdl = m_entity->GetModelComponent().GetModel();
	if(mdl == nullptr)
		return;
	auto animId = mdl->LookupAnimation(name);
	if(animId == -1)
		return;
	m_animIdMoveSpeed[animId] = speed;
}
float BaseNPC::GetMoveSpeed(int32_t animId) const
{
	auto speed = 0.f;
	GetMoveSpeed(animId,speed);
	return speed;
}
bool BaseNPC::GetMoveSpeed(int32_t animId,float &speed) const
{
	auto it = m_animIdMoveSpeed.find(animId);
	if(it == m_animIdMoveSpeed.end())
	{
		speed = 0.f;
		return false;
	}
	auto &scale = m_entity->GetScale();
	speed = it->second *umath::abs_max(scale.x,scale.y,scale.z);
	return true;
}
void BaseNPC::ClearMoveSpeed(int32_t animId)
{
	auto it = m_animIdMoveSpeed.find(animId);
	if(it == m_animIdMoveSpeed.end())
		return;
	m_animIdMoveSpeed.erase(it);
}
void BaseNPC::ClearMoveSpeed(const std::string &name)
{
	auto &mdl = m_entity->GetModelComponent().GetModel();
	if(mdl == nullptr)
		return;
	auto it = m_animMoveSpeed.find(name);
	if(it != m_animMoveSpeed.end())
		m_animMoveSpeed.erase(name);
	auto animId = mdl->LookupAnimation(name);
	if(animId == -1)
		return;
	ClearMoveSpeed(animId);
}
Activity BaseNPC::GetMoveActivity() const {return m_moveInfo.moveActivity;}

void BaseNPC::BlendAnimationMovement(std::vector<Orientation> &boneOrientations,std::vector<Vector3> *boneScales)
{
	if(m_seqIdle == -1)
		return;
	//auto act = m_entity->GetActivity();
	//if(act != m_moveActivity) // Doesn't work clientside (Would have to transfer move activity)
	//	return;
	Model *mdl = m_entity->GetModelComponent().GetModel().get();
	auto moveAnimId = m_entity->GetModelComponent().GetAnimation();
	auto moveAnim = mdl->GetAnimation(moveAnimId);
	if(moveAnim == nullptr || m_animMoveInfo.blend == false)
		return;
	auto anim = mdl->GetAnimation(m_seqIdle);
	if(anim == nullptr)
		return;
	auto frame = anim->GetFrame(0);
	if(frame == NULL)
		return;
	auto &vel = m_entity->GetVelocity();
	float speed = uvec::length(vel);
	if(m_lastMovementBlendScale == 0.f && speed < 0.5f) // Arbitrary limit; Don't blend animations if speed is below this value
		return;
	float scale = 0.f;
	float speedMax = umath::abs(GetMaxSpeed());
	if(speedMax > 0.f)
	{
		scale = 1.f -(speed /speedMax);
		if(scale < 0.f)
			scale = 0.f;
	}
	m_lastMovementBlendScale = scale = umath::approach(m_lastMovementBlendScale,scale,0.05f);
	m_entity->GetModelComponent().BlendBoneFrames(boneOrientations,boneScales,*anim,frame.get(),scale);
}

void BaseNPC::OnPathDestinationReached()
{
	m_moveInfo.moving = false; // TODO Do this properly
}
bool BaseNPC::HasReachedDestination() const {return m_navInfo.bTargetReached;}
float BaseNPC::GetDistanceToMoveTarget() const {return uvec::distance(m_entity->GetPosition(),GetMoveTarget());}

void BaseNPC::StopMoving()
{
	m_moveInfo.moveDir = {};
	m_moveInfo.moving = false;
	m_moveInfo.moveSpeed = nullptr;
}

void BaseNPC::SetPathNodeIndex(uint32_t nodeIdx,const Vector3 &prevPos)
{
	for(auto i=decltype(m_navInfo.pathInfo->splineNodes.size()){0};i<(m_navInfo.pathInfo->splineNodes.size() -1);++i)
		m_navInfo.pathInfo->splineNodes[i] = (m_navInfo.pathInfo->splineNodes[i +1] != nullptr) ? std::make_unique<Vector3>(*m_navInfo.pathInfo->splineNodes[i +1]) : nullptr;
	m_navInfo.pathInfo->splineNodes.back() = std::make_unique<Vector3>(prevPos);
	m_navInfo.pathInfo->pathIdx = nodeIdx;
	OnPathNodeChanged(m_navInfo.pathInfo->pathIdx);
}

void BaseNPC::OnPathNodeChanged(uint32_t nodeIdx) {}

void BaseNPC::PathStep(float)
{
	if(CanMove() == false)
		return;
	auto &pos = m_entity->GetPosition();
	const auto destReachDist = 10.f;
	Vector3 tgt;
	if(m_moveInfo.moveOnPath == true && m_navInfo.pathInfo != nullptr && m_navInfo.pathInfo->path != nullptr)
	{
		auto &path = *m_navInfo.pathInfo->path;
		if(path.GetNode(m_navInfo.pathInfo->pathIdx,pos,tgt) == false)
			tgt = m_moveInfo.moveTarget; // No path nodes left; Just try to move directly to our target
		else
		{
			tgt.y = 0.f;
			while(uvec::planar_distance_sqr(pos,tgt,GetUpDirection()) <= umath::pow2(destReachDist))
			{
				SetPathNodeIndex(m_navInfo.pathInfo->pathIdx +1,tgt);
				if(m_navInfo.pathInfo->pathIdx >= CUInt32(path.pathCount))
				{
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
				path.GetNode(m_navInfo.pathInfo->pathIdx,pos,tgt);
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
		moveTarget = (*p1) +glm::proj(moveTarget -(*p1),(*p2) -(*p1));

		game->DrawLine(moveTarget,moveTarget +Vector3(0,50,0),Color::Fuchsia,0.1f);

		auto segDist = uvec::distance(*p1,*p2);
		auto s = (segDist > 0.f) ? umath::min(uvec::distance(*p1,moveTarget) /segDist,1.f) : 0.f;
		//Con::cout<<"s: "<<s<<"; segDist: "<<segDist<<"; Distance to move target: "<<uvec::distance(*p1,moveTarget)<<Con::endl;
		tgt = util::calc_hermite_spline_position(*p0,*p1,*p2,p3,s,1.f);//0.f);

		game->DrawLine(*p0,*p1,Color::Yellow,0.1f);
		game->DrawLine(*p1,*p2,Color::Aqua,0.1f);
		game->DrawLine(*p2,p3,Color::Lime,0.1f);
		game->DrawLine(tgt,tgt +Vector3(0,100,0),Color::Red,0.1f);
		*/
		//
	}
	else // No navigation info available; Try to move to target in a straight line
	{
		tgt = m_moveInfo.moveTarget;
		if(uvec::planar_distance_sqr(pos,tgt,GetUpDirection()) <= umath::pow2(destReachDist))
		{
			StopMoving();
			m_navInfo.bTargetReached = true;
			OnPathDestinationReached();
			return;
		}
	}

	auto dir = tgt -pos;
	dir = uvec::project_to_plane(dir,GetUpDirection(),0.f);
	uvec::normalize(&dir);
	ResolvePathObstruction(dir);
	m_moveInfo.moveDir = dir;
}

void BaseNPC::ResolvePathObstruction(Vector3 &dir)
{
	if(m_moveInfo.moving == false)
		return;
	auto *game = m_entity->GetNetworkState()->GetGameState();
	auto &t = game->CurTime();
	if(t < m_obstruction.nextObstructionCheck)
	{
		if(m_obstruction.pathObstructed == true)
			dir = m_obstruction.resolveDirection;
		return;
	}
	auto pos = m_entity->GetCenter();

	TraceData data {};
	data.SetSource(pos);
	data.SetSource(m_entity);
	data.SetTarget(pos +dir *(m_entity->GetCollisionRadius() *1.1f));
	data.SetFilter(m_obstruction.sweepFilter);
	data.SetFlags(FTRACE::FILTER_INVERT);
	data.SetCollisionFilterGroup(m_entity->GetCollisionFilter());
	data.SetCollisionFilterMask(m_entity->GetCollisionFilterMask());
	
	auto r = game->Sweep(data);
	if((m_obstruction.pathObstructed = r.hit) == true && (r.entity.IsValid() == false || IsObstruction(*r.entity.get()))) // Obstructed
	{
		auto aimDir = m_entity->GetForward();
		auto newDir = aimDir -glm::proj(aimDir,r.normal);
		uvec::normalize(&newDir);
		dir = newDir;
		m_obstruction.resolveDirection = dir;
	}
	m_obstruction.nextObstructionCheck = t +((m_obstruction.pathObstructed == false) ? 0.8 : 0.2);
}

bool BaseNPC::IsObstruction(const BaseEntity &ent) const {return true;}

void BaseNPC::SimulateMovement(double tDelta)
{
	BaseActor::SimulateMovement(tDelta);
	PathStep(static_cast<float>(tDelta));
}

bool BaseNPC::UpdateMovement()
{
	if(CanMove() == false)
		return false;
	return BaseCharacter::UpdateMovement();
}

bool BaseNPC::CanMove() const {return (m_entity->GetMoveType() == MOVETYPE::WALK) ? true : false;}

Vector2 BaseNPC::CalcMovementSpeed() const
{
	if(m_entity->GetMoveType() != MOVETYPE::WALK || m_entity->IsOnGround() == false)
		return {uvec::length(m_entity->GetVelocity()),0.f};
	auto speed = 0.f;
	if(GetMoveSpeed(m_entity->GetModelComponent().GetAnimation(),speed))
		return {speed,0.f};
	if(m_animMoveInfo.moving == true)
	{
		Vector2 move {};
		if(m_entity->CalcAnimationMovementSpeed(&move.y,&move.x) == true)
			return move;
	}
	return {speed,0.f};
}
float BaseNPC::CalcAirMovementModifier() const {return 0.f;}
float BaseNPC::CalcMovementAcceleration() const {return 80.f;}
Vector3 BaseNPC::CalcMovementDirection(const Vector3&,const Vector3&) const
{
	if(m_entity->GetMoveType() != MOVETYPE::WALK || m_entity->IsOnGround() == false)
	{
		auto vel = m_entity->GetVelocity();
		uvec::normalize(&vel);
		return vel;
	}
	if(m_animMoveInfo.moving == true)
	{
		auto bMoveForward = true;
		auto &mdl = m_entity->GetModelComponent().GetModel();
		if(mdl != nullptr)
		{
			auto anim = mdl->GetAnimation(m_entity->GetModelComponent().GetAnimation());
			if(anim != nullptr)
			{
				auto *bc = anim->GetBlendController();
				if(bc != nullptr && bc->controller == GetMoveController())
					bMoveForward = false; // Animation has a move blend-controller, which means it probably allows sideways or backwards movement
			}
		}
		if(bMoveForward == true)
			return m_entity->GetForward();
	}
	return (m_moveInfo.moving == true) ? m_moveInfo.moveDir : Vector3{};
}
#endif