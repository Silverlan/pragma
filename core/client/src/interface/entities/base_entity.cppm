// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include <format>

export module pragma.client:entities.base_entity;

export import pragma.shared;

export namespace pragma {
	class CRenderComponent;
	class CSceneComponent;
};
namespace cBaseEntity {
	using namespace pragma::ecs::baseEntity;
	CLASS_ENUM_COMPAT pragma::ComponentEventId EVENT_ON_SCENE_FLAGS_CHANGED;
}
export namespace pragma::ecs {
	class DLLCLIENT CBaseEntity : public BaseEntity {
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CBaseEntity();
		void Construct(unsigned int idx, unsigned int clientIdx);
		virtual void InitializeLuaObject(lua::State *lua) override;

		virtual pragma::ComponentHandle<pragma::BaseAnimatedComponent> GetAnimatedComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseWeaponComponent> GetWeaponComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseVehicleComponent> GetVehicleComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseAIComponent> GetAIComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseCharacterComponent> GetCharacterComponent() const override;
		virtual pragma::ComponentHandle<pragma::BasePlayerComponent> GetPlayerComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseTimeScaleComponent> GetTimeScaleComponent() const override;
		virtual pragma::ComponentHandle<pragma::BaseNameComponent> GetNameComponent() const override;
		virtual bool IsCharacter() const override;
		virtual bool IsPlayer() const override;
		virtual bool IsWeapon() const override;
		virtual bool IsVehicle() const override;
		virtual bool IsNPC() const override;

		// Returns the server-side representation of this entity (If the entity isn't clientside only)
		// This only works for single-player / listen servers!
		pragma::ecs::BaseEntity *GetServersideEntity() const;

		pragma::CRenderComponent *GetRenderComponent() const;

		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet);
		virtual void ReceiveSnapshotData(NetPacket &packet);
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet);

		virtual void Remove() override;
		virtual void OnRemove() override;

		bool IsClientsideOnly() const;
		virtual bool IsNetworkLocal() const override;

		unsigned int GetClientIndex();
		virtual uint32_t GetLocalIndex() const override;

		uint32_t GetSceneFlags() const;
		const util::PUInt32Property &GetSceneFlagsProperty() const;
		void AddToScene(pragma::CSceneComponent &scene);
		void RemoveFromScene(pragma::CSceneComponent &scene);
		void RemoveFromAllScenes();
		bool IsInScene(const pragma::CSceneComponent &scene) const;
		std::vector<pragma::CSceneComponent *> GetScenes() const;

		void AddChild(CBaseEntity &ent);

		// Quick-access
		const bounding_volume::AABB &GetLocalRenderBounds() const;
		const bounding_volume::AABB &GetAbsoluteRenderBounds(bool updateBounds = true) const;
		//

		void SendNetEventTCP(UInt32 eventId, NetPacket &data) const;
		void SendNetEventTCP(UInt32 eventId) const;
		void SendNetEventUDP(UInt32 eventId, NetPacket &data) const;
		void SendNetEventUDP(UInt32 eventId) const;

		virtual pragma::NetworkState *GetNetworkState() const override final;
	protected:
		virtual void DoSpawn() override;
		virtual void OnComponentAdded(pragma::BaseEntityComponent &component) override;
		virtual void OnComponentRemoved(pragma::BaseEntityComponent &component) override;

		friend pragma::BaseEntityComponent;
		uint32_t m_clientIdx = 0u;
		util::PUInt32Property m_sceneFlags = nullptr;
		pragma::CRenderComponent *m_renderComponent = nullptr;
	};
}

export {
	inline DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, pragma::ecs::CBaseEntity &ent) { return ent.print(os); }

	template<>
	struct std::formatter<pragma::ecs::CBaseEntity> : std::formatter<std::string> {
		auto format(pragma::ecs::CBaseEntity &ent, format_context &ctx) -> decltype(ctx.out())
		{
			std::stringstream ss;
			ent.print(ss);
			return std::format_to(ctx.out(), "{}", ss.str());
		}
	};
};
