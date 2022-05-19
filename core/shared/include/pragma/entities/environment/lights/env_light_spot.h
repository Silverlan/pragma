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

		virtual void SetOuterConeAngle(umath::Degree ang);
		umath::Degree GetOuterConeAngle() const;

		umath::Fraction GetBlendFraction() const;
		virtual void SetBlendFraction(umath::Fraction fraction);

		virtual void SetConeStartOffset(float offset);
		float GetConeStartOffset() const;

		const util::PFloatProperty &GetBlendFractionProperty() const;
		const util::PFloatProperty &GetOuterConeAngleProperty() const;
		const util::PFloatProperty &GetConeStartOffsetProperty() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		util::PFloatProperty m_blendFraction = nullptr;
		util::PFloatProperty m_outerConeAngle = nullptr;
		util::PFloatProperty m_coneStartOffset = nullptr;
		pragma::NetEventId m_netEvSetConeStartOffset = pragma::INVALID_NET_EVENT;
	};
};

#endif