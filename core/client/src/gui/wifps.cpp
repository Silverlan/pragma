/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wifps.h"
#include <wgui/types/witext.h>

import pragma.string.unicode;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;

LINK_WGUI_TO_CLASS(WIFPS, WIFPS);

WIFPS::WIFPS() : WIBase(), m_text(), m_fpsLast(0), m_tLastUpdate(0.0) {}

WIFPS::~WIFPS() {}

void WIFPS::Initialize()
{
	WIBase::Initialize();
	EnableThinking();
	m_text = CreateChild<WIText>();
	if(m_text.IsValid()) {
		auto *t = static_cast<WIText *>(m_text.get());
		t->EnableShadow(true);
		t->SetShadowOffset(2, 2);
		t->SetShadowBlurSize(0.1f);
		t->SetShadowColor(0.f, 0.f, 0.f, 1.f);
		t->SetText("FPS 000 Frame Time: 000.00ms");
		t->AddStyleClass("fps_counter");
		t->SetColor(1.f, 1.f, 1.f, 1.f);
		t->SizeToContents();
		auto &sz = m_text->GetSize();
		SetSize(sz.x, sz.y);
	}
}

void WIFPS::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIBase::Think(drawCmd);
	if(!m_text.IsValid())
		return;
	auto &tCur = client->RealTime();
	if(tCur - m_tLastUpdate > 0.75) {
		m_tLastUpdate = tCur;
		auto fps = c_engine->GetFPS();
		if(fps != m_fpsLast) {
			auto frameTime = c_engine->GetFrameTime();
			m_fpsLast = fps;
			auto strFps = util::round_string(fps, 0);
			auto l = strFps.length();
			if(l < 3)
				strFps += std::string(3 - l, ' ');
			auto t = static_cast<WIText *>(m_text.get());
			Color col {0, 255, 0, 255};
			if(fps < 60) {
				Color b {255, 128, 0, 255};
				if(fps >= 30)
					col = col.Lerp(b, static_cast<float>(60 - fps) / 30.f);
				else {
					col = b;
					b = {255, 0, 0, 255};
					col = col.Lerp(b, 1.f - static_cast<float>(fps) / 30.f);
				}
			}
			t->SetColor(col);
			t->SetText("FPS " + strFps + " Frame Time: " + util::round_string(frameTime, 2) + "ms");
		}
	}
}
