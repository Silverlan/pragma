// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.play_animation;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskPlayAnimationBase : public BehaviorNode {
		  protected:
			int32_t m_animation;
			Result m_resultState;
			BaseAIComponent *m_targetNpc = nullptr;
			CallbackHandle m_cbOnAnimationComplete;
			CallbackHandle m_cbOnPlayAnimation;
			virtual int32_t SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId = 0);
			virtual int32_t GetAnimation(BaseAIComponent &ent) const = 0;
			virtual void PlayAnimation(BaseAIComponent &ent) = 0;
			bool StartTask(const Schedule *sched, BaseAIComponent &ent);
			virtual void OnStopped() override;
			virtual void Clear() override;
			void UnlockAnimation();
		  public:
			TaskPlayAnimationBase(const TaskPlayAnimationBase &) = default;
			TaskPlayAnimationBase(SelectorType selectorType = SelectorType::Sequential);
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
		};

		class DLLSERVER TaskPlayAnimation : public TaskPlayAnimationBase {
		  protected:
			bool GetFaceTarget(const Schedule *sched, BaseAIComponent &ent, Vector3 &tgt) const;
			virtual int32_t GetAnimation(BaseAIComponent &ent) const override;
			virtual void PlayAnimation(BaseAIComponent &ent) override;
		  public:
			enum class Parameter : uint32_t { Animation = 0, FaceTarget };
			TaskPlayAnimation(const TaskPlayAnimation &) = default;
			TaskPlayAnimation(SelectorType selectorType = SelectorType::Sequential);
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }

			void SetAnimation(int32_t animation);
			void SetAnimation(const std::string &animation);
			void SetFaceTarget(const Vector3 &target);
			void SetFaceTarget(ecs::BaseEntity &ent);
			void SetFacePrimaryTarget();
		};

		class DLLSERVER TaskPlayLayeredAnimation : public TaskPlayAnimationBase {
		  protected:
			int32_t m_slot = 0;
			virtual int32_t GetAnimation(BaseAIComponent &ent) const override;
			virtual void PlayAnimation(BaseAIComponent &ent) override;
		  public:
			enum class Parameter : uint32_t { Animation = 0, AnimationSlot = 2 };
			TaskPlayLayeredAnimation(const TaskPlayLayeredAnimation &) = default;
			TaskPlayLayeredAnimation(SelectorType selectorType = SelectorType::Sequential);
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }

			void SetAnimation(int32_t animation);
			void SetAnimation(const std::string &animation);
			void SetAnimationSlot(int32_t animationSlot);
		};
	};
};
