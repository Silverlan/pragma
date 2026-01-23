// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.base_microphone;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvMicrophoneComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
	};
};
