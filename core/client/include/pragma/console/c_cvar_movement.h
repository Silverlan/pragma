/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_CVAR_MOVEMENT_H__
#define __C_CVAR_MOVEMENT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

namespace Console
{
	namespace commands
	{
		DLLCLIENT void sprint_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void sprint_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void walk_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void walk_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void jump_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void jump_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void crouch_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void crouch_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void attack_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void attack_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void attack2_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void attack2_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void attack3_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void attack3_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void attack4_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void attack4_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void reload_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void reload_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void use_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void use_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void noclip(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void notarget(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void godmode(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void next_weapon(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void give_weapon(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void previous_weapon(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void give_ammo(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void suicide(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);
		DLLCLIENT void hurtme(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void forward_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args,float magnitude);
		DLLCLIENT void forward_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void backward_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args,float magnitude);
		DLLCLIENT void backward_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void left_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args,float magnitude);
		DLLCLIENT void left_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void right_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args,float magnitude);
		DLLCLIENT void right_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &args);

		DLLCLIENT void turn_left_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv,float magnitude);
		DLLCLIENT void turn_left_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);

		DLLCLIENT void turn_right_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv,float magnitude);
		DLLCLIENT void turn_right_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);

		DLLCLIENT void turn_up_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv,float magnitude);
		DLLCLIENT void turn_up_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);

		DLLCLIENT void turn_down_in(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv,float magnitude);
		DLLCLIENT void turn_down_out(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
	};
};
REGISTER_CONCOMMAND_TOGGLE_CL(sprint,Console::commands::sprint_in,Console::commands::sprint_out,ConVarFlags::None,"Enables sprinting while moving forward.");
REGISTER_CONCOMMAND_TOGGLE_CL(walk,Console::commands::walk_in,Console::commands::walk_out,ConVarFlags::None,"Enables walking while moving forward.");
REGISTER_CONCOMMAND_TOGGLE_CL(jump,Console::commands::jump_in,Console::commands::jump_out,ConVarFlags::None,"Prompts the player character to jump.");
REGISTER_CONCOMMAND_TOGGLE_CL(crouch,Console::commands::crouch_in,Console::commands::crouch_out,ConVarFlags::None,"Prompts the player character to crouch.");
REGISTER_CONCOMMAND_TOGGLE_CL(attack,Console::commands::attack_in,Console::commands::attack_out,ConVarFlags::None,"Triggers the primary fire for the player character's equipped weapon.");
REGISTER_CONCOMMAND_TOGGLE_CL(attack2,Console::commands::attack2_in,Console::commands::attack2_out,ConVarFlags::None,"Triggers the secondary fire for the player character's equipped weapon.");
REGISTER_CONCOMMAND_TOGGLE_CL(attack3,Console::commands::attack3_in,Console::commands::attack3_out,ConVarFlags::None,"Triggers the tertiary fire for the player character's equipped weapon.");
REGISTER_CONCOMMAND_TOGGLE_CL(attack4,Console::commands::attack4_in,Console::commands::attack4_out,ConVarFlags::None,"Triggers the quaternary fire for the player character's equipped weapon.");
REGISTER_CONCOMMAND_TOGGLE_CL(reload,Console::commands::reload_in,Console::commands::reload_out,ConVarFlags::None,"Triggers the reload for the player character's equipped weapon.");
REGISTER_CONCOMMAND_TOGGLE_CL(use,Console::commands::use_in,Console::commands::use_out,ConVarFlags::None,"Prompts the player character to use whatever is in front of him.");
REGISTER_CONCOMMAND_CL(noclip,Console::commands::noclip,ConVarFlags::None,"Toggles noclip mode. Requires cheats to be enabled (Or single player mode running).");
REGISTER_CONCOMMAND_CL(notarget,Console::commands::notarget,ConVarFlags::None,"Toggles notarget mode. Requires cheats to be enabled (Or single player mode running).");
REGISTER_CONCOMMAND_CL(godmode,Console::commands::godmode,ConVarFlags::None,"Toggles god mode. Requires cheats to be enabled (Or single player mode running).");
REGISTER_CONCOMMAND_CL(give_weapon,Console::commands::give_weapon,ConVarFlags::None,"Gives the local player the specified weapon and deploys it.");
REGISTER_CONCOMMAND_CL(next_weapon,Console::commands::next_weapon,ConVarFlags::None,"Switches to the next weapon in the player's inventory.");
REGISTER_CONCOMMAND_CL(previous_weapon,Console::commands::previous_weapon,ConVarFlags::None,"Switches to the previous weapon in the player's inventory.");
REGISTER_CONCOMMAND_CL(give_ammo,Console::commands::give_ammo,ConVarFlags::None,"Gives the local player the specified ammunition.");
REGISTER_CONCOMMAND_CL(suicide,Console::commands::suicide,ConVarFlags::None,"Commit suicide.");
REGISTER_CONCOMMAND_CL(hurtme,Console::commands::hurtme,ConVarFlags::None,"Applies the specified amount of damage to self.");
REGISTER_CONCOMMAND_TOGGLE_CL(forward,Console::commands::forward_in,Console::commands::forward_out,ConVarFlags::JoystickAxisContinuous,"Moves the player character forward while active.");
REGISTER_CONCOMMAND_TOGGLE_CL(backward,Console::commands::backward_in,Console::commands::backward_out,ConVarFlags::JoystickAxisContinuous,"Moves the player character backward while active.");
REGISTER_CONCOMMAND_TOGGLE_CL(left,Console::commands::left_in,Console::commands::left_out,ConVarFlags::JoystickAxisContinuous,"Moves the player character left while active.");
REGISTER_CONCOMMAND_TOGGLE_CL(right,Console::commands::right_in,Console::commands::right_out,ConVarFlags::JoystickAxisContinuous,"Moves the player character right while active.");
REGISTER_CONCOMMAND_TOGGLE_CL(turn_left,Console::commands::turn_left_in,Console::commands::turn_left_out,ConVarFlags::JoystickAxisContinuous,"Continuously turns the player character leftwards.");
REGISTER_CONCOMMAND_TOGGLE_CL(turn_right,Console::commands::turn_right_in,Console::commands::turn_right_out,ConVarFlags::JoystickAxisContinuous,"Continuously turns the player character leftwards.");
REGISTER_CONCOMMAND_TOGGLE_CL(turn_up,Console::commands::turn_up_in,Console::commands::turn_up_out,ConVarFlags::JoystickAxisContinuous,"Continuously turns the player character leftwards.");
REGISTER_CONCOMMAND_TOGGLE_CL(turn_down,Console::commands::turn_down_in,Console::commands::turn_down_out,ConVarFlags::JoystickAxisContinuous,"Continuously turns the player character leftwards.");

#endif
