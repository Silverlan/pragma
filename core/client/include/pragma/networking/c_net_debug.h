/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_NET_DEBUG_H__
#define __C_NET_DEBUG_H__
#include "pragma/networking/c_net_definitions.h"

DECLARE_NETMESSAGE_CL(debug_drawpoint);
DECLARE_NETMESSAGE_CL(debug_drawline);
DECLARE_NETMESSAGE_CL(debug_drawbox);
DECLARE_NETMESSAGE_CL(debug_drawtext);
DECLARE_NETMESSAGE_CL(debug_drawsphere);
DECLARE_NETMESSAGE_CL(debug_drawcone);
DECLARE_NETMESSAGE_CL(debug_drawaxis);
DECLARE_NETMESSAGE_CL(debug_drawpath);
DECLARE_NETMESSAGE_CL(debug_drawspline);
DECLARE_NETMESSAGE_CL(debug_drawplane);
DECLARE_NETMESSAGE_CL(debug_draw_mesh);
DECLARE_NETMESSAGE_CL(debug_drawtruncatedcone);
DECLARE_NETMESSAGE_CL(debug_drawcylinder);

#endif
