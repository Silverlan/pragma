/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_TASK_DEBUG_H__
#define __AI_TASK_DEBUG_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

namespace pragma {
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

#endif
