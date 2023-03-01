/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __TASK_TARGET_H__
#define __TASK_TARGET_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

namespace pragma {
	namespace ai {
		class DLLSERVER TaskTarget : public ai::BehaviorNode {
		  protected:
			using BehaviorNode::BehaviorNode;
			const BaseEntity *GetTargetEntity(const Schedule *sched, pragma::SAIComponent &ent) const;
			bool GetTargetPosition(const Schedule *sched, pragma::SAIComponent &ent, Vector3 &pos) const;
		  public:
			enum class Parameter : uint32_t { Target = 0u };
			void SetTarget(const Vector3 &target);
			void SetTarget(const EntityHandle &hEnt);
		};
	};
};

#endif
