// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_score;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	namespace baseScoreComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_SCORE_CHANGED)
	}
	class DLLNETWORK BaseScoreComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		using Score = int32_t;
		virtual void Initialize() override;
		virtual ~BaseScoreComponent() override;

		Score GetScore() const;
		const util::PInt32Property &GetScoreProperty() const;
		void SetScore(Score score);
		void AddScore(Score score);
		void SubtractScore(Score score);
	  protected:
		BaseScoreComponent(ecs::BaseEntity &ent);
		util::PInt32Property m_score;
		NetEventId m_netEvSetScore = INVALID_NET_EVENT;
		CallbackHandle m_cbOnScoreChanged = {};
	};
	struct DLLNETWORK CEOnScoreChanged : public ComponentEvent {
		CEOnScoreChanged(BaseScoreComponent::Score score);
		virtual void PushArguments(lua::State *l) override;
		BaseScoreComponent::Score score;
	};
};
