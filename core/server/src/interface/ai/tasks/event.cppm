// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.event;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskEvent : public BehaviorNode {
		  public:
			enum class Parameter : uint32_t { EventId = 0, EventArgStart };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetEventId(AnimationEvent::Type eventId);
			void SetEventArgument(uint32_t argIdx, const std::string &arg);
		};
	};
};
