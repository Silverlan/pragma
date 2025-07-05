// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_INPUT_COMPONENT_HPP__
#define __C_INPUT_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
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
	};
};

#endif
