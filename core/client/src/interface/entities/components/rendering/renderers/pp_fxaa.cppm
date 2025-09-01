// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/renderers/c_renderer_component.hpp"

export module pragma.client.entities.components:pp_fxaa;

import :pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpFxaaComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpFxaaComponent(BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "fxaa"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Fxaa); }
		virtual PostProcessingEffectData::Flags GetFlags() const override { return PostProcessingEffectData::Flags::ToneMapped; }
	  private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
	};
};
