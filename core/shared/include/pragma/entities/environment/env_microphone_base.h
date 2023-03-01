/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_MICROPHONE_BASE_H__
#define __ENV_MICROPHONE_BASE_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK BaseEnvMicrophoneComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
	};
};

#endif
