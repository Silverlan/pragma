/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_LIGHT_POINT_H__
#define __ENV_LIGHT_POINT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvLightPointComponent
		: public BaseEntityComponent
	{
	public:
		static Candela CalcIntensityAtPoint(
			const Vector3 &lightPos,Candela intensity,const Vector3 &point,std::optional<float> radius={}
		);

		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		float CalcDistanceFalloff(const Vector3 &point) const;
	protected:
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
	};
};

#endif