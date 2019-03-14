#ifndef __WIDEBUGDEPTHTEXTURE_H__
#define __WIDEBUGDEPTHTEXTURE_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>
#include <prosper_util.hpp>

class DLLCLIENT WIDebugDepthTexture
	: public WIBase
{
public:
	WIDebugDepthTexture();
	virtual ~WIDebugDepthTexture() override;
	virtual void Initialize() override;
	virtual void Update() override;

	void SetTexture(prosper::Texture &texture);
	void SetTexture(prosper::Texture &texture,prosper::util::BarrierImageLayout srcLayout,prosper::util::BarrierImageLayout dstLayout,uint32_t layerId=0u);

	void SetContrastFactor(float contrastFactor);
	float GetContrastFactor() const;

	void Update(float nearZ,float farZ);
	void SetShouldResolveImage(bool b);
protected:
	void UpdateResolvedTexture();
	WIHandle m_hTextureRect;

	util::WeakHandle<prosper::Shader> m_whDepthToRgbShader = {};
	util::WeakHandle<prosper::Shader> m_whCubeDepthToRgbShader = {};
	util::WeakHandle<prosper::Shader> m_whCsmDepthToRgbShader = {};
	CallbackHandle m_depthToRgbCallback = {};
	std::shared_ptr<prosper::RenderTarget> m_srcDepthRenderTarget = nullptr;
	std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
	prosper::util::BarrierImageLayout m_srcBarrierImageLayout = {};
	prosper::util::BarrierImageLayout m_dstBarrierImageLayout = {};
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupDepthTex = nullptr;
	//Vulkan::TextureHandle m_hTexture; // prosper TODO
	//Vulkan::RenderTarget m_renderTarget; // prosper TODO
	uint32_t m_imageLayer = 0u;
	float m_contrastFactor = 1.f;
	//Vulkan::DescriptorSet m_descTexture; // prosper TODO
	
	bool m_bResolveImage = true;
	//Vulkan::Texture m_singleSampleTexture = nullptr; // prosper TODO
	//Vulkan::Texture GetSamplerTexture(); // prosper TODO
};

#endif
