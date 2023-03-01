/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_IO_COMPONENT_HPP__
#define __BASE_IO_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/output.h"
#include <sharedutils/property/util_property.hpp>

struct AttachmentData;
namespace pragma {
	struct DLLNETWORK CEInputData : public ComponentEvent {
		CEInputData(const std::string &input, BaseEntity *activator, BaseEntity *caller, const std::string &data);
		virtual void PushArguments(lua_State *l) override;
		const std::string &input;
		BaseEntity *const activator;
		BaseEntity *const caller;
		const std::string &data;
	};
	class DLLNETWORK BaseIOComponent : public BaseEntityComponent {
	  public:
		enum class IoFlags : uint32_t {
			None = 0u,
			ForceDelayedFire = 1u // Never triggers the input/output immediately, even if the delay is set to 0
		};

		static pragma::ComponentEventId EVENT_HANDLE_INPUT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual void Initialize() override;

		void StoreOutput(std::string name, std::string entities, std::string input, std::string param, float delay = 0.f, int times = -1);
		void StoreOutput(std::string name, std::string info);
		void TriggerOutput(std::string name, BaseEntity *activator, IoFlags flags = IoFlags::None);

		virtual void Input(std::string input, BaseEntity *activator, BaseEntity *caller, std::string data);
		void Input(const std::string input, BaseEntity *activator = nullptr, BaseEntity *caller = nullptr);
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		BaseIOComponent(BaseEntity &ent);
		bool FireSingleOutput(Output &output, BaseEntity *activator, IoFlags flags = IoFlags::None);

		std::unordered_map<std::string, std::vector<Output>> m_outputs;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseIOComponent::IoFlags)

#endif
