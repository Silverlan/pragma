// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"




export module pragma.client:entities.components.pp_tone_mapping;
import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpToneMappingComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpToneMappingComponent(pragma::ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "tone_mapping"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::ToneMapping); }
		virtual PostProcessingEffectData::Flags GetFlags() const override;

		void SetApplyToHdrImage(bool applyToHdrImage);
	  private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		bool m_applyToHdrImage = false;
	};
};
