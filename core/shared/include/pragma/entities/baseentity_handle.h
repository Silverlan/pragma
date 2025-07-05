// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASEENTITY_HANDLE_H__
#define __BASEENTITY_HANDLE_H__

#include <sharedutils/util_shared_handle.hpp>

class BaseEntity;
using EntityHandle = util::TWeakSharedHandle<BaseEntity>;

#endif
