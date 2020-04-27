/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIDEBUGMIPMAPS_H__
#define __WIDEBUGMIPMAPS_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

namespace prosper {class Texture;};
class DLLCLIENT WIDebugMipMaps
	: public WIBase
{
protected:
	std::vector<WIHandle> m_hTextures;
	std::vector<std::shared_ptr<prosper::Texture>> m_textures;
public:
	WIDebugMipMaps();
	virtual void Initialize() override;
	void SetTexture(const std::shared_ptr<prosper::Texture> &texture);
};

#endif
