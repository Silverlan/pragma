// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_RENDERER_PP_FXAA_COMPONENT_HPP__
#define __C_RENDERER_PP_FXAA_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"

namespace pragma {
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

#endif
