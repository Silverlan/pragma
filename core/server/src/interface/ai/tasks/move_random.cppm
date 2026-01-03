// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.move_random;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskMoveRandom : public BehaviorNode {
		  protected:
			Vector3 m_moveTarget;
		  public:
			enum class Parameter : uint32_t { Distance = 0u, MoveActivity };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Think(const Schedule *sched, BaseAIComponent &ent) override;
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			float GetMoveDistance(const Schedule *sched) const;
			void SetMoveDistance(float dist);
			void SetMoveActivity(Activity act);
		};
	};
};
