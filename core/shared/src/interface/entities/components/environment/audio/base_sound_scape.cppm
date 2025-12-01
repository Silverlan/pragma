// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.base_sound_scape;

export import :entities.components.base;

export namespace pragma {
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
