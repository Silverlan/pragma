// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void forward_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveForward, true, magnitude);
}

static void forward_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveForward, false);
}

static void backward_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveBackward, true, magnitude);
}

static void backward_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveBackward, false);
}

static void left_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveLeft, true, magnitude);
}

static void left_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveLeft, false);
}

static void right_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveRight, true, magnitude);
}

static void right_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveRight, false);
}

static void sprint_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Sprint, true);
}

static void sprint_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Sprint, false);
}

static void walk_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Walk, true);
}

static void walk_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Walk, false);
}

static void jump_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Jump, true);
}

static void jump_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Jump, false);
}

static void crouch_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Crouch, true);
}

static void crouch_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Crouch, false);
}

static void attack_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack, true);
}

static void attack_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack, false);
}

static void attack2_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack2, true);
}

static void attack2_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack2, false);
}

static void attack3_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack3, true);
}

static void attack3_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack3, false);
}

static void attack4_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack4, true);
}

static void attack4_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack4, false);
}

static void reload_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Reload, true);
}

static void reload_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Reload, false);
}

static void use_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Use, true);
}

static void use_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Use, false);
}

static void noclip(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("noclip", state))
		return;
	ClientState *client = static_cast<ClientState *>(state);
	client->SendPacket("noclip", pragma::networking::Protocol::SlowReliable);
}

static void notarget(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("notarget", state))
		return;
	ClientState *client = static_cast<ClientState *>(state);
	client->SendPacket("notarget", pragma::networking::Protocol::SlowReliable);
}

static void godmode(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("godmode", state))
		return;
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("godmode", pragma::networking::Protocol::SlowReliable);
}

static void suicide(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!check_cheats("suicide", state))
		return;
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("suicide", pragma::networking::Protocol::SlowReliable);
}

static void hurtme(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &args)
{
	if(!check_cheats("hurtme", state))
		return;
	auto dmg = !args.empty() ? atoi(args.front().c_str()) : 1;
	NetPacket p;
	p->Write<uint16_t>(static_cast<uint16_t>(dmg));
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("hurtme", p, pragma::networking::Protocol::SlowReliable);
}

static void give_weapon(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(argv.empty() || !client->IsGameActive())
		return;
	if(!check_cheats("give_weapon", state))
		return;
	NetPacket p;
	p->WriteString(argv.front());
	client->SendPacket("give_weapon", p, pragma::networking::Protocol::SlowReliable);
}

static void strip_weapons(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	if(!check_cheats("strip_weapons", state))
		return;
	NetPacket p;
	client->SendPacket("strip_weapons", p, pragma::networking::Protocol::SlowReliable);
}

static void next_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("weapon_next", pragma::networking::Protocol::FastUnreliable);
}

static void previous_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("weapon_previous", pragma::networking::Protocol::FastUnreliable);
}

static void give_ammo(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(argv.empty() || !client->IsGameActive())
		return;
	uint32_t amount = (argv.size() > 1) ? util::to_int(argv[1]) : 50;
	if(!check_cheats("give_ammo", state))
		return;
	NetPacket p;
	p->WriteString(argv.front());
	p->Write<uint32_t>(amount);
	client->SendPacket("give_ammo", p, pragma::networking::Protocol::SlowReliable);
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
		cbCalcView = pragma::get_cgame()->AddCallback("CalcView", FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>::Create([](std::reference_wrapper<Vector3>, std::reference_wrapper<Quat> rot, std::reference_wrapper<Quat> rotMod) {
			const auto tFactor = 0.016f; // 60 FPS as reference
			auto t = pragma::get_cgame()->DeltaRealTime() / tFactor;
			EulerAngles angVertical(-turn_speeds.up + turn_speeds.down, 0.f, 0.f);
			EulerAngles angHorizontal(0.f, -turn_speeds.right + turn_speeds.left, 0.f);
			rot.get() = uquat::create(angHorizontal * t) * rot.get() * uquat::create(angVertical * t);
		}));
	}
	if(!cbGameEnd.IsValid()) {
		cbGameEnd = pragma::get_cgame()->AddCallback("OnGameEnd", FunctionCallback<void, CGame *>::Create([](CGame *) {
			turn_speeds.left = 0.f;
			turn_speeds.right = 0.f;
			turn_speeds.up = 0.f;
			turn_speeds.down = 0.f;
			update_turn_callbacks();
		}));
	}
}

static void turn_left_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.left = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_left_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.left = 0.f;
	update_turn_callbacks();
}
static void turn_right_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.right = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_right_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.right = 0.f;
	update_turn_callbacks();
}
static void turn_up_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.up = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_up_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.up = 0.f;
	update_turn_callbacks();
}
static void turn_down_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	turn_speeds.down = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
static void turn_down_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
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
