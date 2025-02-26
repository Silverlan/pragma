/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_input_component.hpp"
#include "pragma/console/c_cvar.h"
#include <wgui/wgui.h>
#include <prosper_window.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

CInputComponent::CInputComponent(BaseEntity &ent) : BaseEntityComponent {ent} {}
CInputComponent::~CInputComponent() {}
void CInputComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CInputComponent::Initialize() { BaseEntityComponent::Initialize(); }

static CVar cvAcceleration = GetClientConVar("cl_mouse_acceleration");
void CInputComponent::UpdateMouseMovementDeltaValues()
{
	float xDelta, yDelta;
	auto *window = WGUI::GetInstance().FindFocusedWindow();
	if(window && window->IsValid() && WGUI::GetInstance().GetFocusedElement(window) == nullptr) {
		auto size = (*window)->GetSize();
		auto w = size.x;
		auto h = size.y;
		auto pos = (*window)->GetCursorPos();
		(*window)->SetCursorPos(Vector2i(umath::round(w / 2.f), umath::round(h / 2.f)));
		xDelta = pos.x - w / 2.f;
		yDelta = pos.y - h / 2.f;
		if((h % 2) != 0)
			yDelta -= 0.5f;
	}
	else {
		xDelta = 0.f;
		yDelta = 0.f;
	}

	auto acc = cvAcceleration->GetFloat() + 1.f;
	if(yDelta != 0.f)
		yDelta = umath::pow(CFloat(abs(yDelta)), acc) * ((yDelta > 0.f) ? 1 : -1);
	if(xDelta != 0.f)
		xDelta = umath::pow(CFloat(abs(xDelta)), acc) * ((xDelta > 0.f) ? 1 : -1);
	m_mouseDeltaX = xDelta;
	m_mouseDeltaY = yDelta;
}

float CInputComponent::GetMouseDeltaX() const { return m_mouseDeltaX; }
float CInputComponent::GetMouseDeltaY() const { return m_mouseDeltaY; }
