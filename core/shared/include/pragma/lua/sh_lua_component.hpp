/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SH_LUA_COMPONENT_HPP__
#define __SH_LUA_COMPONENT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/lua/luaobjectbase.h"
#include "pragma/util/util_variable_type.hpp"
#include <pragma/lua/luaapi.h>
#include <any>

extern DLLNETWORK Engine *engine;

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BaseLuaBaseEntityComponent
		: public pragma::BaseEntityComponent
	{
	public:
		enum class MemberFlags : uint32_t
		{
			None = 0u,
			PropertyBit = 1u,
			GetterBit = PropertyBit<<1u,
			SetterBit = GetterBit<<1u,
			StoreBit = SetterBit<<1u,
			KeyValueBit = StoreBit<<1u,
			InputBit = KeyValueBit<<1u,
			OutputBit = InputBit<<1u,
			NetworkedBit = OutputBit<<1u, // Will be sent from server to client when the entity has been requested by a client
			UseHasGetterBit = NetworkedBit<<1u,
			UseIsGetterBit = UseHasGetterBit<<1u,
			TransmitOnChange = UseIsGetterBit<<1u | NetworkedBit, // Same as above, but also transmits the value of the member to all clients whenever it has been changed
			SnapshotData = UseIsGetterBit<<2u | NetworkedBit, // Same as NetworkedBit, but also transmits the value of the member every snapshot

			Default = GetterBit | SetterBit | StoreBit | KeyValueBit | InputBit | OutputBit,
			DefaultNetworked = Default | NetworkedBit,
			DefaultTransmit = Default | TransmitOnChange,
			DefaultSnapshot = Default | SnapshotData
		};
		struct MemberInfo
		{
			std::string name;
			util::VarType type;
			std::any initialValue;
			BaseLuaBaseEntityComponent::MemberFlags flags;
			uint32_t version;
		};
		static void RegisterMember(const luabind::object &oClass,const std::string &memberName,util::VarType memberType,const std::any &initialValue,MemberFlags memberFlags,uint32_t version);
		static std::vector<MemberInfo> *GetMemberInfos(const luabind::object &oClass);
		static void ClearMembers(lua_State *l);

		virtual void Initialize() override;
		virtual void InitializeMembers(const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &members);
		virtual void OnTick(double dt) override;
		void SetupLua(const luabind::object &o);
		void SetNetworked(bool b);
		bool IsNetworked() const;
		void SetShouldTransmitSnapshotData(bool b);

		bool ShouldTransmitSnapshotData() const;

		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		virtual void OnAttached(BaseEntity &ent) override;
		virtual void OnDetached(BaseEntity &ent) override;

		const luabind::object &GetLuaObject() const;
		luabind::object &GetLuaObject();

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		virtual uint32_t GetVersion() const override;

		CallbackHandle BindInitComponentEvent(ComponentId componentId,const std::function<void(std::reference_wrapper<pragma::BaseEntityComponent>)> &callback);
		CallbackHandle BindNetEvent(pragma::NetEventId eventId,const std::function<void(std::reference_wrapper<NetPacket>,pragma::BasePlayerComponent*)> &callback);
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		using pragma::BaseEntityComponent::BindEvent;

		// These should only be called through Lua
		CallbackHandle BindInitComponentEvent(lua_State *l,pragma::ComponentId componentId,luabind::object methodNameOrFunction);
		CallbackHandle BindEvent(lua_State *l,pragma::ComponentId eventId,luabind::object methodNameOrFunction);
		CallbackHandle BindNetEvent(lua_State *l,pragma::NetEventId eventId,luabind::object methodNameOrFunction);
		virtual void OnMemberValueChanged(uint32_t memberIdx);

		void Lua_Initialize() {}
		static void default_Lua_Initialize(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_OnTick(double dt) {}
		static void default_Lua_OnTick(lua_State *l,BaseLuaBaseEntityComponent &hComponent,double dt) {}

		void Lua_OnRemove() {}
		static void default_Lua_OnRemove(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_OnEntitySpawn() {}
		static void default_Lua_OnEntitySpawn(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_OnAttachedToEntity() {}
		static void default_Lua_OnAttachedToEntity(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_OnDetachedToEntity() {}
		static void default_Lua_OnDetachedToEntity(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_HandleEvent(uint32_t eventId) {}
		static void default_Lua_HandleEvent(lua_State *l,BaseLuaBaseEntityComponent &hComponent,uint32_t eventId) {}

		void Lua_Save(udm::LinkedPropertyWrapper &udm) {}
		static void default_Lua_Save(lua_State *l,BaseLuaBaseEntityComponent &hComponent,udm::LinkedPropertyWrapper &udm) {}

		void Lua_Load(udm::LinkedPropertyWrapper &udm,uint32_t version) {}
		static void default_Lua_Load(lua_State *l,BaseLuaBaseEntityComponent &hComponent,udm::LinkedPropertyWrapper &udm,uint32_t version) {}

		void Lua_OnEntityComponentAdded(BaseLuaBaseEntityComponent &hComponent) {}
		static void default_Lua_OnEntityComponentAdded(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}

		void Lua_OnEntityComponentRemoved(BaseLuaBaseEntityComponent &hComponent) {}
		static void default_Lua_OnEntityComponentRemoved(lua_State *l,BaseLuaBaseEntityComponent &hComponent) {}
	protected:
		BaseLuaBaseEntityComponent(BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void InvokeNetEventHandle(const std::string &methodName,NetPacket &packet,pragma::BasePlayerComponent *pl)=0;
		virtual void InitializeMember(const MemberInfo &memberInfo);
		std::any GetMemberValue(const MemberInfo &memberInfo) const;
		void SetMemberValue(const MemberInfo &memberInfo,const std::any &value) const;
		const std::vector<MemberInfo> &GetMembers() const;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		std::unordered_map<pragma::NetEventId,CallbackHandle> m_boundNetEvents;

		struct NetworkedMemberInfo
		{
			std::vector<size_t> networkedMembers = {};
			std::vector<size_t> snapshotMembers = {};
			std::unordered_map<size_t,size_t> memberIndexToNetworkedIndex = {}; // m_member index to networkedMembers index
			pragma::NetEventId netEvSetMember = pragma::INVALID_NET_EVENT;
		};
		// Most components most likely will not have any networked members, so we'll only initialize the networked data for those that need it.
		std::unique_ptr<NetworkedMemberInfo> m_networkedMemberInfo = nullptr;

		using BaseEntityComponent::InitializeLuaObject;
	private:
		std::vector<MemberInfo> m_members = {};
		std::unordered_map<std::string,size_t> m_memberNameToIndex = {};
		bool m_bShouldTransmitNetData = false;
		bool m_bShouldTransmitSnapshotData = false;
		uint32_t m_version = 1u;
		std::unordered_map<pragma::ComponentId,CallbackHandle> m_initComponentCallbacks;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseLuaBaseEntityComponent::MemberFlags)

#endif
