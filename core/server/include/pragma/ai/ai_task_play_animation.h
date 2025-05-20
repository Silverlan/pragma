/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_TASK_PLAY_ANIMATION_H__
#define __AI_TASK_PLAY_ANIMATION_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"
#include <sharedutils/functioncallback.h>

namespace pragma {
	namespace ai {
		class DLLSERVER TaskPlayAnimationBase : public ai::BehaviorNode {
		  protected:
			int32_t m_animation;
			Result m_resultState;
			pragma::SAIComponent *m_targetNpc = nullptr;
			CallbackHandle m_cbOnAnimationComplete;
			CallbackHandle m_cbOnPlayAnimation;
			virtual int32_t SelectAnimation(const Schedule *sched, pragma::SAIComponent &ent, uint8_t paramId = 0);
			virtual int32_t GetAnimation(pragma::SAIComponent &ent) const = 0;
			virtual void PlayAnimation(pragma::SAIComponent &ent) = 0;
			bool StartTask(const Schedule *sched, pragma::SAIComponent &ent);
			virtual void OnStopped() override;
			virtual void Clear() override;
			void UnlockAnimation();
		  public:
			TaskPlayAnimationBase(const TaskPlayAnimationBase &) = default;
			TaskPlayAnimationBase(SelectorType selectorType = SelectorType::Sequential);
			virtual Result Think(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
		};

		class DLLSERVER TaskPlayAnimation : public TaskPlayAnimationBase {
		  protected:
			bool GetFaceTarget(const Schedule *sched, pragma::SAIComponent &ent, Vector3 &tgt) const;
			virtual int32_t GetAnimation(pragma::SAIComponent &ent) const override;
			virtual void PlayAnimation(pragma::SAIComponent &ent) override;
		  public:
			enum class Parameter : uint32_t { Animation = 0, FaceTarget };
			TaskPlayAnimation(const TaskPlayAnimation &) = default;
			TaskPlayAnimation(SelectorType selectorType = SelectorType::Sequential);
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual Result Think(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }

			void SetAnimation(int32_t animation);
			void SetAnimation(const std::string &animation);
			void SetFaceTarget(const Vector3 &target);
			void SetFaceTarget(BaseEntity &ent);
			void SetFacePrimaryTarget();
		};

		class DLLSERVER TaskPlayLayeredAnimation : public TaskPlayAnimationBase {
		  protected:
			int32_t m_slot = 0;
			virtual int32_t GetAnimation(pragma::SAIComponent &ent) const override;
			virtual void PlayAnimation(pragma::SAIComponent &ent) override;
		  public:
			enum class Parameter : uint32_t { Animation = 0, AnimationSlot = 2 };
			TaskPlayLayeredAnimation(const TaskPlayLayeredAnimation &) = default;
			TaskPlayLayeredAnimation(SelectorType selectorType = SelectorType::Sequential);
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }

			void SetAnimation(int32_t animation);
			void SetAnimation(const std::string &animation);
			void SetAnimationSlot(int32_t animationSlot);
		};
	};
};

#endif
