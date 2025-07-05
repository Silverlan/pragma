// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WIDEBUGHDRBLOOM_HPP__
#define __WIDEBUGHDRBLOOM_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugHDRBloom : public WITexturedRect {
  public:
	WIDebugHDRBloom();
	virtual ~WIDebugHDRBloom() override;
  private:
	virtual void DoUpdate() override;
	void UpdateBloomImage();
	CallbackHandle m_cbRenderHDRMap = {};
	std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
};

#endif
