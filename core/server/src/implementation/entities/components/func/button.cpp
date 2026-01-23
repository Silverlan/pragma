// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.func.button;

import :game;
import :server_state;

using namespace pragma;

void SButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();

	BindEvent(usableComponent::EVENT_CAN_USE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData &>(evData.get()).canUse;
		bCanUse = (m_tNextUse - SGame::Get()->CurTime()) <= 0.f;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent<UsableComponent>();
}

////////////

void SButtonComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncButton::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SButtonComponent>();
}
