// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_ENTITY_COMPONENT_MEMBER_REGISTER_HPP__
#define __BASE_ENTITY_COMPONENT_MEMBER_REGISTER_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma {
	class BaseEntityComponentSystem;
	class DLLNETWORK DynamicMemberRegister {
	  public:
		DynamicMemberRegister();

		std::optional<ComponentMemberIndex> GetMemberIndex(const std::string &name) const;
		const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const;

		const std::vector<ComponentMemberInfo> &GetMembers() const { return m_members; }
		const std::unordered_map<pragma::GString, ComponentMemberIndex> &GetMemberIndexMap() const { return m_memberNameToIndex; }
	  protected:
		friend BaseEntityComponentSystem;
		virtual void OnMemberRegistered(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) {}
		virtual void OnMemberRemoved(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) {}
		size_t GetDynamicMemberStartOffset() const;
		void ReserveMembers(uint32_t count);
		ComponentMemberIndex RegisterMember(ComponentMemberInfo &&memberInfo);
		ComponentMemberIndex RegisterMember(const ComponentMemberInfo &memberInfo);
		void ClearMembers();
		void RemoveMember(ComponentMemberIndex idx);
		void RemoveMember(const std::string &name);
		void UpdateMemberNameMap();
	  private:
		std::vector<ComponentMemberInfo> m_members = {};
		std::unordered_map<pragma::GString, ComponentMemberIndex> m_memberNameToIndex = {};
	};
};

#endif
