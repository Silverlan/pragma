/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_SCORE_COMPONENT_HPP__
#define __BASE_SCORE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseScoreComponent
		: public BaseEntityComponent
	{
	public:
		static pragma::ComponentEventId EVENT_ON_SCORE_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		using Score = int32_t;
		virtual void Initialize() override;
		virtual ~BaseScoreComponent() override;

		Score GetScore() const;
		const util::PInt32Property &GetScoreProperty() const;
		void SetScore(Score score);
		void AddScore(Score score);
		void SubtractScore(Score score);
	protected:
		BaseScoreComponent(BaseEntity &ent);
		util::PInt32Property m_score;
		pragma::NetEventId m_netEvSetScore = pragma::INVALID_NET_EVENT;
		CallbackHandle m_cbOnScoreChanged = {};
	};
	struct DLLNETWORK CEOnScoreChanged
		: public ComponentEvent
	{
		CEOnScoreChanged(BaseScoreComponent::Score score);
		virtual void PushArguments(lua_State *l) override;
		BaseScoreComponent::Score score;
	};
};

#endif
