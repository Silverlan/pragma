// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.raytracing_renderer;
import :client_state;
import :engine;

using namespace pragma;

void CRaytracingRendererComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CRaytracingRenderer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRaytracingRendererComponent>();
}
