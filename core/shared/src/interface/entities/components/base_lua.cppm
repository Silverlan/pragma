// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"
#include <any>

export module pragma.shared:entities.components.base_lua;

export import :entities.components.base;
export import :entities.member_type;
export import :scripting.lua.types;
export import :util.any;

export {
	class ClassMembers;
	namespace pragma {
		class BasePlayerComponent;
		namespace detail {
			constexpr ents::EntityMemberType util_type_to_member_type(util::VarType type)
			{
				switch(type) {
				case util::VarType::Bool:
					return ents::EntityMemberType::Boolean;
				case util::VarType::Double:
					return ents::EntityMemberType::Double;
				case util::VarType::Float:
					return ents::EntityMemberType::Float;
				case util::VarType::Int8:
					return ents::EntityMemberType::Int8;
				case util::VarType::Int16:
					return ents::EntityMemberType::Int16;
				case util::VarType::Int32:
					return ents::EntityMemberType::Int32;
				case util::VarType::Int64:
					return ents::EntityMemberType::Int64;
				case util::VarType::String:
					return ents::EntityMemberType::String;
				case util::VarType::UInt8:
					return ents::EntityMemberType::UInt8;
				case util::VarType::UInt16:
					return ents::EntityMemberType::UInt16;
				case util::VarType::UInt32:
					return ents::EntityMemberType::UInt32;
				case util::VarType::UInt64:
					return ents::EntityMemberType::UInt64;
				case util::VarType::EulerAngles:
					return ents::EntityMemberType::EulerAngles;
				case util::VarType::Vector:
					return ents::EntityMemberType::Vector3;
				case util::VarType::Vector2:
					return ents::EntityMemberType::Vector2;
				case util::VarType::Vector4:
					return ents::EntityMemberType::Vector4;
				case util::VarType::Quaternion:
					return ents::EntityMemberType::Quaternion;
				case util::VarType::Entity:
					return ents::EntityMemberType::Entity;
				case util::VarType::Transform:
					return ents::EntityMemberType::Transform;
				case util::VarType::ScaledTransform:
					return ents::EntityMemberType::ScaledTransform;
				}
				return ents::EntityMemberType::Invalid;
			}
			constexpr util::VarType member_type_to_util_type(ents::EntityMemberType type)
			{
				switch(type) {
				case ents::EntityMemberType::Boolean:
					return util::VarType::Bool;
				case ents::EntityMemberType::Double:
					return util::VarType::Double;
				case ents::EntityMemberType::Float:
					return util::VarType::Float;
				case ents::EntityMemberType::Int8:
					return util::VarType::Int8;
				case ents::EntityMemberType::Int16:
					return util::VarType::Int16;
				case ents::EntityMemberType::Int32:
					return util::VarType::Int32;
				case ents::EntityMemberType::Int64:
					return util::VarType::Int64;
				case ents::EntityMemberType::String:
					return util::VarType::String;
				case ents::EntityMemberType::UInt8:
					return util::VarType::UInt8;
				case ents::EntityMemberType::UInt16:
					return util::VarType::UInt16;
				case ents::EntityMemberType::UInt32:
					return util::VarType::UInt32;
				case ents::EntityMemberType::UInt64:
					return util::VarType::UInt64;
				case ents::EntityMemberType::EulerAngles:
					return util::VarType::EulerAngles;
				case ents::EntityMemberType::Vector3:
					return util::VarType::Vector;
				case ents::EntityMemberType::Vector2:
					return util::VarType::Vector2;
				case ents::EntityMemberType::Vector4:
					return util::VarType::Vector4;
				case ents::EntityMemberType::Quaternion:
					return util::VarType::Quaternion;
				case ents::EntityMemberType::Entity:
					return util::VarType::Entity;
				case ents::EntityMemberType::Transform:
					return util::VarType::Transform;
				case ents::EntityMemberType::ScaledTransform:
					return util::VarType::ScaledTransform;
				}
				return util::VarType::Invalid;
			}
		};
		class DLLNETWORK BaseLuaBaseEntityComponent : public BaseEntityComponent, public DynamicMemberRegister {
		  public:
			using MemberIndex = uint32_t;
			static constexpr auto INVALID_MEMBER = std::numeric_limits<MemberIndex>::max();
			enum class MemberFlags : uint32_t {
				None = 0u,
				PropertyBit = 1u,
				GetterBit = PropertyBit << 1u,
				SetterBit = GetterBit << 1u,
				StoreBit = SetterBit << 1u,
				KeyValueBit = StoreBit << 1u,
				InputBit = KeyValueBit << 1u,
				OutputBit = InputBit << 1u,
				NetworkedBit = OutputBit << 1u, // Will be sent from server to client when the entity has been requested by a client
				UseHasGetterBit = NetworkedBit << 1u,
				UseIsGetterBit = UseHasGetterBit << 1u,
				TransmitOnChange = UseIsGetterBit << 1u | NetworkedBit, // Same as above, but also transmits the value of the member to all clients whenever it has been changed
				SnapshotData = UseIsGetterBit << 2u | NetworkedBit,     // Same as NetworkedBit, but also transmits the value of the member every snapshot

				Default = GetterBit | SetterBit | StoreBit | KeyValueBit | InputBit | OutputBit,
				DefaultNetworked = Default | NetworkedBit,
				DefaultTransmit = Default | TransmitOnChange,
				DefaultSnapshot = Default | SnapshotData
			};
			struct MemberInfo {
				struct TransformCompositeInfo {
					ComponentMemberIndex posIdx = INVALID_COMPONENT_MEMBER_INDEX;
					ComponentMemberIndex rotIdx = INVALID_COMPONENT_MEMBER_INDEX;
					ComponentMemberIndex scaleIdx = INVALID_COMPONENT_MEMBER_INDEX;
				};

				MemberInfo() = default;
				MemberInfo(const MemberInfo &other);
				MemberInfo(MemberInfo &&other);
				MemberInfo(const std::string &functionName, const std::string &memberName, size_t memberNameHash, const std::string &memberVariableName, ents::EntityMemberType type, const std::any &initialValue, MemberFlags flags,
				  const luabind::object &onChange, const std::optional<ComponentMemberInfo> &componentMemberInfo);
				MemberInfo &operator=(const MemberInfo &other);
				MemberInfo &operator=(MemberInfo &&other);

				GString functionName;
				GString memberName;
				size_t memberNameHash;
				std::string memberVariableName;
				ents::EntityMemberType type;
				std::any initialValue;
				MemberFlags flags;
				mutable luabind::object onChange;
				std::unique_ptr<TransformCompositeInfo> transformCompositeInfo;

				std::optional<ComponentMemberInfo> componentMemberInfo {};
			};
			struct DLLNETWORK DynamicMemberInfo {
				bool enabled = false;
				std::any value;
				luabind::object onChange;
			};
			static MemberIndex RegisterMember(const luabind::object &oClass, const std::string &memberName, ents::EntityMemberType memberType, const std::any &initialValue, MemberFlags memberFlags, const Lua::map<std::string, void> &attributes);
			static std::vector<MemberInfo> *GetMemberInfos(const luabind::object &oClass);
			static void ClearMembers(lua::State *l);

			const MemberInfo *GetLuaMemberInfo(ComponentMemberInfo &memberInfo) const;
			virtual void Initialize() override;
			virtual void InitializeMembers(const std::vector<MemberInfo> &members);
			virtual void OnTick(double dt) override;
			virtual uint32_t GetStaticMemberCount() const override;
			void SetupLua(const luabind::object &o);
			void SetNetworked(bool b);
			bool IsNetworked() const;
			void SetShouldTransmitSnapshotData(bool b);

			virtual bool ShouldTransmitSnapshotData() const override;

			virtual void OnMembersChanged() override { BaseEntityComponent::OnMembersChanged(); }
			virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

			virtual void OnAttached(ecs::BaseEntity &ent) override;
			virtual void OnDetached(ecs::BaseEntity &ent) override;

			const luabind::object &GetLuaObject() const;
			luabind::object &GetLuaObject();
			const std::vector<DynamicMemberInfo> &GetDynamicMembers() const { return m_dynamicMembers; }
			const DynamicMemberInfo *GetDynamicMemberInfo(ComponentMemberIndex idx) const;
			DynamicMemberInfo *GetDynamicMemberInfo(ComponentMemberIndex idx);

			template<typename T>
			void SetDynamicMemberValue(ComponentMemberIndex memberIndex, const T &value);
			template<typename T>
			bool GetDynamicMemberValue(ComponentMemberIndex memberIndex, T &outValue, ents::EntityMemberType &outType);
			std::any *GetDynamicMemberValue(ComponentMemberIndex memberIndex, ents::EntityMemberType &outType);

			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
			virtual uint32_t GetVersion() const override;

			CallbackHandle BindInitComponentEvent(ComponentId componentId, const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &callback);
			CallbackHandle BindNetEvent(NetEventId eventId, const std::function<void(std::reference_wrapper<NetPacket>, BasePlayerComponent *)> &callback);
			virtual void OnEntitySpawn() override;
			virtual void OnEntityPostSpawn() override;
			virtual void OnRemove() override;
			using BaseEntityComponent::BindEvent;

			void ReserveMembers(uint32_t count);
			ComponentMemberIndex RegisterMember(ComponentMemberInfo &&memberInfo, luabind::object &onChange);
			ComponentMemberIndex RegisterMember(const ComponentMemberInfo &memberInfo, luabind::object &onChange);
			void ClearMembers();
			void RemoveMember(ComponentMemberIndex idx);
			void RemoveMember(const std::string &name);
			void UpdateMemberNameMap();

			// These should only be called through Lua
			CallbackHandle BindInitComponentEvent(lua::State *l, ComponentId componentId, luabind::object methodNameOrFunction);
			CallbackHandle BindEvent(lua::State *l, ComponentId eventId, luabind::object methodNameOrFunction);
			CallbackHandle BindNetEvent(lua::State *l, NetEventId eventId, luabind::object methodNameOrFunction);
			virtual void OnMemberValueChanged(uint32_t memberIdx);

			void Lua_Initialize() {}
			static void default_Lua_Initialize(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnTick(double dt) {}
			static void default_Lua_OnTick(lua::State *l, BaseLuaBaseEntityComponent &hComponent, double dt) {}

			void Lua_OnRemove() {}
			static void default_Lua_OnRemove(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnEntitySpawn() {}
			static void default_Lua_OnEntitySpawn(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnEntityPostSpawn() {}
			static void default_Lua_OnEntityPostSpawn(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnActiveStateChanged(bool activate) {}
			static void default_Lua_OnActiveStateChanged(lua::State *l, BaseLuaBaseEntityComponent &hComponent, bool activate) {}

			void Lua_OnAttachedToEntity() {}
			static void default_Lua_OnAttachedToEntity(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnDetachedToEntity() {}
			static void default_Lua_OnDetachedToEntity(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_HandleEvent(uint32_t eventId) {}
			static void default_Lua_HandleEvent(lua::State *l, BaseLuaBaseEntityComponent &hComponent, uint32_t eventId) {}

			void Lua_Save(udm::LinkedPropertyWrapper &udm) {}
			static void default_Lua_Save(lua::State *l, BaseLuaBaseEntityComponent &hComponent, udm::LinkedPropertyWrapper &udm) {}

			void Lua_Load(udm::LinkedPropertyWrapper &udm, uint32_t version) {}
			static void default_Lua_Load(lua::State *l, BaseLuaBaseEntityComponent &hComponent, udm::LinkedPropertyWrapper &udm, uint32_t version) {}

			void Lua_OnEntityComponentAdded(BaseLuaBaseEntityComponent &hComponent) {}
			static void default_Lua_OnEntityComponentAdded(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}

			void Lua_OnEntityComponentRemoved(BaseLuaBaseEntityComponent &hComponent) {}
			static void default_Lua_OnEntityComponentRemoved(lua::State *l, BaseLuaBaseEntityComponent &hComponent) {}
		  protected:
			BaseLuaBaseEntityComponent(ecs::BaseEntity &ent);
			luabind::object *GetClassObject();
			const luabind::object *GetClassObject() const { return const_cast<BaseLuaBaseEntityComponent *>(this)->GetClassObject(); }
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, BasePlayerComponent *pl) = 0;
			virtual void InitializeMember(const MemberInfo &memberInfo);
			virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
			std::any GetMemberValue(const MemberInfo &memberInfo) const;
			void SetMemberValue(const MemberInfo &memberInfo, const std::any &value) const;
			const std::vector<MemberInfo> &GetMembers() const;
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
			std::unordered_map<NetEventId, CallbackHandle> m_boundNetEvents;

			struct NetworkedMemberInfo {
				std::vector<size_t> networkedMembers = {};
				std::vector<size_t> snapshotMembers = {};
				std::unordered_map<size_t, size_t> memberIndexToNetworkedIndex = {}; // m_member index to networkedMembers index
				NetEventId netEvSetMember = INVALID_NET_EVENT;
			};
			// Most components most likely will not have any networked members, so we'll only initialize the networked data for those that need it.
			std::unique_ptr<NetworkedMemberInfo> m_networkedMemberInfo = nullptr;

			using BaseEntityComponent::InitializeLuaObject;
		  private:
			mutable ClassMembers *m_classMembers = nullptr;

			virtual void OnMemberRegistered(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) override;
			virtual void OnMemberRemoved(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index) override;
			virtual void OnActiveStateChanged(bool active) override;

			std::vector<MemberInfo> m_members = {};
			std::vector<DynamicMemberInfo> m_dynamicMembers;
			uint32_t m_dynamicMemberStartOffset = 0;
			std::unordered_map<GString, size_t> m_memberNameToIndex = {};
			uint32_t m_classMemberIndex = std::numeric_limits<uint32_t>::max();
			bool m_bShouldTransmitNetData = false;
			bool m_bShouldTransmitSnapshotData = false;
			uint32_t m_version = 1u;
			std::unordered_map<ComponentId, CallbackHandle> m_initComponentCallbacks;
		};

		namespace LuaCore {
			DLLNETWORK std::optional<ComponentMemberInfo> get_component_member_info(lua::State *l, const std::string &functionName, ents::EntityMemberType memberType, const std::any &initialValue, BaseLuaBaseEntityComponent::MemberFlags memberFlags,
			  const Lua::map<std::string, void> &attributes, luabind::object &outOnChange, bool dynamicMember);
			DLLNETWORK BaseLuaBaseEntityComponent::MemberFlags string_to_member_flags(lua::State *l, std::string_view strFlags);
			template<typename T>
			void register_shared_lua_component_methods(auto &def)
			{
				def.def(
				  "RegisterMember",
				  +[](lua::State *l, T &hComponent, const std::string &memberName, ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes,
				     BaseLuaBaseEntityComponent::MemberFlags memberFlags) -> std::optional<ComponentMemberIndex> {
					  luabind::object onChange;
					  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
					  auto memberInfo = get_component_member_info(l, memberName, memberType, anyInitialValue, memberFlags, attributes, onChange, true);
					  if(!memberInfo.has_value())
						  return {};
					  return hComponent.RegisterMember(std::move(*memberInfo), onChange);
				  });
				def.def(
				  "RegisterMember", +[](lua::State *l, T &hComponent, const std::string &memberName, ents::EntityMemberType memberType, Lua::udm_type oDefault, const std::string &memberFlags) -> std::optional<ComponentMemberIndex> {
					  luabind::object onChange;
					  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
					  auto t = luabind::newtable(l);
					  auto memberInfo = get_component_member_info(l, memberName, memberType, anyInitialValue, string_to_member_flags(l, memberFlags), t, onChange, true);
					  if(!memberInfo.has_value())
						  return {};
					  return hComponent.RegisterMember(std::move(*memberInfo), onChange);
				  });
				def.def(
				  "RegisterMember",
				  +[](lua::State *l, T &hComponent, const std::string &memberName, ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes, const std::string &memberFlags) -> std::optional<ComponentMemberIndex> {
					  luabind::object onChange;
					  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
					  auto memberInfo = get_component_member_info(l, memberName, memberType, anyInitialValue, string_to_member_flags(l, memberFlags), attributes, onChange, true);
					  if(!memberInfo.has_value())
						  return {};
					  return hComponent.RegisterMember(std::move(*memberInfo), onChange);
				  });
				def.def(
				  "RegisterMember", +[](lua::State *l, T &hComponent, const std::string &memberName, ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes) -> std::optional<ComponentMemberIndex> {
					  luabind::object onChange;
					  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
					  auto memberInfo = get_component_member_info(l, memberName, memberType, anyInitialValue, BaseLuaBaseEntityComponent::MemberFlags::Default, attributes, onChange, true);
					  if(!memberInfo.has_value())
						  return {};
					  return hComponent.RegisterMember(std::move(*memberInfo), onChange);
				  });
				def.def(
				  "RegisterMember", +[](lua::State *l, T &hComponent, const std::string &memberName, ents::EntityMemberType memberType, Lua::udm_type oDefault) -> std::optional<ComponentMemberIndex> {
					  luabind::object onChange;
					  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
					  auto t = luabind::newtable(l);
					  auto memberInfo = get_component_member_info(l, memberName, memberType, anyInitialValue, BaseLuaBaseEntityComponent::MemberFlags::Default, t, onChange, true);
					  if(!memberInfo.has_value())
						  return {};
					  return hComponent.RegisterMember(std::move(*memberInfo), onChange);
				  });
				def.def("ClearMembers", static_cast<void (T::*)()>(&T::ClearMembers));
				def.def("ReserveMembers", static_cast<void (T::*)(uint32_t)>(&T::ReserveMembers));
				def.def("RemoveMember", static_cast<void (T::*)(ComponentMemberIndex)>(&T::RemoveMember));
				def.def("RemoveMember", static_cast<void (T::*)(const std::string &)>(&T::RemoveMember));
				def.def("UpdateMemberNameMap", static_cast<void (T::*)()>(&T::UpdateMemberNameMap));
				def.def("OnMembersChanged", static_cast<void (T::*)()>(&T::OnMembersChanged));
			}
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseLuaBaseEntityComponent::MemberFlags)

	namespace pragma {
		template<typename T>
		void BaseLuaBaseEntityComponent::SetDynamicMemberValue(ComponentMemberIndex memberIndex, const T &value)
		{
			auto *memberInfo = GetMemberInfo(memberIndex);
			if(!memberInfo)
				return;
			ents::EntityMemberType type;
			auto *anyVal = GetDynamicMemberValue(memberIndex, type);
			if(!anyVal)
				return;
			udm::visit(ents::member_type_to_udm_type(memberInfo->type), [this, memberInfo, &value, anyVal](auto tag) {
				using TMember = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T, TMember>())
					*anyVal = udm::convert<T, TMember>(value);
			});
		}
		template<typename T>
		bool BaseLuaBaseEntityComponent::GetDynamicMemberValue(ComponentMemberIndex memberIndex, T &outValue, ents::EntityMemberType &outType)
		{
			auto *anyVal = GetDynamicMemberValue(memberIndex, outType);
			if(!anyVal)
				return false;
			return udm::visit(ents::member_type_to_udm_type(outType), [anyVal, &outValue](auto tag) {
				using TMember = typename decltype(tag)::type;
				if constexpr(udm::is_udm_type<T>() && udm::is_udm_type<TMember>() && is_valid_component_property_type_v<TMember> && udm::is_convertible<TMember, T>()) {
					outValue = udm::convert<TMember, T>(std::any_cast<TMember>(*anyVal));
					return true;
				}
				return false;
			});
		}
	}

	namespace Lua {
		DLLNETWORK void register_base_entity_component(luabind::module_ &modEnts);
	}
};
