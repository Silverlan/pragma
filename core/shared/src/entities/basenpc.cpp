#include "stdafx_shared.h"
#if 0
#include "pragma/entities/basenpc.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/physicstypes.h"
#include "pragma/model/animation/activities.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "pragma/model/animation/fanim.h"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/baseworld.h"
#include "pragma/math/util_hermite.h"
#include "pragma/util/util_approach_rotation.hpp"

decltype(BaseNPC::s_npcCount) BaseNPC::s_npcCount = {0};
decltype(BaseNPC::s_bThreadInitialized) BaseNPC::s_bThreadInitialized = false;

BaseNPC::PathQuery::PathQuery(const Vector3 &_start,const Vector3 &_end)
	: start(_start),end(_end),complete(false)
{}

//////////////////

BaseNPC::BaseNPC()
	: BaseCharacter(),BaseProp(),m_seqIdle(-1),
	m_navInfo(),m_obstruction()
{
	m_obstruction.nextObstructionCheck = 0.0;
	m_obstruction.pathObstructed = false;
	m_obstruction.resolveDirection = {};

	m_turnSpeed = 160.f;
	++s_npcCount;
}

BaseNPC::~BaseNPC()
{
	s_aiQueueMutex.lock();
		while(!s_aiNavQueue.empty())
			s_aiNavQueue.pop();
	s_aiQueueMutex.unlock();
	if(--s_npcCount == 0)
	{
		s_tAiNav.join();
		s_bThreadInitialized = false;
	}
}

void BaseNPC::OnLookTargetChanged() {}

bool BaseNPC::TurnStep(const Vector3 &target,float &turnAngle,const float *turnSpeed)
{
	if(CanMove() == false)
	{
		turnAngle = 0.f;
		return true;
	}
	auto *nw = m_entity->GetNetworkState();
	auto *game = nw->GetGameState();
	auto dir = target -m_entity->GetPosition();
	auto l = uvec::length(dir);
	if(l == 0.f)
	{
		turnAngle = 0.f;
		return true;
	}
	dir /= l;
	auto speedMax = static_cast<double>((turnSpeed != nullptr) ? *turnSpeed : GetTurnSpeed()) *game->DeltaTickTime();
	Vector2 rotAm = {};
	const Vector2 pitchLimit {0.f,0.f};
	auto newRot = uquat::approach_direction(m_entity->GetOrientation(),GetUpDirection(),dir,Vector2(speedMax,speedMax),&rotAm,&pitchLimit);
	m_entity->SetOrientation(newRot);
	return (umath::abs(rotAm.y) <= speedMax) ? true : false;

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
	auto y = umath::approach_angle(0.f,angTgt.y,speedMax);
	auto diff = umath::get_angle_difference(angTgt.y,y);
	angTgt.y = y;
	m_entity->SetOrientation();
	turnAngle = diff;
	return (fabs(diff) <= speedMax) ? true : false;*/
	/*auto angCur = m_entity->GetAngles();
	auto angTgt = uvec::to_angle(dir);
	angTgt.p = 0;
	angTgt.r = 0;
	auto speedMax = static_cast<double>((turnSpeed != nullptr) ? *turnSpeed : GetTurnSpeed()) *game->DeltaTickTime();
	auto y = umath::approach_angle(angCur.y,angTgt.y,speedMax);
	auto diff = umath::get_angle_difference(angTgt.y,y);
	angTgt.y = y;
	m_entity->SetAngles(angTgt);
	turnAngle = diff;
	return (fabs(diff) <= speedMax) ? true : false;*/
}

bool BaseNPC::TurnStep(const Vector3 &target,const float *turnSpeed)
{
	auto turnAngle = 0.f;
	return TurnStep(target,turnAngle,turnSpeed);
}

void BaseNPC::Initialize()
{
	BaseCharacter::InitializeCharacter();
	m_entity = dynamic_cast<BaseEntity*>(this);
	m_entity->GetModelComponent().AddCallback(pragma::EComponentModelCallbackEvent::OnModelChanged,FunctionCallback<>::Create([this]() {
		auto &mdl = m_entity->GetModelComponent().GetModel();
		if(mdl == nullptr)
			return;
		for(auto &pair : m_animMoveSpeed)
		{
			auto animId = mdl->LookupAnimation(pair.first);
			if(animId == -1)
				continue;
			m_animIdMoveSpeed[animId] = pair.second;
		}
	}));
	m_entity->GetModelComponent().AddCallback(pragma::EComponentModelCallbackEvent::OnModelChanged,FunctionCallback<void,std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) {
		m_seqIdle = -1;
		if(mdl.get() == nullptr)
			return;
		std::vector<unsigned int> animations;
		mdl.get()->GetAnimations(Activity::Idle,animations);
		if(animations.empty())
			return;
		m_seqIdle = animations.front();
		auto it = std::find_if(animations.begin(),animations.end(),[&mdl](uint32_t animId) {
			auto anim = mdl.get()->GetAnimation(animId);
			return (anim != nullptr && !anim->HasFlag(FAnim::NoRepeat)) ? true : false; // Prefer an idle animation that can be repeated (More likely to be a generic idle animation)
		});
		if(it != animations.end())
			m_seqIdle = *it;
	}));
	m_entity->GetModelComponent().AddCallback(pragma::EComponentModelCallbackEvent::OnAnimationStart,FunctionCallback<void,int32_t,Activity,pragma::FPlayAnim>::Create([this](int32_t animId,Activity act,pragma::FPlayAnim flags) {
		auto *anim = m_entity->GetModelComponent().GetAnimationObject();
		if(anim == nullptr)
		{
			m_animMoveInfo.moving = false;
			m_animMoveInfo.blend = false;
			return;
		}
		m_animMoveInfo.blend = !anim->HasFlag(FAnim::NoMoveBlend);
		m_animMoveInfo.moving = (anim->HasFlag(FAnim::MoveX) || anim->HasFlag(FAnim::MoveZ)) ? true : false;
	}));
	m_entity->GetModelComponent().AddCallback(pragma::EComponentModelCallbackEvent::OnBlendAnimation,FunctionCallback<void,pragma::ComponentModel::AnimationSlotInfo*,Activity,std::vector<Orientation>*,std::vector<Vector3>*>::Create([this](
		pragma::ComponentModel::AnimationSlotInfo *animInfo,Activity act,std::vector<Orientation> *boneOrientations,std::vector<Vector3> *boneScales
	) {
		if(animInfo == &m_entity->GetModelComponent().GetBaseAnimationInfo()) // Only apply for base animation, not for gestures
			BaseNPC::BlendAnimationMovement(*boneOrientations,boneScales);
	}));

	auto *game = m_entity->GetNetworkState()->GetGameState();
	auto *world = game->GetWorld();
	m_obstruction.sweepFilter.push_back(m_entity->GetHandle());
	if(world != nullptr)
		m_obstruction.sweepFilter.push_back(dynamic_cast<LuaObj<EntityHandle>*>(world)->GetHandle()); // The navigation mesh should already ensure there's no collision with the world, so we'll exclude it here

	if(s_bThreadInitialized == false)
	{
		s_bThreadInitialized = true;
		auto wpNavMesh = std::weak_ptr<pragma::nav::Mesh>(m_entity->GetNetworkState()->GetGameState()->GetNavMesh());
		s_tAiNav = std::thread([wpNavMesh]() {
			while(s_npcCount > 0)
			{
				s_aiQueueMutex.lock();
					auto bEmpty = s_aiNavQueue.empty();
					if(bEmpty == false)
					{
						auto item = s_aiNavQueue.front();
						auto navMesh = wpNavMesh.lock();
						std::shared_ptr<RcPathResult> path = nullptr;
						if(navMesh != nullptr && (path = navMesh->FindPath(item->start,item->end)) != nullptr)
							item->pathInfo = std::shared_ptr<PathInfo>(new PathInfo(path));
						s_aiNavQueue.pop();
						item->complete = true;

						bEmpty = s_aiNavQueue.empty();
					}
				s_aiQueueMutex.unlock();
				if(bEmpty == true)
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});
	}
}

void BaseNPC::InitializeController()
{
	BaseCharacter::InitializeController();
}

void BaseNPC::InitializePhysObj(PhysObj *phys)
{
	BaseCharacter::InitializePhysObj(phys);
	m_entity->AddCollisionFilter(CollisionMask::NPC);
}

void BaseNPC::Spawn()
{}

void BaseNPC::Think(double tDelta)
{
	BaseCharacter::Think(tDelta);
	UpdatePath();
	LookAtStep(tDelta);
}

const char *BaseNPC::MoveResultToString(MoveResult result)
{
	switch(result)
	{
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
#endif
