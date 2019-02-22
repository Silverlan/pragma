#ifndef __BASE_ENTITY_COMPONENT_HPP__
#define __BASE_ENTITY_COMPONENT_HPP__

#include "pragma/entities/components/base_game_object_component.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/entities/entity_component_info.hpp"
#include "pragma/util/util_handled.hpp"
#include <unordered_map>
#include <sharedutils/callback_handler.h>
#include <sharedutils/util_extensible_enum.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>

class BaseEntity;
namespace pragma
{
	class DLLNETWORK EEntityComponentCallbackEvent
		: public util::ExtensibleEnum
	{
	public:
		using util::ExtensibleEnum::ExtensibleEnum;

		static const EEntityComponentCallbackEvent Count;
	protected:
		enum class E : uint32_t
		{
			Count
		};
	};
};
DEFINE_STD_HASH_SPECIALIZATION(pragma::EEntityComponentCallbackEvent);
class DataStream;
namespace pragma
{
	struct ComponentEvent;
	class BaseEntityComponentSystem;
	class EntityComponentManager;
	class DLLNETWORK BaseEntityComponent
		: public std::enable_shared_from_this<BaseEntityComponent>
	{
	public:
		// Note: Use BaseEntityComponent::OnEntityComponentAdded to initialize data for other components
		// instead of using this event!
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_ADDED;
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_REMOVED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		BaseEntityComponent(const BaseEntityComponent&)=delete;
		BaseEntityComponent &operator=(const BaseEntityComponent&)=delete;
		BaseEntityComponent(BaseEntityComponent&&)=delete;
		BaseEntityComponent &operator=(BaseEntityComponent &&)=delete;

		virtual ~BaseEntityComponent();
		const BaseEntity &GetEntity() const;
		BaseEntity &GetEntity();
		const BaseEntity &operator->() const;
		BaseEntity &operator->();

		ComponentId GetComponentId() const;

		// Component version; Used for loading and saving the component
		virtual uint32_t GetVersion() const {return 1u;}

		// Binds the specified function to an event specifically for this component. The function will be called
		// whenever THIS component has triggered that event.
		CallbackHandle AddEventCallback(ComponentEventId eventId,const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback);
		CallbackHandle AddEventCallback(ComponentEventId eventId,const CallbackHandle &hCallback);
		void RemoveEventCallback(ComponentEventId eventId,const CallbackHandle &hCallback);

		// Invokes all registered event callbacks for this component.
		// Only call this method directly if the event has been registered
		// with the component type! Otherwise callbacks bound through BindEvent
		// will not be invoked for this event type!
		util::EventReply InvokeEventCallbacks(ComponentEventId eventId,ComponentEvent &evData) const;
		util::EventReply InvokeEventCallbacks(ComponentEventId eventId) const;

		// Broadcasts an event to ALL components of the attached entity.
		// This will also invoke event callbacks for this component for the specified event.
		util::EventReply BroadcastEvent(ComponentEventId eventId,ComponentEvent &evData) const;
		util::EventReply BroadcastEvent(ComponentEventId eventId) const;

		// Triggers an event for this component only
		util::EventReply InjectEvent(ComponentEventId eventId,ComponentEvent &evData);
		util::EventReply InjectEvent(ComponentEventId eventId);

		// Binds the specified function to an event. The function will be called, whenever the event was broadcasted (or injected)
		CallbackHandle BindEvent(ComponentEventId eventId,const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback);

		// Same as above, but assumes the callback never 'handles' the event. This is mostly to avoid cases where the return value is omitted by accident.
		CallbackHandle BindEventUnhandled(ComponentEventId eventId,const std::function<void(std::reference_wrapper<ComponentEvent>)> &fCallback);

		virtual void OnAttached(BaseEntity &ent);
		virtual void OnDetached(BaseEntity &ent);

		virtual void Save(DataStream &ds);
		void Load(DataStream &ds);

		virtual void OnEntitySpawn();
		virtual void OnEntityPostSpawn();
		virtual void Initialize();
		virtual void OnRemove();

		const luabind::object &GetLuaObject() const;
		luabind::object &GetLuaObject();
		lua_State *GetLuaState() const;
		void PushLuaObject();
		void PushLuaObject(lua_State *l);

		util::WeakHandle<const BaseEntityComponent> GetHandle() const;
		util::WeakHandle<BaseEntityComponent> GetHandle();
		template<class TComponent>
			util::WeakHandle<const TComponent> GetHandle() const;
		template<class TComponent>
			util::WeakHandle<TComponent> GetHandle();

		enum class CallbackType : uint8_t
		{
			Component = 0u,
			Entity
		};

		// Helper functions
		pragma::NetEventId SetupNetEvent(const std::string &name) const;
		void FlagCallbackForRemoval(const CallbackHandle &hCallback,CallbackType cbType,BaseEntityComponent *component=nullptr);

		// Do not overwrite these unless the component is a descendant of SBaseNetComponent/SBaseSnapshotComponent respectively!
		virtual bool ShouldTransmitNetData() const;
		virtual bool ShouldTransmitSnapshotData() const;
	protected:
		friend EntityComponentManager;
		friend BaseEntityComponentSystem;
		BaseEntityComponent(BaseEntity &ent);
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData);
		virtual void Load(DataStream &ds,uint32_t version);

		// Used for typed callback lookups. If this function doesn't change outTypeIndex, the actual component's type is used
		// as reference. Overwrite this on the serverside or clientside version of the component,
		// whenever the component uses 'InvokeEventCallbacks' with a shared event id
		// on both the client and server, and return the base type. In this case 'RegisterEvent' should always point to the base type as well!
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const;

		// This is called if the argument component has just been added to the entity this component belongs to,
		// or if this component has just been added to an entity (in which case this function is called with 
		// each component the entity already had at that time)
		virtual void OnEntityComponentAdded(BaseEntityComponent &component);
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component);

		void InitializeLuaObject();
		virtual luabind::object InitializeLuaObject(lua_State *l)=0;
		template<class TComponent>
			luabind::object InitializeLuaObject(lua_State *l);

		struct CallbackInfo
		{
			CallbackHandle hCallback;
			BaseEntityComponent *pComponent = nullptr;
		};
		std::vector<CallbackInfo> m_callbackInfos;
		ComponentId m_componentId = std::numeric_limits<ComponentId>::max();

		mutable std::unordered_map<ComponentEventId,std::vector<CallbackHandle>> m_eventCallbacks;
		mutable std::unordered_map<ComponentEventId,std::vector<CallbackHandle>> m_boundEvents;
	protected:
		void OnEntityComponentAdded(BaseEntityComponent &component,bool bSkipEventBinding);
		luabind::object m_luaObj = {};
		BaseEntity &m_entity;
	private:
		friend BaseEntityComponentSystem;
	};
};

template<class THandleWrapper>
	luabind::object pragma::BaseEntityComponent::InitializeLuaObject(lua_State *l)
{
	static_assert(std::is_base_of<BaseEntityComponentHandleWrapper,THandleWrapper>::value,"THandleWrapper must be a descendant of BaseEntityComponentHandleWrapper!");
	return luabind::object(l,THandleWrapper{GetHandle()});
}

template<class TComponent>
	util::WeakHandle<const TComponent> pragma::BaseEntityComponent::GetHandle() const
{
	return util::WeakHandle<const TComponent>(std::static_pointer_cast<const TComponent>(shared_from_this()));
}
template<class TComponent>
	util::WeakHandle<TComponent> pragma::BaseEntityComponent::GetHandle()
{
	return util::WeakHandle<TComponent>(std::static_pointer_cast<TComponent>(shared_from_this()));
}

#endif
