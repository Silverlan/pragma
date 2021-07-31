/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEWHEEL_H__
#define __BASEWHEEL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/vehicle.hpp"

namespace pragma
{
	class BaseVehicleComponent;
	class DLLNETWORK BaseWheelComponent
		: public BaseEntityComponent
	{
	public:
		virtual ~BaseWheelComponent() override;
		virtual void Initialize() override;
		util::WeakHandle<pragma::BaseVehicleComponent> GetVehicle();

		void SetupWheel(BaseVehicleComponent &vhc,const pragma::physics::WheelCreateInfo &createInfo,uint8_t wheelId);
	protected:
		BaseWheelComponent(BaseEntity &ent);
		void UpdatePose();
		ComponentHandle<pragma::BaseVehicleComponent> m_vehicle = {};
		uint8_t m_wheelId = 0u;
		physics::WheelCreateInfo m_createInfo = {};
		umath::Transform m_localTransform = {};

		virtual void OnTick(double dt) override;
		void UpdateWheel();
	private:
		CallbackHandle m_cbOnSpawn = {};
	};
};

#endif
