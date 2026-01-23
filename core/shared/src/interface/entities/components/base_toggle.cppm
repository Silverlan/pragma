// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_toggle;

export import :entities.components.base;

export namespace pragma {
	namespace baseToggleComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_TURN_ON)
		REGISTER_COMPONENT_EVENT(EVENT_ON_TURN_OFF)
	}
	class DLLNETWORK BaseToggleComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class SpawnFlags : uint32_t { StartOn = SF_STARTON };

		BaseToggleComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		bool IsTurnedOn() const;
		virtual void TurnOn();
		virtual void TurnOff();
		void SetTurnedOn(bool b);
		void Toggle();
		const util::PBoolProperty &GetTurnedOnProperty() const;
		virtual void OnEntitySpawn() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		util::PBoolProperty m_bTurnedOn = nullptr;
		bool m_bStartDisabled = false;
		NetEventId m_netEvToggleState = INVALID_NET_EVENT;
		bool ToggleInput(std::string input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, std::string data);
	};
};
