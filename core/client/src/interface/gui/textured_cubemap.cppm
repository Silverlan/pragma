// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.textured_cubemap;

export import :rendering.cube_map_side;
export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WITexturedCubemap : public WIBase {
	  public:
		WITexturedCubemap();
		virtual void Initialize() override;
		void SetTexture(prosper::Texture &tex);
		WITexturedRect *GetSideElement(rendering::CubeMapSide side);
		void SetLOD(float lod);
	  protected:
		std::array<WIHandle, 6> m_hCubemapSides = {};
	};
};
