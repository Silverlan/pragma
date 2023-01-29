/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_SOUNDSCAPE_H__
#define __ENV_SOUNDSCAPE_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK BaseEnvSoundScapeComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		std::unordered_map<unsigned int, std::string> m_positions;
		std::string m_kvSoundScape;
		float m_kvRadius = -1.f;
	};
};

#endif
