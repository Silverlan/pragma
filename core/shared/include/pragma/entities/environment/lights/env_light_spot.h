/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_LIGHT_SPOT_H__
#define __ENV_LIGHT_SPOT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvLightSpotComponent
		: public BaseEntityComponent
	{
	public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		BaseEnvLightSpotComponent(BaseEntity &ent);
		virtual void Initialize() override;
		// Changes the half-angle for for outer cone
		virtual void SetOuterCutoffAngle(umath::Degree ang);
		// Changes the half-angle for for inner cone
		virtual void SetInnerCutoffAngle(umath::Degree ang);

		// The half-angle for for outer cone
		umath::Degree GetOuterCutoffAngle() const;
		// The half-angle for for inner cone
		umath::Degree GetInnerCutoffAngle() const;

		virtual void SetConeStartOffset(float offset);
		float GetConeStartOffset() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		util::PFloatProperty m_angInnerCutoff = nullptr;
		util::PFloatProperty m_angOuterCutoff = nullptr;
		util::PFloatProperty m_coneStartOffset = nullptr;
		pragma::NetEventId m_netEvSetConeStartOffset = pragma::INVALID_NET_EVENT;
	};
};

#endif