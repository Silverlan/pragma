// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:entities.components.handle;

export import pragma.util;

export namespace pragma {
	template<class T>
	using ComponentHandle = util::TWeakSharedHandle<T>;
};
