#ifndef __WIDEBUGSHADOWMAP_HPP__
#define __WIDEBUGSHADOWMAP_HPP__

#include "pragma/entities/environment/lights/c_env_light.h"
#include "wgui/wibase.h"

class DLLCLIENT WIDebugShadowMap
	: public WIBase
{
public:
	WIDebugShadowMap();

	void SetLightSource(pragma::CLightComponent &lightSource);
	void SetShadowMapSize(uint32_t w,uint32_t h);

	void SetContrastFactor(float contrastFactor);
	float GetContrastFactor() const;

	void SetShadowMapType(pragma::CLightComponent::ShadowMapType type);
private:
	virtual void DoUpdate() override;
	std::vector<WIHandle> m_shadowMapImages = {};
	util::WeakHandle<pragma::CLightComponent> m_lightHandle = {};
	Vector2i m_shadowMapSize = {256,256};
	float m_contrastFactor = 1.f;
	pragma::CLightComponent::ShadowMapType m_shadowMapType = pragma::CLightComponent::ShadowMapType::Static;
};

#endif