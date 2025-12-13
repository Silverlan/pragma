// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.func.base_button;

export import :audio.sound;
export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseFuncButtonComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void OnEntitySpawn() override;
	  protected:
		std::string m_kvUseSound;
		float m_kvWaitTime = 0.f;
		float m_tNextUse = 0.f;
		std::shared_ptr<audio::ALSound> m_useSound = nullptr;
	};
};
