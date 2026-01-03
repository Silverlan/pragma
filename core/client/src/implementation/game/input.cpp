// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :game;
import :engine;
import pragma.string.unicode;

Bool pragma::CGame::RawMouseInput(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	if(m_inputCallbackHandler.CallLuaEvents<int, int, int>("OnMouseInput", static_cast<int>(button), static_cast<int>(state), static_cast<int>(mods)) == util::EventReply::Handled)
		return false;
	return true;
}
Bool pragma::CGame::RawKeyboardInput(platform::Key key, int, platform::KeyState state, platform::Modifier mods, float magnitude)
{
	if(m_inputCallbackHandler.CallLuaEvents<int, int, int>("OnKeyboardInput", static_cast<int>(key), static_cast<int>(state), static_cast<int>(mods)) == util::EventReply::Handled)
		return false;
	return true;
}
Bool pragma::CGame::RawCharInput(unsigned int c)
{
	if(m_inputCallbackHandler.CallLuaEvents<int>("OnCharInput", c) == util::EventReply::Handled)
		return false;
	return true;
}
Bool pragma::CGame::RawScrollInput(Vector2 offset)
{
	if(m_inputCallbackHandler.CallLuaEvents<float, float>("OnScrollInput", CFloat(offset.x), CFloat(offset.y)) == util::EventReply::Handled)
		return false;
	return true;
}

Bool pragma::CGame::MouseInput(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	auto r = false;
	int bt = static_cast<int>(button) - static_cast<int>(platform::Key::Last);
	if(CallLuaCallbacks<bool, int, int, int>("OnMouseInput", &r, bt, static_cast<int>(state), static_cast<int>(mods)) == CallbackReturnType::HasReturnValue)
		return r;
	return true;
}
Bool pragma::CGame::KeyboardInput(platform::Key key, int, platform::KeyState state, platform::Modifier mods, float magnitude)
{
	auto r = false;
	if(CallLuaCallbacks<bool, int, int, int>("OnKeyboardInput", &r, static_cast<int>(key), static_cast<int>(state), static_cast<int>(mods)) == CallbackReturnType::HasReturnValue)
		return r;
	return true;
}
Bool pragma::CGame::CharInput(unsigned int c)
{
	auto r = false;
	if(CallLuaCallbacks<bool, int>("OnCharInput", &r, c) == CallbackReturnType::HasReturnValue)
		return r;
	return true;
}
Bool pragma::CGame::ScrollInput(Vector2 offset)
{
	auto r = false;
	if(CallLuaCallbacks<bool, float, float>("OnScrollInput", &r, CFloat(offset.x), CFloat(offset.y)) == CallbackReturnType::HasReturnValue)
		return r;
	return true;
}

bool pragma::CGame::OnWindowShouldClose(prosper::Window &window)
{
	bool ret = true;
	CallLuaCallbacks<bool, prosper::Window *>("OnWindowShouldClose", &ret, &window);
	return ret;
}
void pragma::CGame::OnPreedit(prosper::Window &window, const string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret)
{
	CallLuaCallbacks<void, prosper::Window *, std::string, std::vector<int>, int, int>("OnPreedit", &window, preeditString.cpp_str(), blockSizes, focusedBlock, caret);
}
void pragma::CGame::OnIMEStatusChanged(prosper::Window &window, bool imeEnabled) { CallLuaCallbacks<void, prosper::Window *, bool>("OnIMEStatusChanged", &window, imeEnabled); }
void pragma::CGame::OnDragEnter(prosper::Window &window) { CallLuaCallbacks<bool, prosper::Window *>("OnWindowDragEnter", &window); }
void pragma::CGame::OnDragExit(prosper::Window &window) { CallLuaCallbacks<bool, prosper::Window *>("OnWindowDragExit", &window); }
void pragma::CGame::OnFilesDropped(std::vector<std::string> &files)
{
	auto *l = GetLuaState();
	auto t = Lua::CreateTable(l);
	auto &droppedFiles = get_cengine()->GetDroppedFiles();
	for(auto i = decltype(droppedFiles.size()) {0}; i < droppedFiles.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushString(l, droppedFiles[i].fileName);
		Lua::SetTableValue(l, t);
	}
	auto o = luabind::object(luabind::from_stack(l, -1));
	CallLuaCallbacks<void, luabind::object>("OnFilesDropped", o);
}
