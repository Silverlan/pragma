// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEF_RECIPIENTFILTER_H__
#define __LDEF_RECIPIENTFILTER_H__
#include "pragma/lua/ldefinitions.h"
namespace pragma::networking {
	class TargetRecipientFilter;
};
lua_registercheck(RecipientFilter, pragma::networking::TargetRecipientFilter);
#endif
