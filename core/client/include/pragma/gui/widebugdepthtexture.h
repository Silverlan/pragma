/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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

	void SetTexture(prosper::Texture &texture,bool stencil=false);
	void SetTexture(prosper::Texture &texture,prosper::util::BarrierImageLayout srcLayout,prosper::util::BarrierImageLayout dstLayout,uint32_t layerId=0u,bool stencil=false);

	void SetContrastFactor(float contrastFactor);
	float GetContrastFactor() const;

	void Setup(float nearZ,float farZ);
	void SetShouldResolveImage(bool b);
protected:
	virtual void DoUpdate() override;
	void UpdateResolvedTexture();
	WIHandle m_hTextureRect;

	util::WeakHandle<prosper::Shader> m_whDepthToRgbShader = {};
	util::WeakHandle<prosper::Shader> m_whCubeDepthToRgbShader = {};
	util::WeakHandle<prosper::Shader> m_whCsmDepthToRgbShader = {};
	CallbackHandle m_depthToRgbCallback = {};
	std::shared_ptr<prosper::Texture> m_srcDepthTex = nullptr;
	std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
	prosper::util::BarrierImageLayout m_srcBarrierImageLayout = {};
	prosper::util::BarrierImageLayout m_dstBarrierImageLayout = {};
	std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgSceneDepthTex = nullptr;
	uint32_t m_imageLayer = 0u;
	float m_contrastFactor = 1.f;
	
	bool m_bResolveImage = true;
};

#endif
