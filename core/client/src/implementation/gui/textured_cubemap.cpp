// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;

import :gui.textured_cubemap;

WITexturedCubemap::WITexturedCubemap() : WIBase {} {}

void WITexturedCubemap::Initialize()
{
	WIBase::Initialize();
	SetSize(512, 384);

	auto w = GetWidth();
	auto h = GetHeight();
	auto &wgui = WGUI::GetInstance();
	uint8_t faceIdx = 0u;
	for(auto &hSide : m_hCubemapSides) {
		auto *p = wgui.Create<WITexturedRect>(this);
		p->SetSize(128, 128);
		switch(static_cast<CubeMapSide>(faceIdx)) {
		case CubeMapSide::Right:
			p->SetPos(0, 128);
			break;
		case CubeMapSide::Top:
			p->SetPos(128, 0);
			break;
		case CubeMapSide::Front:
			p->SetPos(128, 128);
			break;
		case CubeMapSide::Bottom:
			p->SetPos(128, 256);
			break;
		case CubeMapSide::Left:
			p->SetPos(256, 128);
			break;
		case CubeMapSide::Back:
			p->SetPos(384, 128);
			break;
		}
		p->SetAnchor(p->GetLeft() / static_cast<float>(w), p->GetTop() / static_cast<float>(h), p->GetRight() / static_cast<float>(w), p->GetBottom() / static_cast<float>(h));
		hSide = p->GetHandle();
		++faceIdx;
	}
}

void WITexturedCubemap::SetLOD(float lod)
{
	for(auto &hSide : m_hCubemapSides)
		static_cast<WITexturedRect *>(hSide.get())->SetLOD(lod);
}

void WITexturedCubemap::SetTexture(prosper::Texture &tex)
{
	if(tex.GetImage().IsCubemap() == false)
		return;
	for(auto faceIdx = decltype(m_hCubemapSides.size()) {0u}; faceIdx < m_hCubemapSides.size(); ++faceIdx) {
		auto &hEl = m_hCubemapSides.at(faceIdx);
		if(hEl.IsValid() == false)
			continue;
		auto *pEl = static_cast<WITexturedRect *>(hEl.get());
		pEl->SetTexture(tex, faceIdx);
	}
}
WITexturedRect *WITexturedCubemap::GetSideElement(CubeMapSide side) { return static_cast<WITexturedRect *>(m_hCubemapSides.at(umath::to_integral(side)).get()); }
