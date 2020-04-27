/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PHYSICS_COMPONENT_HPP__
#define __C_PHYSICS_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/networking/nwm_velocity_correction.hpp>

namespace pragma
{
	class DLLCLIENT CPhysicsComponent final
		: public BasePhysicsComponent,
		public CBaseNetComponent,
		public nwm::VelocityCorrection
	{
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		CPhysicsComponent(BaseEntity &ent) : BasePhysicsComponent(ent) {}
		virtual void Initialize() override;

		virtual void PrePhysicsSimulate() override;
		virtual void PostPhysicsSimulate() override;

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;
	protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
