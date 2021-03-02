/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_SCHEDULE_H__
#define __AI_SCHEDULE_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"
#include <vector>

namespace pragma
{
	class SAIComponent;
	namespace ai
	{
		class DLLSERVER Schedule
			: public std::enable_shared_from_this<Schedule>,
			public ParameterBase
		{
		public:
			using Parameter = BehaviorParameter;
		protected:
			Schedule();
			mutable std::shared_ptr<BehaviorNode> m_rootTask;
			uint32_t m_interruptFlags = 0;
		public:
			Schedule(const Schedule&)=delete;
			Schedule &operator=(const Schedule&)=delete;
			static std::shared_ptr<Schedule> Create();
			std::shared_ptr<Schedule> Copy() const;
			BehaviorNode &GetRootNode() const;
			ai::BehaviorNode::Result Start(pragma::SAIComponent &ent);
			void DebugPrint(std::stringstream &ss) const;
			void SetInterruptFlags(uint32_t f);
			void AddInterruptFlags(uint32_t f);
			uint32_t GetInterruptFlags() const;

			void Cancel() const;
			BehaviorNode::Result Think(pragma::SAIComponent &ent) const;
		};
	};
};

#endif
