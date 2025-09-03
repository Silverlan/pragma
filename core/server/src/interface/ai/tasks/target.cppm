// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

export module pragma.server.ai.tasks.target;

export namespace pragma {
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
