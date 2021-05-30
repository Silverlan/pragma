/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIDEBUGHDRBLOOM_HPP__
#define __WIDEBUGHDRBLOOM_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugHDRBloom
	: public WITexturedRect
{
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