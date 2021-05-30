/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_TASK_DECORATOR_H__
#define __AI_TASK_DECORATOR_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskDecorator
			: public ai::BehaviorNode
		{
		public:
			enum class DLLSERVER DecoratorType : uint32_t
			{
				Inherit = 0,
				AlwaysFail,
				AlwaysSucceed,
				Invert,
				Limit,
				Repeat,
				UntilFail,
				UntilSuccess
			};
		protected:
			bool m_bRestartTaskOnThink = false;
			DecoratorType m_decoratorType = DecoratorType::Inherit;
			bool m_bInitialized = false;
			uint32_t m_count = 0;
			uint32_t m_limit = 0;
			virtual void OnParameterChanged(uint8_t paramId) override;
			void Initialize(const Schedule *sched);
		public:
			enum class Parameter : uint32_t
			{
				DecoratorType = 0,
				Limit
			};
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual Result Think(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual ai::BehaviorNode::Result Start(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Stop() override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;

			void SetDecoratorType(DecoratorType decoratorType);
			void SetLimit(uint32_t limit);
		};
	};
};

#endif
