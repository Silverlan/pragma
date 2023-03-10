/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __COMPONENT_MEMBER_REFERENCE_HPP__
#define __COMPONENT_MEMBER_REFERENCE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include <string>
#include <optional>

namespace pragma {
	struct DLLNETWORK ComponentMemberReference {
		static std::optional<ComponentMemberReference> Create(BaseEntityComponent &component, pragma::ComponentMemberIndex index);
		static std::optional<ComponentMemberReference> Create(const EntityComponentManager &manager, ComponentId componentId, pragma::ComponentMemberIndex index);
		ComponentMemberReference() = default;
		ComponentMemberReference(const ComponentMemberReference &) = default;
		ComponentMemberReference(const std::string &memberName);
		ComponentMemberReference &operator=(const ComponentMemberReference &) = default;

		pragma::ComponentMemberIndex GetMemberIndex() const { return m_index; }
		const std::string &GetMemberName() const { return m_name; }
		size_t GetMemberNameHash() const { return m_nameHash; }

		const ComponentMemberInfo *GetMemberInfo(const BaseEntityComponent &component) const;
	  private:
		mutable pragma::ComponentMemberIndex m_index = INVALID_COMPONENT_MEMBER_INDEX;
		std::string m_name;
		size_t m_nameHash = 0;
	};
};

#endif
