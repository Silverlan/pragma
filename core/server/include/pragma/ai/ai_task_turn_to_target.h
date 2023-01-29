/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_TASK_TURN_TO_TARGET_H__
#define __AI_TASK_TURN_TO_TARGET_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_task_target.h"

namespace pragma {
	namespace ai {
		class DLLSERVER TaskTurnToTarget : public TaskTarget {
		  protected:
			std::unique_ptr<float> m_targetAng = nullptr;
			bool IsFacingTarget(pragma::SAIComponent &ent, const Vector3 &pos) const;
		  public:
			TaskTurnToTarget();
			TaskTurnToTarget(const TaskTurnToTarget &other);
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
		};
	};
};

#endif
