// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.random;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskRandom : public BehaviorNode {
		  protected:
			double m_tFinished = 0.0;
		  public:
			enum class Parameter : uint32_t { Chance = 0 };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			float GetChance() const;
			void SetChance(float chance);
		};
	};
};
