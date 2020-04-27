/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIDEBUGGLOWBLOOM_HPP__
#define __WIDEBUGGLOWBLOOM_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugGlowBloom
	: public WITexturedRect
{
public:
	WIDebugGlowBloom();
	virtual ~WIDebugGlowBloom() override;
private:
	virtual void DoUpdate() override;
	void UpdateBloomImage();
	CallbackHandle m_cbRenderHDRMap = {};
	std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
};

#endif