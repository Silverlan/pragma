// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.member_reference;

export import :entities.enums;

export namespace pragma {
	class BaseEntityComponent;
	class EntityComponentManager;
	struct ComponentMemberInfo;
	struct DLLNETWORK ComponentMemberReference {
		static std::optional<ComponentMemberReference> Create(BaseEntityComponent &component, ComponentMemberIndex index);
		static std::optional<ComponentMemberReference> Create(const EntityComponentManager &manager, ComponentId componentId, ComponentMemberIndex index);
		ComponentMemberReference() = default;
		ComponentMemberReference(const ComponentMemberReference &) = default;
		ComponentMemberReference(const std::string &memberName);
		ComponentMemberReference &operator=(const ComponentMemberReference &) = default;

		ComponentMemberIndex GetMemberIndex() const { return m_index; }
		const std::string &GetMemberName() const { return m_name; }
		size_t GetMemberNameHash() const { return m_nameHash; }

		const ComponentMemberInfo *GetMemberInfo(const BaseEntityComponent &component) const;
	  private:
		mutable ComponentMemberIndex m_index = INVALID_COMPONENT_MEMBER_INDEX;
		std::string m_name;
		size_t m_nameHash = 0;
	};
};
