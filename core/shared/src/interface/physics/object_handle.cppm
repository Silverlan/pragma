// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "sharedutils/util_shared_handle.hpp"

export module pragma.shared:physics.object_handle;

export {
	class PhysObj;
	using PhysObjHandle = util::TWeakSharedHandle<PhysObj>;
};
