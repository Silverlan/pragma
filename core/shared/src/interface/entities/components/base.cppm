// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

#ifdef _WIN32
#if __cpp_lib_format >= 202207L
#include <format>

#endif
#endif

export module pragma.shared:entities.components.base;

export import :entities.base_entity_handle;
export import :entities.enums;
export import :entities.components.dynamic_member_register;
export import :entities.components.handle;
export import :entities.components.events.event;
export import :entities.components.events.event_info;
export import :scripting.lua.base_lua_handle;
export import pragma.util;

export namespace pragma {
	class DLLNETWORK EEntityComponentCallbackEvent : public util::ExtensibleEnum {
	  public:
		using ExtensibleEnum::ExtensibleEnum;

		static const EEntityComponentCallbackEvent Count;
	  protected:
		enum class E : uint32_t { Count };
	};
};
export namespace std {
	template<>
	struct hash<pragma::EEntityComponentCallbackEvent> {
		std::size_t operator()(const pragma::EEntityComponentCallbackEvent &object) const { return object.Hash(); }
	};
}

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma {
		class Game;
	}
	namespace pragma {
		class NetworkState;
		class EntityComponentManager;
		class BaseEntityComponentSystem;
		struct ComponentMemberInfo;
		struct ComponentInfo;
		struct ComponentEvent;
		using TRegisterComponentMember = const std::function<ComponentMemberIndex(ComponentMemberInfo &&)> &;

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

		template<typename... Args>
		using format_string_t = spdlog::format_string_t<Args...>;

		namespace baseEntityComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_COMPONENT_ADDED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_COMPONENT_REMOVED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_MEMBERS_CHANGED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ACTIVE_STATE_CHANGED)
		}

		class DLLNETWORK BaseEntityComponent : public BaseLuaHandle, public std::enable_shared_from_this<BaseEntityComponent> {
		  public:
			// Note: Use BaseEntityComponent::OnEntityComponentAdded to initialize data for other components
			// instead of using this event!

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
			template<typename TClass>
			static spdlog::logger &get_logger();
			enum class StateFlags : uint32_t {
				None = 0u,
				IsThinking = 1u,
				IsLogicEnabled = IsThinking << 1u,
				Removed = IsLogicEnabled << 1u,
				CleanedUp = Removed << 1u,
				IsInactive = CleanedUp << 1u,
				IsInitializing = IsInactive << 1u,
			};

			enum class LogSeverity : uint8_t {
				Normal = 0,
				Warning,
				Error,
				Critical,
				Debug,
			};

			BaseEntityComponent(const BaseEntityComponent &) = delete;
			BaseEntityComponent &operator=(const BaseEntityComponent &) = delete;
			BaseEntityComponent(BaseEntityComponent &&) = delete;
			BaseEntityComponent &operator=(BaseEntityComponent &&) = delete;

			virtual ~BaseEntityComponent();
			const ecs::BaseEntity &GetEntity() const;
			ecs::BaseEntity &GetEntity();
			const ecs::BaseEntity &operator->() const;
			ecs::BaseEntity &operator->();

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

			void SetPropertyAnimated(const std::string &property, bool animated);
			bool IsPropertyAnimated(const std::string &property);

			bool GetTransformMemberPos(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outPos) const;
			bool GetTransformMemberRot(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &outRot) const;
			bool GetTransformMemberScale(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outScale) const;
			bool GetTransformMemberPose(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &outPose) const;

			bool ConvertTransformMemberPosToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutPos) const;
			bool ConvertTransformMemberRotToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &inOutRot) const;
			bool ConvertTransformMemberScaleToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutScale) const;
			bool ConvertTransformMemberPoseToTargetSpace(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &inOutPose) const;

			bool GetTransformMemberParentPos(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outPos) const;
			bool GetTransformMemberParentRot(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &outRot) const;
			bool GetTransformMemberParentScale(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &outScale) const;
			bool GetTransformMemberParentPose(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &outPose) const;

			bool ConvertPosToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutPos, math::CoordinateSpace *optOutMemberSpace = nullptr) const;
			bool ConvertRotToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Quat &inOutRot, math::CoordinateSpace *optOutMemberSpace = nullptr) const;
			bool ConvertScaleToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, Vector3 &inOutScale, math::CoordinateSpace *optOutMemberSpace = nullptr) const;
			bool ConvertPoseToMemberSpace(ComponentMemberIndex idx, math::CoordinateSpace space, math::ScaledTransform &inOutPose, math::CoordinateSpace *optOutMemberSpace = nullptr) const;

			bool SetTransformMemberPos(ComponentMemberIndex idx, math::CoordinateSpace space, const Vector3 &pos);
			bool SetTransformMemberRot(ComponentMemberIndex idx, math::CoordinateSpace space, const Quat &rot);
			bool SetTransformMemberScale(ComponentMemberIndex idx, math::CoordinateSpace space, const Vector3 &scale);
			bool SetTransformMemberPose(ComponentMemberIndex idx, math::CoordinateSpace space, const math::ScaledTransform &pose);

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

			virtual void OnAttached(ecs::BaseEntity &ent);
			virtual void OnDetached(ecs::BaseEntity &ent);

			virtual void Save(udm::LinkedPropertyWrapperArg udm);
			void Load(udm::LinkedPropertyWrapperArg udm);

			virtual void OnEntitySpawn();
			virtual void OnEntityPostSpawn();
			virtual void Initialize();
			virtual void PostInitialize();
			virtual void OnRemove();

			ComponentHandle<const BaseEntityComponent> GetHandle() const;
			ComponentHandle<BaseEntityComponent> GetHandle();
			template<class TComponent>
			ComponentHandle<const TComponent> GetHandle() const;
			template<class TComponent>
			ComponentHandle<TComponent> GetHandle();

			enum class CallbackType : uint8_t { Component = 0u, Entity };

			// Helper functions
			NetEventId SetupNetEvent(const std::string &name) const;
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

			template<typename... Args>
			void Log(spdlog::level::level_enum level, const std::string &msg) const;

			template<typename... Args>
			void Log(spdlog::level::level_enum level, format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogTrace(format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogDebug(format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogInfo(format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogWarn(format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogError(format_string_t<Args...> fmt, Args &&...args) const;
			template<typename... Args>
			void LogCritical(format_string_t<Args...> fmt, Args &&...args) const;

			void SetActive(bool enabled);
			bool IsActive() const;
			void Activate();
			void Deactivate();

			std::string GetUri() const;
			std::string GetMemberUri(const std::string &memberName) const;
			std::optional<std::string> GetMemberUri(ComponentMemberIndex memberIdx) const;
			static std::optional<std::string> GetUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier);
			static std::optional<std::string> GetMemberUri(Game *game, std::variant<util::Uuid, std::string> entityIdentifier, std::variant<ComponentId, std::string> componentIdentifier, std::variant<ComponentMemberIndex, std::string> memberIdentifier);
		  protected:
			friend EntityComponentManager;
			friend BaseEntityComponentSystem;
			BaseEntityComponent(ecs::BaseEntity &ent);
			void CleanUp();
			void UpdateTickPolicy();
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData);
			virtual void OnActiveStateChanged(bool active);
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version);
			virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const;
			virtual void OnMembersChanged();
			spdlog::logger &InitLogger() const;
			static spdlog::logger &get_logger(std::type_index typeIndex);

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
			ComponentId m_componentId = std::numeric_limits<ComponentId>::max();

			std::vector<CallbackInfo> &GetCallbackInfos() const;
			std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> &GetEventCallbacks() const;
			std::unordered_map<ComponentEventId, std::vector<CallbackHandle>> &GetBoundEvents() const;
		  protected:
			void OnEntityComponentAdded(BaseEntityComponent &component, bool bSkipEventBinding);
			ecs::BaseEntity &m_entity;

			StateFlags m_stateFlags = StateFlags::None;
			TickData m_tickData {};
		  private:
			friend BaseEntityComponentSystem;

			mutable std::unique_ptr<std::vector<CallbackInfo>> m_callbackInfos;
			mutable std::unique_ptr<std::unordered_map<ComponentEventId, std::vector<CallbackHandle>>> m_eventCallbacks;
			mutable std::unique_ptr<std::unordered_map<ComponentEventId, std::vector<CallbackHandle>>> m_boundEvents;
		};
		DLLNETWORK std::ostream &operator<<(std::ostream &os, const BaseEntityComponent &component);

		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEntityComponent::StateFlags)

	namespace pragma {
		template<class TComponent>
		ComponentHandle<const TComponent> BaseEntityComponent::GetHandle() const
		{
			return BaseLuaHandle::GetHandle<const TComponent>();
		}
		template<class TComponent>
		ComponentHandle<TComponent> BaseEntityComponent::GetHandle()
		{
			return BaseLuaHandle::GetHandle<TComponent>();
		}

		template<typename... Args>
		void BaseEntityComponent::Log(spdlog::level::level_enum level, const std::string &msg) const
		{
			auto &logger = InitLogger();
			logger.log(level, msg);
		}

		template<typename... Args>
		void BaseEntityComponent::Log(spdlog::level::level_enum level, format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.log(level, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogTrace(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.trace(fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogDebug(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.debug(fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogInfo(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.info(fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogWarn(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.warn(fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogError(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.error(fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void BaseEntityComponent::LogCritical(format_string_t<Args...> fmt, Args &&...args) const
		{
			auto &logger = InitLogger();
			logger.critical(fmt, std::forward<Args>(args)...);
		}

		spdlog::logger *find_logger(Game &game, std::type_index typeIndex);

		template<typename TClass>
		spdlog::logger *find_logger(Game &game)
		{
			return find_logger(game, typeid(TClass));
		}

		template<typename TClass>
		spdlog::logger &BaseEntityComponent::get_logger()
		{
			return get_logger(typeid(TClass));
		}
	}
};
