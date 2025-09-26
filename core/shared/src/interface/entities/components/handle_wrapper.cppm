// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <sharedutils/util_weak_handle.hpp>
#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.handle_wrapper;

export struct DLLNETWORK BaseEntityComponentHandleWrapper {
	BaseEntityComponentHandleWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &wh) : handle(wh) {}
	BaseEntityComponentHandleWrapper() = default;
	util::WeakHandle<pragma::BaseEntityComponent> handle = {};
	bool expired() const { return handle.expired(); }

	pragma::BaseEntityComponent *operator->() { return get(); }
	const pragma::BaseEntityComponent *get() const { return handle.get(); }
	pragma::BaseEntityComponent *get() { return handle.get(); }
};
