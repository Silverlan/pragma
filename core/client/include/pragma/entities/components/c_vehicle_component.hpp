/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_VEHICLE_COMPONENT_HPP__
#define __C_VEHICLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>
#include <pragma/util/util_handled.hpp>

class CBaseEntity;
namespace pragma
{
	class DLLCLIENT CVehicleComponent final
		: public BaseVehicleComponent,
		public CBaseSnapshotComponent
	{
	public:
		CVehicleComponent(BaseEntity &ent);
		virtual ~CVehicleComponent() override;
		static unsigned int GetVehicleCount();
		static const std::vector<CVehicleComponent*> &GetAll();
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		virtual void ReceiveSnapshotData(NetPacket &packet) override;
	protected:
		void ReadWheelInfo(NetPacket &packet);
		CallbackHandle m_hCbSteeringWheel;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		static std::vector<CVehicleComponent*> s_vehicles;
	};
};

#endif
