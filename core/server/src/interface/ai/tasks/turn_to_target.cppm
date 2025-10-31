// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"




export module pragma.server.ai.tasks.turn_to_target;

import pragma.server.ai.tasks.target;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskTurnToTarget : public TaskTarget {
		  protected:
			std::unique_ptr<float> m_targetAng = nullptr;
			bool IsFacingTarget(pragma::BaseAIComponent &ent, const Vector3 &pos) const;
		  public:
			TaskTurnToTarget();
			TaskTurnToTarget(const TaskTurnToTarget &other);
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, pragma::BaseAIComponent &ent) override;
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
		};
	};
};
