/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WITEXTUREDCUBEMAP_HPP__
#define __WITEXTUREDCUBEMAP_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_cubemapside.h"
#include <wgui/wibase.h>

namespace prosper {class Texture;};
class WITexturedRect;
class DLLCLIENT WITexturedCubemap
	: public WIBase
{
public:
	WITexturedCubemap();
	virtual void Initialize() override;
	void SetTexture(prosper::Texture &tex);
	WITexturedRect *GetSideElement(CubeMapSide side);
	void SetLOD(float lod);
protected:
	std::array<WIHandle,6> m_hCubemapSides = {};
};

#endif
