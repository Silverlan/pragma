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
#include <udm.hpp>
#include <any>

extern DLLNETWORK Engine *engine;

class BaseEntity;
struct ClassMembers;
namespace pragma
{
	namespace detail
	{
		constexpr udm::Type util_type_to_udm_type(util::VarType type)
		{
			switch(type)
			{
			case util::VarType::Bool:
				return udm::Type::Boolean;
			case util::VarType::Double:
				return udm::Type::Double;
			case util::VarType::Float:
				return udm::Type::Float;
			case util::VarType::Int8:
				return udm::Type::Int8;
			case util::VarType::Int16:
				return udm::Type::Int16;
			case util::VarType::Int32:
				return udm::Type::Int32;
			case util::VarType::Int64:
				return udm::Type::Int64;
			case util::VarType::String:
				return udm::Type::String;
			case util::VarType::UInt8:
				return udm::Type::UInt8;
			case util::VarType::UInt16:
				return udm::Type::UInt16;
			case util::VarType::UInt32:
				return udm::Type::UInt32;
			case util::VarType::UInt64:
				return udm::Type::UInt64;
			case util::VarType::EulerAngles:
				return udm::Type::EulerAngles;
			case util::VarType::Vector:
				return udm::Type::Vector3;
			case util::VarType::Vector2:
				return udm::Type::Vector2;
			case util::VarType::Vector4:
				return udm::Type::Vector4;
			case util::VarType::Quaternion:
				return udm::Type::Quaternion;
			}
			return udm::Type::Invalid;
		}
		constexpr util::VarType udm_type_to_util_type(udm::Type type)
		{
			switch(type)
			{
			case udm::Type::Boolean:
				return util::VarType::Bool;
			case udm::Type::Double:
				return util::VarType::Double;
			case udm::Type::Float:
				return util::VarType::Float;
			case udm::Type::Int8:
				return util::VarType::Int8;
			case udm::Type::Int16:
				return util::VarType::Int16;
			case udm::Type::Int32:
				return util::VarType::Int32;
			case udm::Type::Int64:
				return util::VarType::Int64;
			case udm::Type::String:
				return util::VarType::String;
			case udm::Type::UInt8:
				return util::VarType::UInt8;
			case udm::Type::UInt16:
				return util::VarType::UInt16;
			case udm::Type::UInt32:
				return util::VarType::UInt32;
			case udm::Type::UInt64:
				return util::VarType::UInt64;
			case udm::Type::EulerAngles:
				return util::VarType::EulerAngles;
			case udm::Type::Vector3:
				return util::VarType::Vector;
			case udm::Type::Vector2:
				return util::VarType::Vector2;
			case udm::Type::Vector4:
				return util::VarType::Vector4;
			case udm::Type::Quaternion:
				return util::VarType::Quaternion;
			}
			return util::VarType::Invalid;
		}
	};
	class DLLNETWORK BaseLuaBaseEntityComponent
		: public pragma::BaseEntityComponent
	{
	public:
		using MemberIndex = uint32_t;
		static constexpr auto INVALID_MEMBER = std::numeric_limits<MemberIndex>::max();
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
			std::string functionName;
			std::string memberName;
			size_t memberNameHash;
			std::string memberVariableName;
			udm::Type type;
			std::any initialValue;
			BaseLuaBaseEntityComponent::MemberFlags flags;
			mutable luabind::object onChange;

			std::optional<ComponentMemberInfo> componentMemberInfo {};
		};
		static MemberIndex RegisterMember(const luabind::object &oClass,const std::string &memberName,udm::Type memberType,const std::any &initialValue,MemberFlags memberFlags,const Lua::map<std::string,void> &attributes);
		static std::vector<MemberInfo> *GetMemberInfos(const luabind::object &oClass);
		static void ClearMembers(lua_State *l);

		const MemberInfo *GetLuaMemberInfo(ComponentMemberInfo &memberInfo) const;
		virtual void Initialize() override;
		virtual void InitializeMembers(const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &members);
		virtual void OnTick(double dt) override;
		void SetupLua(const luabind::object &o);
		void SetNetworked(bool b);
		bool IsNetworked() const;
		void SetShouldTransmitSnapshotData(bool b);

		bool ShouldTransmitSnapshotData() const;

		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
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
		luabind::object *GetClassObject();
		const luabind::object *GetClassObject() const {return const_cast<BaseLuaBaseEntityComponent*>(this)->GetClassObject();}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void InvokeNetEventHandle(const std::string &methodName,NetPacket &packet,pragma::BasePlayerComponent *pl)=0;
		virtual void InitializeMember(const MemberInfo &memberInfo);
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
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
		mutable ClassMembers *m_classMembers = nullptr;

		std::vector<MemberInfo> m_members = {};
		std::unordered_map<std::string,size_t> m_memberNameToIndex = {};
		uint32_t m_classMemberIndex = std::numeric_limits<uint32_t>::max();
		bool m_bShouldTransmitNetData = false;
		bool m_bShouldTransmitSnapshotData = false;
		uint32_t m_version = 1u;
		std::unordered_map<pragma::ComponentId,CallbackHandle> m_initComponentCallbacks;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseLuaBaseEntityComponent::MemberFlags)

#endif
