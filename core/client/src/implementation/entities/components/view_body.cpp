// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"


module pragma.client;


import :entities.components.view_body;
import :client_state;
import :entities.components.render;
import :entities.components.transform;

using namespace pragma;


void CViewBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<CTransformComponent>();
	ent.AddComponent<CModelComponent>();
	ent.AddComponent<LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->AddToRenderGroup("firstperson");
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::View);
		pRenderComponent->SetCastShadows(false);
	}
	ent.AddComponent<CAnimatedComponent>();
}
void CViewBodyComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

void CViewBody::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewBodyComponent>();
}
