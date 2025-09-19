// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client.client_state;

Bool ClientState::RawMouseInput(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawMouseInput(button, state, mods);
}
Bool ClientState::RawKeyboardInput(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods, float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawKeyboardInput(key, scanCode, state, mods, magnitude);
}
Bool ClientState::RawCharInput(unsigned int c)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawCharInput(c);
}
Bool ClientState::RawScrollInput(Vector2 offset)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawScrollInput(offset);
}

Bool ClientState::MouseInput(pragma::platform::MouseButton button, pragma::platform::KeyState action, pragma::platform::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->MouseInput(button, action, mods);
}
Bool ClientState::KeyboardInput(pragma::platform::Key key, int scancode, pragma::platform::KeyState action, pragma::platform::Modifier mods, float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->KeyboardInput(key, scancode, action, mods, magnitude);
}
Bool ClientState::CharInput(unsigned int c)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->CharInput(c);
}
Bool ClientState::ScrollInput(Vector2 offset)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->ScrollInput(offset);
}
void ClientState::OnFilesDropped(std::vector<std::string> &files)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnFilesDropped(files);
}
void ClientState::OnDragEnter(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnDragEnter(window);
}
void ClientState::OnDragExit(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnDragExit(window);
}
bool ClientState::OnWindowShouldClose(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return false;
	return game->OnWindowShouldClose(window);
}
void ClientState::OnPreedit(prosper::Window &window, const pragma::string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnPreedit(window, preeditString, blockSizes, focusedBlock, caret);
}
void ClientState::OnIMEStatusChanged(prosper::Window &window, bool imeEnabled)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnIMEStatusChanged(window, imeEnabled);
}
