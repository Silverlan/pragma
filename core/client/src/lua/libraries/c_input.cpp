#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"

Bool ClientState::RawMouseInput(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawMouseInput(button,state,mods);
}
Bool ClientState::RawKeyboardInput(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods,float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->RawKeyboardInput(key,scanCode,state,mods,magnitude);
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

Bool ClientState::MouseInput(GLFW::MouseButton button,GLFW::KeyState action,GLFW::Modifier mods)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->MouseInput(button,action,mods);
}
Bool ClientState::KeyboardInput(GLFW::Key key,int scancode,GLFW::KeyState action,GLFW::Modifier mods,float magnitude)
{
	auto *game = GetGameState();
	if(game == nullptr)
		return true;
	return game->KeyboardInput(key,scancode,action,mods,magnitude);
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
