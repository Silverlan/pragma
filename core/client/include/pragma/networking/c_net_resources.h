/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_NET_RESOURCES_H__
#define __C_NET_RESOURCES_H__

#include "pragma/networking/c_net_definitions.h"

DECLARE_NETMESSAGE_CL(resourceinfo);
DECLARE_NETMESSAGE_CL(resourcecomplete);
DECLARE_NETMESSAGE_CL(resource_fragment);
DECLARE_NETMESSAGE_CL(resource_mdl_rough);

#endif
