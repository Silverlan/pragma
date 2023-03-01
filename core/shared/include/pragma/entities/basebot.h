/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEBOT_H__
#define __BASEBOT_H__

#include "pragma/networkdefinitions.h"

class BaseEntity;
namespace pragma {
	class DLLNETWORK BaseBotComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
