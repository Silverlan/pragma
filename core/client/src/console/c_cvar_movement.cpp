// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/console/c_cvar_movement.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include <pragma/networking/enums.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void Console::commands::forward_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveForward, true, magnitude);
}

void Console::commands::forward_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveForward, false);
}

void Console::commands::backward_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveBackward, true, magnitude);
}

void Console::commands::backward_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveBackward, false);
}

void Console::commands::left_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveLeft, true, magnitude);
}

void Console::commands::left_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveLeft, false);
}

void Console::commands::right_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &, float magnitude)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveRight, true, magnitude);
}

void Console::commands::right_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::MoveRight, false);
}

void Console::commands::sprint_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Sprint, true);
}

void Console::commands::sprint_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Sprint, false);
}

void Console::commands::walk_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Walk, true);
}

void Console::commands::walk_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Walk, false);
}

void Console::commands::jump_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Jump, true);
}

void Console::commands::jump_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Jump, false);
}

void Console::commands::crouch_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Crouch, true);
}

void Console::commands::crouch_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Crouch, false);
}

void Console::commands::attack_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack, true);
}

void Console::commands::attack_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack, false);
}

void Console::commands::attack2_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack2, true);
}

void Console::commands::attack2_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack2, false);
}

void Console::commands::attack3_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack3, true);
}

void Console::commands::attack3_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack3, false);
}

void Console::commands::attack4_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack4, true);
}

void Console::commands::attack4_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Attack4, false);
}

void Console::commands::reload_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Reload, true);
}

void Console::commands::reload_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Reload, false);
}

void Console::commands::use_in(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Use, true);
}

void Console::commands::use_out(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!state->IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(state->GetGameState());
	game->SetActionInput(Action::Use, false);
}

void Console::commands::noclip(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	CHECK_CHEATS("noclip", state, );
	ClientState *client = static_cast<ClientState *>(state);
	client->SendPacket("noclip", pragma::networking::Protocol::SlowReliable);
}

void Console::commands::notarget(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	CHECK_CHEATS("notarget", state, );
	ClientState *client = static_cast<ClientState *>(state);
	client->SendPacket("notarget", pragma::networking::Protocol::SlowReliable);
}

void Console::commands::godmode(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	CHECK_CHEATS("godmode", state, );
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("godmode", pragma::networking::Protocol::SlowReliable);
}

void Console::commands::suicide(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	CHECK_CHEATS("suicide", state, );
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("suicide", pragma::networking::Protocol::SlowReliable);
}

void Console::commands::hurtme(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &args)
{
	CHECK_CHEATS("hurtme", state, );
	auto dmg = !args.empty() ? atoi(args.front().c_str()) : 1;
	NetPacket p;
	p->Write<uint16_t>(static_cast<uint16_t>(dmg));
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("hurtme", p, pragma::networking::Protocol::SlowReliable);
}

void Console::commands::give_weapon(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty() || !client->IsGameActive())
		return;
	CHECK_CHEATS("give_weapon", state, );
	NetPacket p;
	p->WriteString(argv.front());
	client->SendPacket("give_weapon", p, pragma::networking::Protocol::SlowReliable);
}

void Console::commands::strip_weapons(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(!client->IsGameActive())
		return;
	CHECK_CHEATS("strip_weapons", state, );
	NetPacket p;
	client->SendPacket("strip_weapons", p, pragma::networking::Protocol::SlowReliable);
}

void Console::commands::next_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("weapon_next", pragma::networking::Protocol::FastUnreliable);
}

void Console::commands::previous_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &args)
{
	auto *client = static_cast<ClientState *>(state);
	client->SendPacket("weapon_previous", pragma::networking::Protocol::FastUnreliable);
}

void Console::commands::give_ammo(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty() || !client->IsGameActive())
		return;
	uint32_t amount = (argv.size() > 1) ? util::to_int(argv[1]) : 50;
	CHECK_CHEATS("give_ammo", state, );
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
		cbCalcView = c_game->AddCallback("CalcView", FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>::Create([](std::reference_wrapper<Vector3>, std::reference_wrapper<Quat> rot, std::reference_wrapper<Quat> rotMod) {
			const auto tFactor = 0.016f; // 60 FPS as reference
			auto t = c_game->DeltaRealTime() / tFactor;
			EulerAngles angVertical(-turn_speeds.up + turn_speeds.down, 0.f, 0.f);
			EulerAngles angHorizontal(0.f, -turn_speeds.right + turn_speeds.left, 0.f);
			rot.get() = uquat::create(angHorizontal * t) * rot.get() * uquat::create(angVertical * t);
		}));
	}
	if(!cbGameEnd.IsValid()) {
		cbGameEnd = c_game->AddCallback("OnGameEnd", FunctionCallback<void, CGame *>::Create([](CGame *) {
			turn_speeds.left = 0.f;
			turn_speeds.right = 0.f;
			turn_speeds.up = 0.f;
			turn_speeds.down = 0.f;
			update_turn_callbacks();
		}));
	}
}

void Console::commands::turn_left_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.left = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
void Console::commands::turn_left_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.left = 0.f;
	update_turn_callbacks();
}
void Console::commands::turn_right_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.right = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
void Console::commands::turn_right_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.right = 0.f;
	update_turn_callbacks();
}
void Console::commands::turn_up_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.up = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
void Console::commands::turn_up_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.up = 0.f;
	update_turn_callbacks();
}
void Console::commands::turn_down_in(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float magnitude)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.down = (!argv.empty() ? ustring::to_float(argv.front()) : defaultTurnSpeed) * magnitude;
	update_turn_callbacks();
}
void Console::commands::turn_down_out(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(!client->IsGameActive())
		return;
	turn_speeds.down = 0.f;
	update_turn_callbacks();
}
