// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.info.landmark;

using namespace pragma;

void SInfoLandmarkComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void InfoLandmark::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SInfoLandmarkComponent>();
}
