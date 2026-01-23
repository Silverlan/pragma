// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_base;
import :engine;

using namespace pragma;

CRendererPpBaseComponent::CRendererPpBaseComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void CRendererPpBaseComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_renderer = GetEntity().GetComponent<CRasterizationRendererComponent>();

	auto rendererC = GetEntity().GetComponent<CRendererComponent>();
	if(rendererC.expired())
		return;
	m_cbEffect = rendererC->AddPostProcessingEffect(GetIdentifier(), [this](const rendering::DrawSceneInfo &drawSceneInfo) { RenderEffect(drawSceneInfo); }, GetPostProcessingWeight(), [this]() { return GetFlags(); });
}
void CRendererPpBaseComponent::OnRemove()
{
	get_cengine()->GetRenderContext().WaitIdle();
	BaseEntityComponent::OnRemove();
	if(m_cbEffect.IsValid())
		m_cbEffect.Remove();
}
void CRendererPpBaseComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
PostProcessingEffectData::Flags CRendererPpBaseComponent::GetFlags() const { return PostProcessingEffectData::Flags::None; }
void CRendererPpBaseComponent::RenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	DoRenderEffect(drawSceneInfo);
}
