// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.play_activity;

import :ai.tasks.play_animation;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskPlayActivity : public TaskPlayAnimation {
		  protected:
			virtual int32_t SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId = 0) override;
		  public:
			enum class Parameter : uint32_t { Activity = math::to_integral(TaskPlayAnimation::Parameter::Animation), FaceTarget = math::to_integral(TaskPlayAnimation::Parameter::FaceTarget) };
			using TaskPlayAnimation::TaskPlayAnimation;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetActivity(Activity activity);
			void SetFaceTarget(const Vector3 &target);
			void SetFaceTarget(ecs::BaseEntity &target);
		};

		class DLLSERVER TaskPlayLayeredActivity : public TaskPlayLayeredAnimation {
		  protected:
			virtual int32_t SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId = 0) override;
		  public:
			enum class Parameter : uint32_t { Activity = math::to_integral(TaskPlayLayeredAnimation::Parameter::Animation), AnimationSlot = math::to_integral(TaskPlayLayeredAnimation::Parameter::AnimationSlot) };
			using TaskPlayLayeredAnimation::TaskPlayLayeredAnimation;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetActivity(Activity activity);
			void SetAnimationSlot(int32_t animationSlot);
		};
	};
};
