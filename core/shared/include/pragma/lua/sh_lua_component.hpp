#ifndef __SH_LUA_COMPONENT_HPP__
#define __SH_LUA_COMPONENT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/lua/luaobjectbase.h"
#include "pragma/util/util_variable_type.hpp"
#include <luasystem.h>
#include <any>

extern DLLENGINE Engine *engine;

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BaseLuaBaseEntityComponent
		: public pragma::BaseEntityComponent,
		public LuaObjectBase
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
		void SetNetworked(bool b);
		bool IsNetworked() const;
		void SetShouldTransmitSnapshotData(bool b);

		bool ShouldTransmitSnapshotData() const;

		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		virtual void OnAttached(BaseEntity &ent) override;
		virtual void OnDetached(BaseEntity &ent) override;

		const luabind::object &GetLuaObject() const;
		luabind::object &GetLuaObject();

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
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
	protected:
		BaseLuaBaseEntityComponent(BaseEntity &ent,luabind::object &o);
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void InvokeNetEventHandle(const std::string &methodName,NetPacket &packet,pragma::BasePlayerComponent *pl)=0;
		virtual void InitializeMember(const MemberInfo &memberInfo);
		std::any GetMemberValue(const MemberInfo &memberInfo) const;
		void SetMemberValue(const MemberInfo &memberInfo,const std::any &value) const;
		const std::vector<MemberInfo> &GetMembers() const;
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

namespace Lua
{
	template<class TLuaClass>
		void register_base_entity_component(TLuaClass &classDef)
	{
		classDef.def(luabind::tostring(luabind::self));
		classDef.def(luabind::constructor<>());
		classDef.def("SetNetworked",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,bool)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,bool bNetworked) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetNetworked(bNetworked);
		}));
		classDef.def("IsNetworked",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->IsNetworked());
		}));
		classDef.def("SetShouldTransmitSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,bool)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,bool bSend) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetShouldTransmitSnapshotData(bSend);
		}));
		classDef.def("ShouldTransmitSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->ShouldTransmitSnapshotData());
		}));
		classDef.def("GetVersion",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushInt(l,hComponent->GetVersion());
		}));
		classDef.def("RegisterNetEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushInt(l,hComponent->SetupNetEvent(name));
		}));
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,CallbackHandle&,uint32_t,BaseEntityComponentHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,CallbackHandle &hCb,uint32_t callbackType,BaseEntityComponentHandle &hComponentOther) {
			pragma::Lua::check_component(l,hComponent);
			pragma::Lua::check_component(l,hComponentOther);
			if(hCb.IsValid() == false)
				return;
			hComponent->FlagCallbackForRemoval(hCb,static_cast<pragma::BaseEntityComponent::CallbackType>(callbackType),hComponentOther.get());
		}));
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,CallbackHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,CallbackHandle &hCb,uint32_t callbackType) {
			pragma::Lua::check_component(l,hComponent);
			if(hCb.IsValid() == false)
				return;
			hComponent->FlagCallbackForRemoval(hCb,static_cast<pragma::BaseEntityComponent::CallbackType>(callbackType));
		}));
		classDef.def("BindComponentInitEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->BindInitComponentEvent(l,componentId,methodNameOrFunction);
		}));
		classDef.def("BindEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t eventId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->BindEvent(l,eventId,methodNameOrFunction);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->GetEntity().AddComponent(componentId);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->BindInitComponentEvent(l,componentId,methodNameOrFunction);
			hComponent->GetEntity().AddComponent(componentId);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->GetEntity().AddComponent(name);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			auto hNewComponent = hComponent->GetEntity().AddComponent(name);
			if(hNewComponent.expired())
				return;
			hComponent->BindInitComponentEvent(hNewComponent->GetComponentId(),methodNameOrFunction);
		}));
		classDef.def("OnMemberValueChanged",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t memberIndex) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->OnMemberValueChanged(memberIndex);
		}));
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object,uint32_t,uint32_t)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault,uint32_t memberFlags,uint32_t version) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,static_cast<pragma::BaseLuaBaseEntityComponent::MemberFlags>(memberFlags),version);
		}))];
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object,uint32_t)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault,uint32_t memberFlags) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,static_cast<pragma::BaseLuaBaseEntityComponent::MemberFlags>(memberFlags),0u);
		}))];
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,pragma::BaseLuaBaseEntityComponent::MemberFlags::Default,0u);
		}))];

		classDef.add_static_constant("MEMBER_FLAG_NONE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::None));
		classDef.add_static_constant("MEMBER_FLAG_BIT_PROPERTY",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::PropertyBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::GetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_SETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_STORE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::StoreBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_KEY_VALUE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::KeyValueBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_INPUT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::InputBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_OUTPUT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::OutputBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_NETWORKED",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::NetworkedBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_USE_HAS_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseHasGetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_USE_IS_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseIsGetterBit));
		
		classDef.add_static_constant("MEMBER_FLAG_TRANSMIT_ON_CHANGE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::TransmitOnChange));
		classDef.add_static_constant("MEMBER_FLAG_SNAPSHOT_DATA",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SnapshotData));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::Default));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_NETWORKED",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultNetworked));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_TRANSMIT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultTransmit));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_SNAPSHOT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultSnapshot));

		classDef.def("Initialize",&LuaBaseEntityComponentWrapper::Initialize,&LuaBaseEntityComponentWrapper::default_Initialize);
		classDef.def("OnRemove",&LuaBaseEntityComponentWrapper::OnRemove,&LuaBaseEntityComponentWrapper::default_OnRemove);
		classDef.def("OnEntitySpawn",&LuaBaseEntityComponentWrapper::OnEntitySpawn,&LuaBaseEntityComponentWrapper::default_OnEntitySpawn);
		classDef.def("OnAttachedToEntity",&LuaBaseEntityComponentWrapper::OnAttachedToEntity,&LuaBaseEntityComponentWrapper::default_OnAttachedToEntity);
		classDef.def("OnDetachedFromEntity",&LuaBaseEntityComponentWrapper::OnDetachedToEntity,&LuaBaseEntityComponentWrapper::default_OnDetachedToEntity);
		// HandleEvent is variadic and can't be defined like this in luabind!
		//classDef.def("HandleEvent",&LuaBaseEntityComponentWrapper::HandleEvent,&LuaBaseEntityComponentWrapper::default_HandleEvent);
		classDef.def("Save",&LuaBaseEntityComponentWrapper::Save,&LuaBaseEntityComponentWrapper::default_Save);
		classDef.def("Load",&LuaBaseEntityComponentWrapper::Load,&LuaBaseEntityComponentWrapper::default_Load);
	}
};

#endif
