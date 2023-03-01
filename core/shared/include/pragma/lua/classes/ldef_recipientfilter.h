/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDEF_RECIPIENTFILTER_H__
#define __LDEF_RECIPIENTFILTER_H__
#include "pragma/lua/ldefinitions.h"
namespace pragma::networking {
	class TargetRecipientFilter;
};
lua_registercheck(RecipientFilter, pragma::networking::TargetRecipientFilter);
#endif
