#ifndef __WIDEBUGSHADOWMAP_HPP__
#define __WIDEBUGSHADOWMAP_HPP__

#include "pragma/clientdefinitions.h"
#include "wgui/wibase.h"

class DLLCLIENT WIDebugShadowMap
	: public WIBase
{
public:
	WIDebugShadowMap();
	virtual void Update() override;

	void SetLightSource(pragma::CLightComponent &lightSource);
	void SetShadowMapSize(uint32_t w,uint32_t h);

	void SetContrastFactor(float contrastFactor);
	float GetContrastFactor() const;
private:
	std::vector<WIHandle> m_shadowMapImages = {};
	util::WeakHandle<pragma::CLightComponent> m_lightHandle = {};
	Vector2i m_shadowMapSize = {256,256};
	float m_contrastFactor = 1.f;
};

#endif