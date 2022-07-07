/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_BLOOM_COMPONENT_HPP__
#define __C_RENDERER_PP_BLOOM_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"

namespace pragma
{
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpBloomComponent final
		: public CRendererPpBaseComponent
	{
	public:
		CRendererPpBloomComponent(BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override {return "bloom";}
		virtual uint32_t GetPostProcessingWeight() const override {return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Bloom);}
	private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
	};
};

#endif
