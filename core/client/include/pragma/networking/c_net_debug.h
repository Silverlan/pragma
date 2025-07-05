// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
