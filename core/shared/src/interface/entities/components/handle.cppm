// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "sharedutils/util_shared_handle.hpp"
#include <sharedutils/util_weak_handle.hpp>

export module pragma.shared:entities.components.handle;

export namespace pragma {
	template<class T>
	using ComponentHandle = util::TWeakSharedHandle<T>;
};
