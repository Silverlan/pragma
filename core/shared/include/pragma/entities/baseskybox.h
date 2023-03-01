/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASESKYBOX_H__
#define __BASESKYBOX_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/console/conout.h>

namespace pragma {
	class DLLNETWORK BaseSkyboxComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		virtual void SetSkyAngles(const EulerAngles &ang);
		const EulerAngles &GetSkyAngles() const;
	  protected:
		EulerAngles m_skyAngles = {};
		pragma::NetEventId m_netEvSetSkyAngles = pragma::INVALID_NET_EVENT;
	};
};

#endif
