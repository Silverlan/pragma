/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_BASE_COMPONENT_HPP__
#define __C_RENDERER_PP_BASE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace util {
	struct DrawSceneInfo;
};
namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpBaseComponent : public BaseEntityComponent {
	  public:
		CRendererPpBaseComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual std::string GetIdentifier() const = 0;
		virtual uint32_t GetPostProcessingWeight() const = 0;
		virtual PostProcessingEffectData::Flags GetFlags() const { return PostProcessingEffectData::Flags::None; }
	  protected:
		void RenderEffect(const util::DrawSceneInfo &drawSceneInfo);
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) = 0;
		pragma::ComponentHandle<CRasterizationRendererComponent> m_renderer {};
		CallbackHandle m_cbEffect;
	};
};

#endif
