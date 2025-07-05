// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wiicon.h"
#include <sharedutils/util_string.h>

extern DLLCLIENT ClientState *client;
LINK_WGUI_TO_CLASS(WIIcon, WIIcon);
#define SILKICON_SPRITESHEET_WIDTH 1024
#define SILKICON_SPRITESHEET_HEIGHT 512
WIIcon::WIIcon() : WITexturedRect() {}

WIIcon::~WIIcon() {}

void WIIcon::Initialize() { WITexturedRect::Initialize(); }

void WIIcon::SetClipping(uint32_t xStart, uint32_t yStart, uint32_t width, uint32_t height)
{
	auto *mat = GetMaterial();
	if(mat == nullptr)
		return;
	auto *info = mat->GetDiffuseMap();
	if(info == nullptr)
		return;
	width += xStart;
	height += yStart;
	auto texWidth = static_cast<float>(info->width);
	auto texHeight = static_cast<float>(info->height);

	Vector2 uvMin(xStart / texWidth, yStart / texHeight);
	Vector2 uvMax(width / texWidth, height / texHeight);
	SetVertexUVCoord(0, uvMax);
	SetVertexUVCoord(1, uvMin);
	SetVertexUVCoord(2, Vector2 {uvMin.x, uvMax.y});
	SetVertexUVCoord(3, uvMin);
	SetVertexUVCoord(4, uvMax);
	SetVertexUVCoord(5, Vector2 {uvMax.x, uvMin.y});
	Update();
}
