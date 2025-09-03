// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

export module pragma.server.ai.tasks.debug;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskDebugPrint : public ai::BehaviorNode {
		  public:
			enum class Parameter : uint32_t { Message = 0 };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			bool GetDebugMessage(const Schedule *sched, std::string &msg) const;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetMessage(const std::string &msg);
		};

		class DLLSERVER TaskDebugDrawText : public ai::BehaviorNode {
		  public:
			enum class Parameter : uint32_t { Message = 0 };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual ai::BehaviorNode::Result Start(const Schedule *sched, pragma::SAIComponent &ent) override;
			bool GetDebugMessage(const Schedule *sched, std::string &msg) const;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;

			void SetMessage(const std::string &msg);
		};
	};
};
