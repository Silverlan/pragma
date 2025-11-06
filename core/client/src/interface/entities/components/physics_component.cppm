// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.physics;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CPhysicsComponent final : public BasePhysicsComponent, public CBaseNetComponent, public nwm::VelocityCorrection {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		CPhysicsComponent(pragma::ecs::BaseEntity &ent) : BasePhysicsComponent(ent) {}
		virtual void Initialize() override;

		virtual void PrePhysicsSimulate() override;
		virtual bool PostPhysicsSimulate() override;

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;

		virtual void OnWake() override;
		virtual void OnSleep() override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
