// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <string>

#include <cinttypes>
#include <optional>
#include <functional>
#include <memory>
#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.base_toggle;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseToggleComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_TURN_ON;
		static ComponentEventId EVENT_ON_TURN_OFF;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class SpawnFlags : uint32_t { StartOn = SF_STARTON };

		BaseToggleComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		bool IsTurnedOn() const;
		virtual void TurnOn();
		virtual void TurnOff();
		void SetTurnedOn(bool b);
		void Toggle();
		const ::util::PBoolProperty &GetTurnedOnProperty() const;
		virtual void OnEntitySpawn() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		::util::PBoolProperty m_bTurnedOn = nullptr;
		bool m_bStartDisabled = false;
		pragma::NetEventId m_netEvToggleState = pragma::INVALID_NET_EVENT;
		bool ToggleInput(std::string input, pragma::ecs::BaseEntity *activator, pragma::ecs::BaseEntity *caller, std::string data);
	};
};
