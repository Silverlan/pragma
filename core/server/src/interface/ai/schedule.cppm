// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <vector>

export module pragma.server.ai.schedule;

export import :behavior;

namespace pragma {
	namespace ai {
		class DLLSERVER Schedule : public std::enable_shared_from_this<Schedule>, public ParameterBase {
		  public:
			using Parameter = BehaviorParameter;
		  protected:
			Schedule();
			mutable std::shared_ptr<BehaviorNode> m_rootTask;
			uint32_t m_interruptFlags = 0;
		  public:
			Schedule(const Schedule &) = delete;
			Schedule &operator=(const Schedule &) = delete;
			static std::shared_ptr<Schedule> Create();
			std::shared_ptr<Schedule> Copy() const;
			BehaviorNode &GetRootNode() const;
			ai::BehaviorNode::Result Start(pragma::BaseAIComponent &ent);
			void DebugPrint(std::stringstream &ss) const;
			void SetInterruptFlags(uint32_t f);
			void AddInterruptFlags(uint32_t f);
			uint32_t GetInterruptFlags() const;

			void Cancel() const;
			BehaviorNode::Result Think(pragma::BaseAIComponent &ent) const;
		};
	};
};
