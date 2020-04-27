/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_POINT_AT_TARGET_COMPONENT_HPP__
#define __BASE_POINT_AT_TARGET_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include "pragma/entities/entity_property.hpp"

namespace pragma
{
	class DLLNETWORK BasePointAtTargetComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;
		
		const pragma::PEntityProperty &GetPointAtTargetProperty() const;
		void SetPointAtTarget(BaseEntity &ent);
		void ClearPointAtTarget();
		BaseEntity *GetPointAtTarget() const;
	protected:
		BasePointAtTargetComponent(BaseEntity &ent);
		virtual void OnEntitySpawn() override;
		virtual void SetPointAtTarget(BaseEntity *ent);

		std::string m_kvPointAtTargetName = "";
		pragma::PEntityProperty m_pointAtTarget;
		pragma::NetEventId m_netEvSetPointAtTarget = pragma::INVALID_NET_EVENT;
		CallbackHandle m_cbTick = {};
	};
};

#endif
