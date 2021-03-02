/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__
#define __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__

#include <sharedutils/util_weak_handle.hpp>
#include "pragma/networkdefinitions.h"

namespace pragma {class BaseEntityComponent;};
struct DLLNETWORK BaseEntityComponentHandleWrapper
{
	BaseEntityComponentHandleWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &wh)
		: handle(wh)
	{}
	BaseEntityComponentHandleWrapper()=default;
	util::WeakHandle<pragma::BaseEntityComponent> handle = {};
	bool expired() const {return handle.expired();}

	pragma::BaseEntityComponent *operator->() {return get();}
	const pragma::BaseEntityComponent *get() const {return handle.get();}
	pragma::BaseEntityComponent *get() {return handle.get();}
};

#endif
