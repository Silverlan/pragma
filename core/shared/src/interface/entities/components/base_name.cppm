// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_name;

export import :entities.components.base;

export namespace pragma {
	struct DLLNETWORK CEOnNameChanged : public ComponentEvent {
		CEOnNameChanged(const std::string &newName);
		virtual void PushArguments(lua::State *l) override;
		const std::string &name;
	};

	namespace baseNameComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_NAME_CHANGED)
	}
	class DLLNETWORK BaseNameComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual ~BaseNameComponent() override;

		virtual void SetName(std::string name);
		const std::string &GetName() const;
		const util::PStringProperty &GetNameProperty() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseNameComponent(ecs::BaseEntity &ent);
		util::PStringProperty m_name;
		CallbackHandle m_cbOnNameChanged = {};
	};
};
