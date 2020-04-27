/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_CVAR_SETTINGS_H__
#define __S_CVAR_SETTINGS_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

REGISTER_CONVAR_SV(sv_password,"",ConVarFlags::Archive,"Sets a password for the server. No player will be able to join, unless they provide the correct password using the 'password' command.");
REGISTER_CONVAR_SV(sv_maxplayers,"1",ConVarFlags::Archive,"Specifies the maximum amount of players that are allowed to join the server.");

REGISTER_CONVAR_SV(sv_physics_simulation_enabled,"1",ConVarFlags::Cheat,"Enables or disables physics simulation.");

REGISTER_CONVAR_SV(sv_water_surface_simulation_edge_iteration_count,"5",ConVarFlags::Archive,"The more iterations, the more detailed the water simulation will be, but at a great performance cost.");
REGISTER_CONVAR_SV(sv_water_surface_simulation_shared,"1",ConVarFlags::Archive,"If enabled, water surface simulation will be shared between client and server (Simulation is only performed once). This will only have an effect in single-player or on listen servers.");

#endif