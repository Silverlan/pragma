// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>
#include <wgui/wibase.h>

export module pragma.client:gui.textured_cubemap;

import :rendering.cube_map_side;

export {
	class DLLCLIENT WITexturedCubemap : public WIBase {
	public:
		WITexturedCubemap();
		virtual void Initialize() override;
		void SetTexture(prosper::Texture &tex);
		WITexturedRect *GetSideElement(CubeMapSide side);
		void SetLOD(float lod);
	protected:
		std::array<WIHandle, 6> m_hCubemapSides = {};
	};
};
