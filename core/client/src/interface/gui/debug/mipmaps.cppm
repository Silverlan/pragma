// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

export module pragma.client.gui:debug_mipmaps;

export {
	class DLLCLIENT WIDebugMipMaps : public WIBase {
	protected:
		std::vector<WIHandle> m_hTextures;
		std::vector<std::shared_ptr<prosper::Texture>> m_textures;
	public:
		WIDebugMipMaps();
		virtual void Initialize() override;
		void SetTexture(const std::shared_ptr<prosper::Texture> &texture);
	};
};
