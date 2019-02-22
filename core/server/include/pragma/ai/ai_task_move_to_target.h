#ifndef __AI_TASK_MOVE_TO_TARGET_H__
#define __AI_TASK_MOVE_TO_TARGET_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_task_target.h"

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskMoveToTarget
			: public TaskTarget
		{
		public:
			enum class Parameter : uint32_t
			{
				Target = umath::to_integral(TaskTarget::Parameter::Target),
				Distance,
				MoveActivity
			};

			using TaskTarget::TaskTarget;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual Result Think(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;

			void SetMoveDistance(float dist);
			void SetMoveActivity(Activity act);
		};
	};
};

#endif
