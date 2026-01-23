// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void forward_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveForward, true, magnitude);
}

static void forward_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveForward, false);
}

static void backward_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveBackward, true, magnitude);
}

static void backward_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveBackward, false);
}

static void left_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveLeft, true, magnitude);
}

static void left_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveLeft, false);
}

static void right_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveRight, true, magnitude);
}

static void right_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::MoveRight, false);
}

static void sprint_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Sprint, true);
}

static void sprint_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Sprint, false);
}

static void walk_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Walk, true);
}

static void walk_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Walk, false);
}

static void jump_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Jump, true);
}

static void jump_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Jump, false);
}

static void crouch_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Crouch, true);
}

static void crouch_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Crouch, false);
}

static void attack_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack, true);
}

static void attack_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack, false);
}

static void attack2_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack2, true);
}

static void attack2_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack2, false);
}

static void attack3_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack3, true);
}

static void attack3_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack3, false);
}

static void attack4_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack4, true);
}

static void attack4_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Attack4, false);
}

static void reload_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Reload, true);
}

static void reload_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Reload, false);
}

static void use_in(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Use, true);
}

static void use_out(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	auto *game = static_cast<pragma::CGame *>(state->GetGameState());
	game->SetActionInput(pragma::Action::Use, false);
}

static void noclip(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("noclip", state))
		return;
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::NOCLIP, pragma::networking::Protocol::SlowReliable);
}

static void notarget(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("notarget", state))
		return;
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::NOTARGET, pragma::networking::Protocol::SlowReliable);
}

static void godmode(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("godmode", state))
		return;
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::GODMODE, pragma::networking::Protocol::SlowReliable);
}

static void suicide(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("suicide", state))
		return;
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::SUICIDE, pragma::networking::Protocol::SlowReliable);
}

static void hurtme(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &args)
{
	if(!check_cheats("hurtme", state))
		return;
	auto dmg = !args.empty() ? pragma::string::to_int(args.front()) : 1;
	NetPacket p;
	p->Write<uint16_t>(static_cast<uint16_t>(dmg));
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::HURTME, p, pragma::networking::Protocol::SlowReliable);
}

static void give_weapon(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(argv.empty() || !client->IsGameActive())
		return;
	if(!check_cheats("give_weapon", state))
		return;
	NetPacket p;
	p->WriteString(argv.front());
	client->SendPacket(pragma::networking::net_messages::server::GIVE_WEAPON, p, pragma::networking::Protocol::SlowReliable);
}

static void strip_weapons(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	if(!check_cheats("strip_weapons", state))
		return;
	NetPacket p;
	client->SendPacket(pragma::networking::net_messages::server::STRIP_WEAPONS, p, pragma::networking::Protocol::SlowReliable);
}

static void next_weapon(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::WEAPON_NEXT, pragma::networking::Protocol::FastUnreliable);
}

static void previous_weapon(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<pragma::ClientState *>(state);
	client->SendPacket(pragma::networking::net_messages::server::WEAPON_PREVIOUS, pragma::networking::Protocol::FastUnreliable);
}

static void give_ammo(pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(argv.empty() || !client->IsGameActive())
		return;
	uint32_t amount = (argv.size() > 1) ? pragma::util::to_int(argv[1]) : 50;
	if(!check_cheats("give_ammo", state))
		return;
	NetPacket p;
	p->WriteString(argv.front());
	p->Write<uint32_t>(amount);
	client->SendPacket(pragma::networking::net_messages::server::GIVE_AMMO, p, pragma::networking::Protocol::SlowReliable);
}

const float defaultTurnSpeed = 3.f;
static struct {
	float left = 0.f;
	float right = 0.f;
	float up = 0.f;
	float down = 0.f;
} turn_speeds;
static CallbackHandle cbCalcView = {};
static CallbackHandle cbGameEnd = {};
static void update_turn_callbacks()
{
	if(turn_speeds.left == 0.f && turn_speeds.right == 0.f && turn_speeds.up == 0.f && turn_speeds.down == 0.f) {
		if(cbCalcView.IsValid())
			cbCalcView.Remove();
		if(cbGameEnd.IsValid())
			cbGameEnd.Remove();
		return;
	}
	if(!cbCalcView.IsValid()) {
		cbCalcView
		  = pragma::get_cgame()->AddCallback("CalcView", FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>::Create([](std::reference_wrapper<Vector3>, std::reference_wrapper<Quat> rot, std::reference_wrapper<Quat> rotMod) {
			    const auto tFactor = 0.016f; // 60 FPS as reference
			    auto t = pragma::get_cgame()->DeltaRealTime() / tFactor;
			    EulerAngles angVertical(-turn_speeds.up + turn_speeds.down, 0.f, 0.f);
			    EulerAngles angHorizontal(0.f, -turn_speeds.right + turn_speeds.left, 0.f);
			    rot.get() = uquat::create(angHorizontal * t) * rot.get() * uquat::create(angVertical * t);
		    }));
	}
	if(!cbGameEnd.IsValid()) {
		cbGameEnd = pragma::get_cgame()->AddCallback("OnGameEnd", FunctionCallback<void, pragma::CGame *>::Create([](pragma::CGame *) {
			turn_speeds.left = 0.f;
			turn_speeds.right = 0.f;
			turn_speeds.up = 0.f;
			turn_speeds.down = 0.f;
			update_turn_callbacks();
		}));
	}
}

static void turn_left_in(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.left = (!argv.empty() ? pragma::string::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_left_out(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.left = 0.f;
	update_turn_callbacks();
}
static void turn_right_in(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.right = (!argv.empty() ? pragma::string::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_right_out(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.right = 0.f;
	update_turn_callbacks();
}
static void turn_up_in(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.up = (!argv.empty() ? pragma::string::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_up_out(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.up = 0.f;
	update_turn_callbacks();
}
static void turn_down_in(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.down = (!argv.empty() ? pragma::string::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_down_out(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.down = 0.f;
	update_turn_callbacks();
}

namespace {
	using namespace pragma::console::client;

	auto UVN = register_command("noclip", &noclip, pragma::console::ConVarFlags::None, "Toggles noclip mode. Requires cheats to be enabled (Or single player mode running).");
	auto UVN = register_command("notarget", &notarget, pragma::console::ConVarFlags::None, "Toggles notarget mode. Requires cheats to be enabled (Or single player mode running).");
	auto UVN = register_command("godmode", &godmode, pragma::console::ConVarFlags::None, "Toggles god mode. Requires cheats to be enabled (Or single player mode running).");
	auto UVN = register_command("give_weapon", &give_weapon, pragma::console::ConVarFlags::None, "Gives the local player the specified weapon and deploys it.");
	auto UVN = register_command("strip_weapons", &strip_weapons, pragma::console::ConVarFlags::None, "Strips all weapons from the local player.");
	auto UVN = register_command("next_weapon", &next_weapon, pragma::console::ConVarFlags::None, "Switches to the next weapon in the player's inventory.");
	auto UVN = register_command("previous_weapon", &previous_weapon, pragma::console::ConVarFlags::None, "Switches to the previous weapon in the player's inventory.");
	auto UVN = register_command("give_ammo", &give_ammo, pragma::console::ConVarFlags::None, "Gives the local player the specified ammunition.");
	auto UVN = register_command("suicide", &suicide, pragma::console::ConVarFlags::None, "Commit suicide.");
	auto UVN = register_command("hurtme", &hurtme, pragma::console::ConVarFlags::None, "Applies the specified amount of damage to self.");

	auto UVN = register_toggle_command("sprint", &sprint_in, &sprint_out, pragma::console::ConVarFlags::None, "Enables sprinting while moving forward.");
	auto UVN = register_toggle_command("walk", &walk_in, &walk_out, pragma::console::ConVarFlags::None, "Enables walking while moving forward.");
	auto UVN = register_toggle_command("jump", &jump_in, &jump_out, pragma::console::ConVarFlags::None, "Prompts the player character to jump.");
	auto UVN = register_toggle_command("crouch", &crouch_in, &crouch_out, pragma::console::ConVarFlags::None, "Prompts the player character to crouch.");
	auto UVN = register_toggle_command("attack", &attack_in, &attack_out, pragma::console::ConVarFlags::None, "Triggers the primary fire for the player character's equipped weapon.");
	auto UVN = register_toggle_command("attack2", &attack2_in, &attack2_out, pragma::console::ConVarFlags::None, "Triggers the secondary fire for the player character's equipped weapon.");
	auto UVN = register_toggle_command("attack3", &attack3_in, &attack3_out, pragma::console::ConVarFlags::None, "Triggers the tertiary fire for the player character's equipped weapon.");
	auto UVN = register_toggle_command("attack4", &attack4_in, &attack4_out, pragma::console::ConVarFlags::None, "Triggers the quaternary fire for the player character's equipped weapon.");
	auto UVN = register_toggle_command("reload", &reload_in, &reload_out, pragma::console::ConVarFlags::None, "Triggers the reload for the player character's equipped weapon.");
	auto UVN = register_toggle_command("use", &use_in, &use_out, pragma::console::ConVarFlags::None, "Prompts the player character to use whatever is in front of him.");
	auto UVN = register_toggle_command("forward", &forward_in, &forward_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Moves the player character forward while active.");
	auto UVN = register_toggle_command("backward", &backward_in, &backward_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Moves the player character backward while active.");
	auto UVN = register_toggle_command("left", &left_in, &left_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Moves the player character left while active.");
	auto UVN = register_toggle_command("right", &right_in, &right_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Moves the player character right while active.");
	auto UVN = register_toggle_command("turn_left", &turn_left_in, &turn_left_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Continuously turns the player character leftwards.");
	auto UVN = register_toggle_command("turn_right", &turn_right_in, &turn_right_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Continuously turns the player character leftwards.");
	auto UVN = register_toggle_command("turn_up", &turn_up_in, &turn_up_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Continuously turns the player character leftwards.");
	auto UVN = register_toggle_command("turn_down", &turn_down_in, &turn_down_out, pragma::console::ConVarFlags::JoystickAxisContinuous, "Continuously turns the player character leftwards.");
}
