#ifndef __AI_TASK_MOVE_RANDOM_H__
#define __AI_TASK_MOVE_RANDOM_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"
#include <pragma/model/animation/activities.h>

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskMoveRandom
			: public ai::BehaviorNode
		{
		protected:
			Vector3 m_moveTarget;
		public:
			enum class Parameter : uint32_t
			{
				Distance = 0u,
				MoveActivity
			};
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual Result Think(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual ai::BehaviorNode::Result Start(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;

			float GetMoveDistance(const Schedule *sched) const;
			void SetMoveDistance(float dist);
			void SetMoveActivity(Activity act);
		};
	};
};

#endif
