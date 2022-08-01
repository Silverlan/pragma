/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_TONE_MAPPING_COMPONENT_HPP__
#define __C_RENDERER_PP_TONE_MAPPING_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"

namespace pragma
{
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpToneMappingComponent final
		: public CRendererPpBaseComponent
	{
	public:
		CRendererPpToneMappingComponent(BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override {return "tone_mapping";}
		virtual uint32_t GetPostProcessingWeight() const override {return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::ToneMapping);}
		virtual PostProcessingEffectData::Flags GetFlags() const override;

		void SetApplyToHdrImage(bool applyToHdrImage);
	private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		bool m_applyToHdrImage = false;
	};
};

#endif
