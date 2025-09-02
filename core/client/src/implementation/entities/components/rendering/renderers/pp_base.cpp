// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

module pragma.client.entities.components;

import :pp_base;
import pragma.client.entities.components.rasterization_renderer;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

CRendererPpBaseComponent::CRendererPpBaseComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void CRendererPpBaseComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_renderer = GetEntity().GetComponent<CRasterizationRendererComponent>();

	auto rendererC = GetEntity().GetComponent<CRendererComponent>();
	if(rendererC.expired())
		return;
	m_cbEffect = rendererC->AddPostProcessingEffect(GetIdentifier(), [this](const util::DrawSceneInfo &drawSceneInfo) { RenderEffect(drawSceneInfo); }, GetPostProcessingWeight(), [this]() { return GetFlags(); });
}
void CRendererPpBaseComponent::OnRemove()
{
	c_engine->GetRenderContext().WaitIdle();
	BaseEntityComponent::OnRemove();
	if(m_cbEffect.IsValid())
		m_cbEffect.Remove();
}
void CRendererPpBaseComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRendererPpBaseComponent::RenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	DoRenderEffect(drawSceneInfo);
}
