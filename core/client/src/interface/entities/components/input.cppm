// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "wgui/wibase.h"
#include <pragma/entities/components/base_entity_component.hpp>

export module pragma.client:entities.components.input;
export namespace pragma {
	class DLLCLIENT CInputComponent final : public BaseEntityComponent {
	  public:
		CInputComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual ~CInputComponent() override;

		float GetMouseDeltaX() const;
		float GetMouseDeltaY() const;

		virtual void InitializeLuaObject(lua_State *l) override;
		void UpdateMouseMovementDeltaValues();
	  private:
		float m_mouseDeltaX = 0.f;
		float m_mouseDeltaY = 0.f;
		WIBase *m_lastFocusedElement = nullptr;
		bool m_initialFocus = true;
	};
};

