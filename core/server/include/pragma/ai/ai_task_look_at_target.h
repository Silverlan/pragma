// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __AI_TASK_LOOK_AT_TARGET_H__
#define __AI_TASK_LOOK_AT_TARGET_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_task_target.h"

namespace pragma {
	namespace ai {
		class DLLSERVER TaskLookAtTarget : public TaskTarget {
		  public:
			enum class Parameter : uint32_t { LookDuration = 1u };
			TaskLookAtTarget() = default;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetLookDuration(float dur);
		};
	};
};

#endif
