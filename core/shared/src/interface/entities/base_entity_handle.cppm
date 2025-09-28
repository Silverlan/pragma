// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <sharedutils/util_shared_handle.hpp>

export module pragma.shared:entities.base_entity_handle;

export {
    class BaseEntity;
    using EntityHandle = util::TWeakSharedHandle<BaseEntity>;
};
