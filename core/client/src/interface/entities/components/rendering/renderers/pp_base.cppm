// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

export module pragma.client.entities.components.pp_base;

import pragma.client.entities.components.rasterization_renderer;
import pragma.client.entities.components.renderer;

export namespace pragma {
	class DLLCLIENT CRendererPpBaseComponent : public BaseEntityComponent {
	  public:
		CRendererPpBaseComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual std::string GetIdentifier() const = 0;
		virtual uint32_t GetPostProcessingWeight() const = 0;
		virtual PostProcessingEffectData::Flags GetFlags() const;
	  protected:
		void RenderEffect(const util::DrawSceneInfo &drawSceneInfo);
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) = 0;
		pragma::ComponentHandle<CRasterizationRendererComponent> m_renderer {};
		CallbackHandle m_cbEffect;
	};
};
