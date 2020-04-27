/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __AI_TASK_WAIT_H__
#define __AI_TASK_WAIT_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskWait
			: public ai::BehaviorNode
		{
		protected:
			double m_tFinished = 0.0;
		public:
			enum class Parameter : uint32_t
			{
				MinWaitTime = 0,
				MaxWaitTime
			};
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual Result Think(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual ai::BehaviorNode::Result Start(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;

			void SetMinWaitTime(float t);
			void SetMaxWaitTime(float t);
			void SetWaitTime(float tMin,float tMax);
		};
	};
};

#endif
