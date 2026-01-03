// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.move_to_target;

import :ai.tasks.target;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskMoveToTarget : public TaskTarget {
		  public:
			enum class Parameter : uint32_t { Target = math::to_integral(TaskTarget::Parameter::Target), Distance, MoveActivity };

			using TaskTarget::TaskTarget;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetMoveDistance(float dist);
			void SetMoveActivity(Activity act);
		};
	};
};
