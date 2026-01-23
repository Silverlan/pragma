// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:physics.object_handle;

export import :types;
export import pragma.util;

export namespace pragma::physics {
	using PhysObjHandle = util::TWeakSharedHandle<PhysObj>;
};
