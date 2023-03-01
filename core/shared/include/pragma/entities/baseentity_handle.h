/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEENTITY_HANDLE_H__
#define __BASEENTITY_HANDLE_H__

#include <sharedutils/util_shared_handle.hpp>

class BaseEntity;
using EntityHandle = util::TWeakSharedHandle<BaseEntity>;

#endif
