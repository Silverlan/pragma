// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.input;

import pragma.gui;
export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CInputComponent final : public BaseEntityComponent {
	  public:
		CInputComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual ~CInputComponent() override;

		float GetMouseDeltaX() const;
		float GetMouseDeltaY() const;

		virtual void InitializeLuaObject(lua::State *l) override;
		void UpdateMouseMovementDeltaValues();
	  private:
		float m_mouseDeltaX = 0.f;
		float m_mouseDeltaY = 0.f;
		gui::Element *m_lastFocusedElement = nullptr;
		bool m_initialFocus = true;
	};
};
