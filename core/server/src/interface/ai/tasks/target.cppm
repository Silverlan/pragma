// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.target;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskTarget : public BehaviorNode {
		  protected:
			using BehaviorNode::BehaviorNode;
			const ecs::BaseEntity *GetTargetEntity(const Schedule *sched, BaseAIComponent &ent) const;
			bool GetTargetPosition(const Schedule *sched, BaseAIComponent &ent, Vector3 &pos) const;
		  public:
			enum class Parameter : uint32_t { Target = 0u };
			void SetTarget(const Vector3 &target);
			void SetTarget(const EntityHandle &hEnt);
		};
	};
};
