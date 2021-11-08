/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_TIME_SCALE_COMPONENT_HPP__
#define __BASE_TIME_SCALE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseTimeScaleComponent
		: public BaseEntityComponent
	{
	public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);

		virtual void Initialize() override;

		virtual void SetTimeScale(float timeScale);
		float GetTimeScale() const;
		const util::PFloatProperty &GetTimeScaleProperty() const;
		float GetEffectiveTimeScale() const;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
	protected:
		BaseTimeScaleComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetTimeScale = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_timeScale = nullptr;
	};
};

#endif
