// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :entities.components.input;
using namespace pragma;

CInputComponent::CInputComponent(ecs::BaseEntity &ent) : BaseEntityComponent {ent} {}
CInputComponent::~CInputComponent() {}
void CInputComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CInputComponent::Initialize() { BaseEntityComponent::Initialize(); }

static auto cvAcceleration = console::get_client_con_var("cl_mouse_acceleration");
void CInputComponent::UpdateMouseMovementDeltaValues()
{
	auto xDelta = 0.f;
	auto yDelta = 0.f;
	auto *window = gui::WGUI::GetInstance().FindFocusedWindow();
	if(window && window->IsValid()) {
		auto *focusedElement = gui::WGUI::GetInstance().GetFocusedElement(window);
		if(!focusedElement) {
			auto size = (*window)->GetSize();
			auto w = size.x;
			auto h = size.y;

			if(m_lastFocusedElement || m_initialFocus) {
				// We went from focused UI element to unfocused (or this is the first time, e.g.
				// when the game was just launched)
				// -> Recenter the mouse cursor
				m_initialFocus = false;
				m_lastFocusedElement = nullptr;
				(*window)->SetCursorPos(Vector2i(math::round(w / 2.f), math::round(h / 2.f)));
			}

			auto pos = (*window)->GetCursorPos();
			(*window)->SetCursorPos(Vector2i(math::round(w / 2.f), math::round(h / 2.f)));
			xDelta = pos.x - w / 2.f;
			yDelta = pos.y - h / 2.f;
			if((w % 2) != 0)
				xDelta -= 0.5f;
			if((h % 2) != 0)
				yDelta -= 0.5f;
		}
		m_lastFocusedElement = focusedElement;
	}

	auto acc = cvAcceleration->GetFloat() + 1.f;
	if(yDelta != 0.f)
		yDelta = math::pow(CFloat(abs(yDelta)), acc) * ((yDelta > 0.f) ? 1 : -1);
	if(xDelta != 0.f)
		xDelta = math::pow(CFloat(abs(xDelta)), acc) * ((xDelta > 0.f) ? 1 : -1);
	m_mouseDeltaX = xDelta;
	m_mouseDeltaY = yDelta;
}

float CInputComponent::GetMouseDeltaX() const { return m_mouseDeltaX; }
float CInputComponent::GetMouseDeltaY() const { return m_mouseDeltaY; }
