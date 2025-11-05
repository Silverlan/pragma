// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"

export module pragma.server:ai.tasks.event;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskEvent : public ai::BehaviorNode {
		  public:
			enum class Parameter : uint32_t { EventId = 0, EventArgStart };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetEventId(AnimationEvent::Type eventId);
			void SetEventArgument(uint32_t argIdx, const std::string &arg);
		};
	};
};
