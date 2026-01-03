// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.turn_to_target;

import :ai.tasks.target;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskTurnToTarget : public TaskTarget {
		  protected:
			std::unique_ptr<float> m_targetAng = nullptr;
			bool IsFacingTarget(BaseAIComponent &ent, const Vector3 &pos) const;
		  public:
			TaskTurnToTarget();
			TaskTurnToTarget(const TaskTurnToTarget &other);
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
		};
	};
};
