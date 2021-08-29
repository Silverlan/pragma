/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_score_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

using namespace pragma;

ComponentEventId BaseScoreComponent::EVENT_ON_SCORE_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseScoreComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_SCORE_CHANGED = componentManager.RegisterEvent("ON_SCORE_CHANGED");
}
void BaseScoreComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,const std::function<ComponentMemberIndex(ComponentMemberInfo&&)> &registerMember)
{
	using T = BaseScoreComponent;
	registerMember(create_component_member_info<
		T,Score,
		static_cast<void(T::*)(Score)>(&T::SetScore),
		static_cast<Score(T::*)() const>(&T::GetScore)
	>("score"));
}
BaseScoreComponent::BaseScoreComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_score(util::Int32Property::Create(0))
{}
BaseScoreComponent::~BaseScoreComponent()
{
	if(m_cbOnScoreChanged.IsValid())
		m_cbOnScoreChanged.Remove();
}
void BaseScoreComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"score",false))
			*m_score = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setscore",false))
			*m_score = util::to_int(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnScoreChanged = m_score->AddCallback([this](std::reference_wrapper<const Score> oldScore,std::reference_wrapper<const Score> newScore) {
		pragma::CEOnScoreChanged onScoreChanged{newScore.get()};
		BroadcastEvent(EVENT_ON_SCORE_CHANGED,onScoreChanged);
	});
	m_netEvSetScore = SetupNetEvent("set_score");
}
BaseScoreComponent::Score BaseScoreComponent::GetScore() const {return *m_score;}
const util::PInt32Property &BaseScoreComponent::GetScoreProperty() const {return m_score;}
void BaseScoreComponent::SetScore(Score score) {*m_score = score;}
void BaseScoreComponent::AddScore(Score score) {SetScore(GetScore() +score);}
void BaseScoreComponent::SubtractScore(Score score) {SetScore(GetScore() -score);}

/////////////////

CEOnScoreChanged::CEOnScoreChanged(BaseScoreComponent::Score score)
	: score{score}
{}
void CEOnScoreChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l,score);
}
