// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
