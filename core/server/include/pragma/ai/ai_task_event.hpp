/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_TASK_EVENT_HPP__
#define __AI_TASK_EVENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"
#include <pragma/model/animation/animation_event.h>

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskEvent
			: public ai::BehaviorNode
		{
		public:
			enum class Parameter : uint32_t
			{
				EventId = 0,
				EventArgStart
			};
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual ai::BehaviorNode::Result Start(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;

			void SetEventId(AnimationEvent::Type eventId);
			void SetEventArgument(uint32_t argIdx,const std::string &arg);
		};
	};
};

#endif
