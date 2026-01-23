// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.handle_wrapper;

export import pragma.util;

export {
	namespace pragma {
		class BaseEntityComponent;
	};
	struct DLLNETWORK BaseEntityComponentHandleWrapper {
		BaseEntityComponentHandleWrapper(const pragma::util::WeakHandle<pragma::BaseEntityComponent> &wh) : handle(wh) {}
		BaseEntityComponentHandleWrapper() = default;
		pragma::util::WeakHandle<pragma::BaseEntityComponent> handle = {};
		bool expired() const { return handle.expired(); }

		pragma::BaseEntityComponent *operator->() { return get(); }
		const pragma::BaseEntityComponent *get() const { return handle.get(); }
		pragma::BaseEntityComponent *get() { return handle.get(); }
	};
}
