// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
