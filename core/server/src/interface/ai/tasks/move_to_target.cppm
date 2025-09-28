// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.ai.tasks.move_to_target;

import pragma.server.ai.tasks.target;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskMoveToTarget : public TaskTarget {
		  public:
			enum class Parameter : uint32_t { Target = umath::to_integral(TaskTarget::Parameter::Target), Distance, MoveActivity };

			using TaskTarget::TaskTarget;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, pragma::BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetMoveDistance(float dist);
			void SetMoveActivity(Activity act);
		};
	};
};
