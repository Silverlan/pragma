/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_VEHICLE_COMPONENT_HPP__
#define __S_VEHICLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>

namespace pragma
{
	class DLLSERVER SVehicleComponent final
		: public BaseVehicleComponent,
		public SBaseSnapshotComponent
	{
	public:
		static unsigned int GetVehicleCount();
		static const std::vector<SVehicleComponent*> &GetAll();
		SVehicleComponent(BaseEntity &ent);
		virtual ~SVehicleComponent() override;
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void Initialize() override;
		virtual void Think(double tDelta) override;

		virtual void SetupSteeringWheel(const std::string &mdl,umath::Degree maxSteeringAngle) override;

		virtual bool ShouldTransmitNetData() const override {return true;};
		virtual void SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitSnapshotData() const override;
	protected:
		void WriteWheelInfo(NetPacket &p,WheelData &data);
		CallbackHandle m_playerAction;
		void OnActionInput(Action action, bool b);
		virtual void OnRemove() override;
		void OnPostSpawn();
		void OnUse(BaseEntity *pl);
		virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		static std::vector<SVehicleComponent*> s_vehicles;
	};
};

#endif
