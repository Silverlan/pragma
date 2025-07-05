// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __AI_TASK_PLAY_ACTIVITY_H__
#define __AI_TASK_PLAY_ACTIVITY_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_task_play_animation.h"
#include <pragma/model/animation/activities.h>

namespace pragma {
	namespace ai {
		class DLLSERVER TaskPlayActivity : public TaskPlayAnimation {
		  protected:
			virtual int32_t SelectAnimation(const Schedule *sched, pragma::SAIComponent &ent, uint8_t paramId = 0) override;
		  public:
			enum class Parameter : uint32_t { Activity = umath::to_integral(TaskPlayAnimation::Parameter::Animation), FaceTarget = umath::to_integral(TaskPlayAnimation::Parameter::FaceTarget) };
			using TaskPlayAnimation::TaskPlayAnimation;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetActivity(Activity activity);
			void SetFaceTarget(const Vector3 &target);
			void SetFaceTarget(BaseEntity &target);
		};

		class DLLSERVER TaskPlayLayeredActivity : public TaskPlayLayeredAnimation {
		  protected:
			virtual int32_t SelectAnimation(const Schedule *sched, pragma::SAIComponent &ent, uint8_t paramId = 0) override;
		  public:
			enum class Parameter : uint32_t { Activity = umath::to_integral(TaskPlayLayeredAnimation::Parameter::Animation), AnimationSlot = umath::to_integral(TaskPlayLayeredAnimation::Parameter::AnimationSlot) };
			using TaskPlayLayeredAnimation::TaskPlayLayeredAnimation;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetActivity(Activity activity);
			void SetAnimationSlot(int32_t animationSlot);
		};
	};
};

#endif
