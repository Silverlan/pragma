/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_ENTITY_COMPONENT_HPP__
#define __BASE_ENTITY_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component_handle_wrapper.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/entities/entity_component_event_info.hpp"
#include "pragma/entities/entity_component_info.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/lua/base_lua_handle.hpp"
#include "pragma/types.hpp"
#include <luasystem.h>
#include <unordered_map>
#include <variant>
#include <sharedutils/callback_handler.h>
#include <sharedutils/util_extensible_enum.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>
#include <typeindex>
#include "pragma/entities/entity_component_manager.hpp"

class BaseEntity;
namespace pragma {
	class DLLNETWORK EEntityComponentCallbackEvent : public util::ExtensibleEnum {
	  public:
		using util::ExtensibleEnum::ExtensibleEnum;

		static const EEntityComponentCallbackEvent Count;
	  protected:
		enum class E : uint32_t { Count };
	};
};
DEFINE_STD_HASH_SPECIALIZATION(pragma::EEntityComponentCallbackEvent);
namespace udm {
	struct LinkedPropertyWrapper;
	using LinkedPropertyWrapperArg = const LinkedPropertyWrapper &;
};
class DataStream;
namespace pragma {
	using TRegisterComponentMember = const std::function<ComponentMemberIndex(ComponentMemberInfo &&)> &;
	struct ComponentInfo;
	struct ComponentEvent;
	class BaseEntityComponentSystem;
	class EntityComponentManager;
	struct ComponentMemberInfo;
	using ComponentMemberIndex = uint32_t;

	enum class TickPolicy : uint8_t {
		Never = 0u,
		WhenVisible, // Not yet implemented!
		Always
	};
	struct DLLNETWORK TickData {
		TickPolicy tickPolicy = TickPolicy::Never;
		double lastTick = 0.0;
		double nextTick = 0.0;
	};

	class DLLNETWORK BaseEntityComponent : public pragma::BaseLuaHandle, public std::enable_shared_from_this<BaseEntityComponent> {
	  public:
		// Note: Use BaseEntityComponent::OnEntityComponentAdded to initialize data for other components
		// instead of using this event!
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_ADDED;
		static ComponentEventId EVENT_ON_ENTITY_COMPONENT_REMOVED;
		static ComponentEventId EVENT_ON_MEMBERS_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		enum class StateFlags : uint32_t { None = 0u, IsThinking = 1u, IsLogicEnabled = IsThinking << 1u, Removed = IsLogicEnabled << 1u };

		enum class LogSeverity : uint8_t { Normal = 0, Warning, Error, Critical, Debug };

		BaseEntityComponent(const BaseEntityComponent &) = delete;
		BaseEntityComponent &operator=(const BaseEntityComponent &) = delete;
		BaseEntityComponent(BaseEntityComponent &&) = delete;
		BaseEntityComponent &operator=(BaseEntityComponent &&) = delete;

		virtual ~BaseEntityComponent();
		const BaseEntity &GetEntity() const;
		BaseEntity &GetEntity();
		const BaseEntity &operator->() const;
		BaseEntity &operator->();

		Game &GetGame();
		const Game &GetGame() const { return const_cast<BaseEntityComponent *>(this)->GetGame(); }
		NetworkState &GetNetworkState();
		const NetworkState &GetNetworkState() const { return const_cast<BaseEntityComponent *>(this)->GetNetworkState(); }
		EntityComponentManager &GetComponentManager();
		const EntityComponentManager &GetComponentManager() const { return const_cast<BaseEntityComponent *>(this)->GetComponentManager(); }

		ComponentId GetComponentId() const;
		const ComponentMemberInfo *FindMemberInfo(const std::string &name) const;

		std::optional<ComponentMemberIndex> GetMemberIndex(const std::string &name) const;
		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const;

		bool GetTransformMemberPos(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outPos, bool useOriginObjectSpace = false) const;
		bool GetTransformMemberRot(ComponentMemberIndex idx, umath::CoordinateSpace space, Quat &outRot, bool useOriginObjectSpace = false) const;
		bool GetTransformMemberScale(ComponentMemberIndex idx, umath::CoordinateSpace space, Vector3 &outScale, bool useOriginObjectSpace = false) const;
		bool GetTransformMemberPose(ComponentMemberIndex idx, umath::CoordinateSpace space, umath::ScaledTransform &outPose, bool useOriginObjectSpace = false) const;

		bool SetTransformMemberPos(ComponentMemberIndex idx, umath::CoordinateSpace space, const Vector3 &pos, bool useOriginObjectSpace = false);
		bool SetTransformMemberRot(ComponentMemberIndex idx, umath::CoordinateSpace space, const Quat &rot, bool useOriginObjectSpace = false);
		bool SetTransformMemberScale(ComponentMemberIndex idx, umath::CoordinateSpace space, const Vector3 &scale, bool useOriginObjectSpace = false);
		bool SetTransformMemberPose(ComponentMemberIndex idx, umath::CoordinateSpace space, const umath::ScaledTransform &pose, bool useOriginObjectSpace = false);

		virtual uint32_t GetStaticMemberCount() const;

		const ComponentInfo *GetComponentInfo() const;

		// Component version; Used for loading and saving the component
		virtual uint32_t GetVersion() const { return 1u; }

		// Binds the specified function to an event specifically for this component. The function will be called
		// whenever THIS component has triggered that event.
		CallbackHandle AddEventCallback(ComponentEventId eventId, const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback);
		CallbackHandle AddEventCallback(ComponentEventId eventId, const CallbackHandle &hCallback);
		void RemoveEventCallback(ComponentEventId eventId, const CallbackHandle &hCallback);

		// Invokes all registered event callbacks for this component.
		// Only call this method directly if the event has been registered
		// with the component type! Otherwise callbacks bound through BindEvent
		// will not be invoked for this event type!
		util::EventReply InvokeEventCallbacks(ComponentEventId eventId, const ComponentEvent &evData) const;
		util::EventReply InvokeEventCallbacks(ComponentEventId eventId, ComponentEvent &evData) const;
		util::EventReply InvokeEventCallbacks(ComponentEventId eventId) const;

		// Broadcasts an event to ALL components of the attached entity.
		// This will also invoke event callbacks for this component for the specified event.
		util::EventReply BroadcastEvent(ComponentEventId eventId, const ComponentEvent &evData) const;
		util::EventReply BroadcastEvent(ComponentEventId eventId, ComponentEvent &evData) const;
		util::EventReply BroadcastEvent(ComponentEventId eventId) const;

		// Triggers an event for this component only
		util::EventReply InjectEvent(ComponentEventId eventId, const ComponentEvent &evData);
		util::EventReply InjectEvent(ComponentEventId eventId, ComponentEvent &evData);
		util::EventReply InjectEvent(ComponentEventId eventId);

		// Binds the specified function to an event. The function will be called, whenever the event was broadcasted (or injected)
		CallbackHandle BindEvent(ComponentEventId eventId, const std::function<util::EventReply(std::reference_wrapper<ComponentEvent>)> &fCallback);

		// Same as above, but assumes the callback never 'handles' the event. This is mostly to avoid cases where the return value is omitted by accident.
		CallbackHandle BindEventUnhandled(ComponentEventId eventId, const std::function<void(std::reference_wrapper<ComponentEvent>)> &fCallback);

		virtual void OnAttached(BaseEntity &ent);
		virtual void OnDetached(BaseEntity &ent);

		virtual void Save(udm::LinkedPropertyWrapperArg udm);
		void Load(udm::LinkedPropertyWrapperArg udm);

		virtual void OnEntitySpawn();
		virtual void OnEntityPostSpawn();
		virtual void Initialize();
		virtual void OnRemove();

		ComponentHandle<const BaseEntityComponent> GetHandle() const;
		ComponentHandle<BaseEntityComponent> GetHandle();
		template<class TComponent>
		ComponentHandle<const TComponent> GetHandle() const;
		template<class TComponent>
		ComponentHandle<TComponent> GetHandle();

		enum class CallbackType : uint8_t { Component = 0u, Entity };

		// Helper functions
		pragma::NetEventId SetupNetEvent(const std::string &name) const;
		void FlagCallbackForRemoval(const CallbackHandle &hCallback, CallbackType cbType, BaseEntityComponent *component = nullptr);

		// Do not overwrite these unless the component is a descendant of SBaseNetComponent/SBaseSnapshotComponent respectively!
		virtual bool ShouldTransmitNetData() const;
		virtual bool ShouldTransmitSnapshotData() const;

		// Tick updates
		void SetTickPolicy(TickPolicy policy);
		TickPolicy GetTickPolicy() const;
		bool ShouldThink() const;
		double LastTick() const;
		double GetNextTick() const;
		void SetNextTick(double t);
		double DeltaTime() const;
		bool Tick(double tDelta);
		virtual void OnTick(double tDelta) {}

		// For internal use only!
		StateFlags GetStateFlags() const { return m_stateFlags; }
		void SetStateFlags(StateFlags stateFlags) { m_stateFlags = stateFlags; }

		void Log(const std::string &msg, LogSeverity severity) const;

		std::string GetUri() const;
		std::string GetMemberUri(const std::string &memberName) const;
		std::optional<std::string> GetMemberUri(ComponentMemberIndex memberIdx) const;
		static std::optional<std::string> GetUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier);
		static std::optional<std::string> GetMemberUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier, std::variant<ComponentMemberIndex, std::string> memberIdentifier);
	  protected:
		friend EntityComponentManager;
		friend BaseEntityComponentSystem;
		BaseEntityComponent(BaseEntity &ent);
		void UpdateTickPolicy();
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData);
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version);
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const;
		virtual void OnMembersChanged();

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

		struct CallbackInfo {
			CallbackHandle hCallback;
			BaseEntityComponent *pComponent = nullptr;
		};
		std::vector<CallbackInfo> m_callbackInfos;
		ComponentId m_componentId = std::numeric_limits<ComponentId>::max();

		mutable std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> m_eventCallbacks;
		mutable std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> m_boundEvents;
	  protected:
		void OnEntityComponentAdded(BaseEntityComponent &component, bool bSkipEventBinding);
		BaseEntity &m_entity;

		StateFlags m_stateFlags = StateFlags::None;
		TickData m_tickData {};
	  private:
		friend BaseEntityComponentSystem;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseEntityComponent::StateFlags)

DLLNETWORK std::ostream &operator<<(std::ostream &os, const pragma::BaseEntityComponent &component);

template<class TComponent>
pragma::ComponentHandle<const TComponent> pragma::BaseEntityComponent::GetHandle() const
{
	return pragma::BaseLuaHandle::GetHandle<const TComponent>();
}
template<class TComponent>
pragma::ComponentHandle<TComponent> pragma::BaseEntityComponent::GetHandle()
{
	return pragma::BaseLuaHandle::GetHandle<TComponent>();
}

#include "pragma/lua/converters/entity_component_converter.hpp"

#endif
