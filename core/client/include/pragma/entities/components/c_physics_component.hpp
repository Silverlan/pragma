// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PHYSICS_COMPONENT_HPP__
#define __C_PHYSICS_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/networking/nwm_velocity_correction.hpp>

namespace pragma {
	class DLLCLIENT CPhysicsComponent final : public BasePhysicsComponent, public CBaseNetComponent, public nwm::VelocityCorrection {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		CPhysicsComponent(BaseEntity &ent) : BasePhysicsComponent(ent) {}
		virtual void Initialize() override;

		virtual void PrePhysicsSimulate() override;
		virtual bool PostPhysicsSimulate() override;

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;

		virtual void OnWake() override;
		virtual void OnSleep() override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
