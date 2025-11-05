// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:gui.debug_shadow_map;

export import :entities.components.lights.light;
export import pragma.gui;

export {
	class DLLCLIENT WIDebugShadowMap : public WIBase {
	  public:
		WIDebugShadowMap();

		void SetLightSource(pragma::CLightComponent &lightSource);
		void SetShadowMapSize(uint32_t w, uint32_t h);

		void SetContrastFactor(float contrastFactor);
		float GetContrastFactor() const;

		void SetShadowMapType(pragma::rendering::ShadowMapType type);
	  private:
		virtual void DoUpdate() override;
		std::vector<WIHandle> m_shadowMapImages = {};
		util::TWeakSharedHandle<pragma::CLightComponent> m_lightHandle = {};
		Vector2i m_shadowMapSize = {256, 256};
		float m_contrastFactor = 1.f;
		pragma::rendering::ShadowMapType m_shadowMapType = pragma::rendering::ShadowMapType::Static;
	};
};
