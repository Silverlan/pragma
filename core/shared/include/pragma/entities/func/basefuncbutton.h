/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCBUTTON_H__
#define __BASEFUNCBUTTON_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
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
		std::shared_ptr<ALSound> m_useSound = nullptr;
	};
};

#endif
