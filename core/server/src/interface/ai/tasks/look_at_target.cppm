// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"

export module pragma.server.ai.tasks.look_at_target;

import pragma.server.ai.tasks.target;

export namespace pragma {
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
