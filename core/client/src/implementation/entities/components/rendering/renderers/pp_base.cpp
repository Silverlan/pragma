// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"
#include <prosper_command_buffer.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_base;
import :engine;


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
	pragma::get_cengine()->GetRenderContext().WaitIdle();
	BaseEntityComponent::OnRemove();
	if(m_cbEffect.IsValid())
		m_cbEffect.Remove();
}
void CRendererPpBaseComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
pragma::PostProcessingEffectData::Flags CRendererPpBaseComponent::GetFlags() const { return PostProcessingEffectData::Flags::None; }
void CRendererPpBaseComponent::RenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	DoRenderEffect(drawSceneInfo);
}
