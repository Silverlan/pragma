/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"

Bool ClientState::RawMouseInput(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawMouseInput(button, state, mods);
}
Bool ClientState::RawKeyboardInput(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods, float magnitude)
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

Bool ClientState::MouseInput(GLFW::MouseButton button, GLFW::KeyState action, GLFW::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->MouseInput(button, action, mods);
}
Bool ClientState::KeyboardInput(GLFW::Key key, int scancode, GLFW::KeyState action, GLFW::Modifier mods, float magnitude)
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
