// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__
#define __BASE_ENTITY_COMPONENT_HANDLE_WRAPPER_HPP__

#include <sharedutils/util_weak_handle.hpp>
#include "pragma/networkdefinitions.h"

namespace pragma {
	class BaseEntityComponent;
};
struct DLLNETWORK BaseEntityComponentHandleWrapper {
	BaseEntityComponentHandleWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &wh) : handle(wh) {}
	BaseEntityComponentHandleWrapper() = default;
	util::WeakHandle<pragma::BaseEntityComponent> handle = {};
	bool expired() const { return handle.expired(); }

	pragma::BaseEntityComponent *operator->() { return get(); }
	const pragma::BaseEntityComponent *get() const { return handle.get(); }
	pragma::BaseEntityComponent *get() { return handle.get(); }
};

#endif
