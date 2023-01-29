/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIDEBUGMSAATEXTURE_HPP__
#define __WIDEBUGMSAATEXTURE_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugMSAATexture : public WIBase {
  public:
	WIDebugMSAATexture();
	virtual void Initialize() override;
	void SetTexture(prosper::Texture &texture);

	// If set to false, all sub-images contained in the multi-sampled image will be displayed
	void SetShouldResolveImage(bool b);
  protected:
	virtual void DoUpdate() override;
	void UpdateResolvedTexture();
	WIHandle m_hTextureRect;

	std::shared_ptr<prosper::Texture> m_msaaTexture = nullptr;
	std::shared_ptr<prosper::Texture> m_resolvedTexture = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupImg = nullptr;

	bool m_bShouldResolveImage = true;
};

#endif
