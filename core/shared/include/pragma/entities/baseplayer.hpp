/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEPLAYER_HPP__
#define __BASEPLAYER_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_weak_handle.hpp>

namespace pragma {
	class BasePlayerComponent;
};
class DLLNETWORK BasePlayer {
  public:
	BasePlayer() = default;
	virtual ~BasePlayer() = default;
	util::WeakHandle<pragma::BasePlayerComponent> GetBasePlayerComponent();
  protected:
	util::WeakHandle<pragma::BasePlayerComponent> m_basePlayerComponent = {};
};

#endif
