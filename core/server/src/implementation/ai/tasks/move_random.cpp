// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.move_random;

import :entities.components;
import :game;

using namespace pragma;

void ai::TaskMoveRandom::Print(const Schedule *sched, std::ostream &o) const { o << "MoveRandom[" << GetMoveDistance(sched) << "]"; }
void ai::TaskMoveRandom::SetMoveDistance(float dist) { SetParameter(math::to_integral(Parameter::Distance), dist); }
void ai::TaskMoveRandom::SetMoveActivity(Activity act) { SetParameter(math::to_integral(Parameter::MoveActivity), math::to_integral(act)); }

float ai::TaskMoveRandom::GetMoveDistance(const Schedule *sched) const
{
	auto *paramDist = GetParameter(sched, math::to_integral(Parameter::Distance));
	auto dist = 1'000.f;
	if(paramDist != nullptr && paramDist->GetType() == Schedule::Parameter::Type::Float)
		dist = paramDist->GetFloat();
	return dist;
}
ai::BehaviorNode::Result ai::TaskMoveRandom::Start(const Schedule *sched, BaseAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);
	auto pTrComponent = ent.GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return Result::Failed;
	auto dist = GetMoveDistance(sched);
	auto dir2 = uvec::create_random_unit_vector2();
	Vector3 dir {dir2.x, 0.f, dir2.y};
	auto &origin = pTrComponent->GetPosition();
	auto endPos = origin + dir * dist;
	Vector3 hitPos = endPos;
	auto &navMesh = SGame::Get()->GetNavMesh();
	if(navMesh != nullptr)
		navMesh->RayCast(origin, endPos, hitPos);
	m_moveTarget = hitPos;
	return Result::Pending;
}

ai::BehaviorNode::Result ai::TaskMoveRandom::Think(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = BehaviorNode::Think(sched, ent);
	if(r != Result::Succeeded)
		return r;
	auto moveAct = Activity::Run;
	auto *paramAct = GetParameter(sched, math::to_integral(Parameter::MoveActivity));
	if(paramAct != nullptr && paramAct->GetType() == Schedule::Parameter::Type::Int)
		moveAct = static_cast<Activity>(paramAct->GetInt());

	auto moveDistance = MAX_NODE_DISTANCE; // TODO
	auto moveResult = ent.MoveTo(m_moveTarget, moveAct);
	if(ent.GetDistanceToMoveTarget() <= moveDistance || moveResult == SAIComponent::MoveResult::TargetReached) {
		ent.OnPathDestinationReached();
		return r;
	}
	if(moveResult == SAIComponent::MoveResult::TargetUnreachable)
		return Result::Failed;
	return Result::Pending;
}
