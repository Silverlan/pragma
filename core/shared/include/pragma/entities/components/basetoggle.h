// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASETOGGLE_H__
#define __BASETOGGLE_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/shared_spawnflags.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>
#include <string>
class BaseEntity;

namespace pragma {
	class DLLNETWORK BaseToggleComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_TURN_ON;
		static ComponentEventId EVENT_ON_TURN_OFF;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class SpawnFlags : uint32_t { StartOn = SF_STARTON };

		BaseToggleComponent(BaseEntity &ent);
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
		bool ToggleInput(std::string input, BaseEntity *activator, BaseEntity *caller, std::string data);
	};
};

#endif
