/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_ENTITY_COMPONENT_MEMBER_REGISTER_HPP__
#define __BASE_ENTITY_COMPONENT_MEMBER_REGISTER_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma {
	class DLLNETWORK DynamicMemberRegister {
	  public:
		DynamicMemberRegister();

		std::optional<ComponentMemberIndex> GetMemberIndex(const std::string &name) const;
		const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const;

		const std::unordered_map<ComponentMemberIndex, ComponentMemberInfo> &GetMembers() const { return m_members; }
		const std::unordered_map<std::string, ComponentMemberIndex> &GetMemberIndexMap() const { return m_memberNameToIndex; }
	  protected:
		virtual void OnMemberRegistered(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) {}
		virtual void OnMemberRemoved(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) {}
		void ReserveMembers(uint32_t count);
		ComponentMemberIndex RegisterMember(ComponentMemberInfo &&memberInfo);
		ComponentMemberIndex RegisterMember(const ComponentMemberInfo &memberInfo);
		void ClearMembers();
		void RemoveMember(ComponentMemberIndex idx);
		void RemoveMember(const std::string &name);
		void UpdateMemberNameMap();
	  private:
		std::unordered_map<ComponentMemberIndex, ComponentMemberInfo> m_members = {};
		std::unordered_map<std::string, ComponentMemberIndex> m_memberNameToIndex = {};
		uint32_t m_nextMemberIndex = std::numeric_limits<uint32_t>::max();
	};
};

#endif
