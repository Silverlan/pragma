// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;
Bool pragma::ClientState::RawMouseInput(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawMouseInput(button, state, mods);
}
Bool pragma::ClientState::RawKeyboardInput(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods, float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawKeyboardInput(key, scanCode, state, mods, magnitude);
}
Bool pragma::ClientState::RawCharInput(unsigned int c)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawCharInput(c);
}
Bool pragma::ClientState::RawScrollInput(Vector2 offset)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawScrollInput(offset);
}

Bool pragma::ClientState::MouseInput(platform::MouseButton button, platform::KeyState action, platform::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->MouseInput(button, action, mods);
}
Bool pragma::ClientState::KeyboardInput(platform::Key key, int scancode, platform::KeyState action, platform::Modifier mods, float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->KeyboardInput(key, scancode, action, mods, magnitude);
}
Bool pragma::ClientState::CharInput(unsigned int c)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->CharInput(c);
}
Bool pragma::ClientState::ScrollInput(Vector2 offset)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->ScrollInput(offset);
}
void pragma::ClientState::OnFilesDropped(std::vector<std::string> &files)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnFilesDropped(files);
}
void pragma::ClientState::OnDragEnter(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnDragEnter(window);
}
void pragma::ClientState::OnDragExit(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnDragExit(window);
}
bool pragma::ClientState::OnWindowShouldClose(prosper::Window &window)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return false;
	return game->OnWindowShouldClose(window);
}
void pragma::ClientState::OnPreedit(prosper::Window &window, const string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnPreedit(window, preeditString, blockSizes, focusedBlock, caret);
}
void pragma::ClientState::OnIMEStatusChanged(prosper::Window &window, bool imeEnabled)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return;
	game->OnIMEStatusChanged(window, imeEnabled);
}
