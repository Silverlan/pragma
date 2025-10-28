// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include "pragma/lua/core.hpp"

#include <string>
#include <utility>

module pragma.shared;

import :entities.components.base_score;

using namespace pragma;

ComponentEventId BaseScoreComponent::EVENT_ON_SCORE_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseScoreComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_SCORE_CHANGED = registerEvent("ON_SCORE_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseScoreComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseScoreComponent;
	{
		auto memberInfo = create_component_member_info<T, Score, static_cast<void (T::*)(Score)>(&T::SetScore), static_cast<Score (T::*)() const>(&T::GetScore)>("score", 0);
		registerMember(std::move(memberInfo));
	}
}
BaseScoreComponent::BaseScoreComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_score(util::Int32Property::Create(0)) {}
BaseScoreComponent::~BaseScoreComponent()
{
	if(m_cbOnScoreChanged.IsValid())
		m_cbOnScoreChanged.Remove();
}
void BaseScoreComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "score", false))
			*m_score = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setscore", false))
			*m_score = util::to_int(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnScoreChanged = m_score->AddCallback([this](std::reference_wrapper<const Score> oldScore, std::reference_wrapper<const Score> newScore) {
		pragma::CEOnScoreChanged onScoreChanged {newScore.get()};
		BroadcastEvent(EVENT_ON_SCORE_CHANGED, onScoreChanged);
	});
	m_netEvSetScore = SetupNetEvent("set_score");
}
BaseScoreComponent::Score BaseScoreComponent::GetScore() const { return *m_score; }
const util::PInt32Property &BaseScoreComponent::GetScoreProperty() const { return m_score; }
void BaseScoreComponent::SetScore(Score score) { *m_score = score; }
void BaseScoreComponent::AddScore(Score score) { SetScore(GetScore() + score); }
void BaseScoreComponent::SubtractScore(Score score) { SetScore(GetScore() - score); }

/////////////////

CEOnScoreChanged::CEOnScoreChanged(BaseScoreComponent::Score score) : score {score} {}
void CEOnScoreChanged::PushArguments(lua_State *l) { Lua::PushInt(l, score); }
