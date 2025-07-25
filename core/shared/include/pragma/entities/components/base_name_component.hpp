// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_NAME_COMPONENT_HPP__
#define __BASE_NAME_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma {

	struct DLLNETWORK CEOnNameChanged : public ComponentEvent {
		CEOnNameChanged(const std::string &newName);
		virtual void PushArguments(lua_State *l) override;
		const std::string &name;
	};

	class DLLNETWORK BaseNameComponent : public BaseEntityComponent {
	  public:
		static pragma::ComponentEventId EVENT_ON_NAME_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual ~BaseNameComponent() override;

		virtual void SetName(std::string name);
		const std::string &GetName() const;
		const util::PStringProperty &GetNameProperty() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseNameComponent(BaseEntity &ent);
		util::PStringProperty m_name;
		CallbackHandle m_cbOnNameChanged = {};
	};
};

#endif
