// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.look_at_target;

import :ai.tasks.target;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskLookAtTarget : public TaskTarget {
		  public:
			enum class Parameter : uint32_t { LookDuration = 1u };
			TaskLookAtTarget() = default;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetLookDuration(float dur);
		};
	};
};
