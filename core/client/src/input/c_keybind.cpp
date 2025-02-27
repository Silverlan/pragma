/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/c_engine.h"
#include <sharedutils/util_string.h>
#include <pragma/console/convars.h>
#include <pragma/clientstate/clientstate.h>
#include <pragma/console/c_cvar.h>

extern CEngine *c_engine;

KeyBind::KeyBind() : m_type(Type::Invalid) { Initialize(); }

KeyBind::KeyBind(std::string bind)
{
	m_type = Type::Regular;
	m_bind = std::make_unique<std::string>(bind);
	Initialize();
}
KeyBind::KeyBind(luabind::function<> function)
{
	m_type = Type::Function;
	m_function = function;
	Initialize();
}
KeyBind::KeyBind(const KeyBind &other) { operator=(other); }
KeyBind &KeyBind::operator=(const KeyBind &other)
{
	m_bind = other.m_bind ? std::make_unique<std::string>(*other.m_bind) : nullptr;
	;
	m_type = other.m_type;
	m_function = other.m_function;
	m_cmds = other.m_cmds;
	return *this;
}
KeyBind::Type KeyBind::GetType() const { return m_type; }
const std::string &KeyBind::GetBind() const
{
	static std::string r;
	auto type = GetType();
	if(m_bind == nullptr || type != Type::Regular)
		return r;
	return *m_bind;
}
std::optional<luabind::function<>> KeyBind::GetFunction() const { return m_function; }
// Deprecated (Replaced by "toggle" console command)
/*DLLCLIENT void KeyBind_CmdToggle(std::string cmd,std::vector<std::string>&)
{
	auto *cf = c_engine->GetConVar(cmd);
	if(cf == nullptr)
		return;
	if(cf->GetType() != ConType::Var)
		return;
	auto *cvar = static_cast<ConVar*>(cf);
	if(cvar->GetBool() == true)
	{
		std::vector<std::string> args = {"0"};
		c_engine->RunConsoleCommand(cmd,args);
	}
	else
	{
		std::vector<std::string> args = {"1"};
		c_engine->RunConsoleCommand(cmd,args);
	}
}*/
void KeyBind::Initialize()
{
	m_cmds.clear();
	auto type = GetType();
	if(type != Type::Regular)
		return;
	auto &bind = GetBind();
	ustring::get_sequence_commands(bind, [this](std::string cmd, std::vector<std::string> &argv) {
		m_cmds.push_back({});
		auto &info = m_cmds.back();
		info.cmd = cmd;
		info.argv = argv;
	});
}

bool KeyBind::Execute(pragma::platform::KeyState inputState, pragma::platform::KeyState pressState, pragma::platform::Modifier mods, float magnitude)
{
	auto type = GetType();
	switch(type) {
	case Type::Regular: // Regular bind
		{
			auto bAxisInput = (mods & pragma::platform::Modifier::AxisInput) != pragma::platform::Modifier::None;
			auto bNegativeAxis = (bAxisInput == true && (mods & pragma::platform::Modifier::AxisNegative) != pragma::platform::Modifier::None) ? true : false;
			auto bReleased = (pressState == pragma::platform::KeyState::Release) ? true : false;
			auto bExecutedCmd = false;
			for(auto &info : m_cmds) {
				auto bActionCmd = (info.cmd.empty() == false && info.cmd.front() == '+') ? true : false;
				c_engine->RunConsoleCommand(info.cmd, info.argv, static_cast<KeyState>(pressState), magnitude, [&info, &bExecutedCmd, pressState, bReleased, bAxisInput, bNegativeAxis, bActionCmd](ConConf *cf, float &magnitude) -> bool {
					auto cmdReleased = bReleased;
					auto flags = cf->GetFlags();
					auto bSingleAxis = ((flags & ConVarFlags::JoystickAxisSingle) != ConVarFlags::None) ? true : false;
					if(bNegativeAxis == true) {
						if(bAxisInput == false || bSingleAxis == false)
							return false;
						magnitude = c_engine->GetRawJoystickAxisMagnitude();
					}

					if(bSingleAxis == true)
						bExecutedCmd = true; // Single axis commands have priority over everything else
					if(bAxisInput == true) {
						// invalidKeyState is true if the input is neither pressed, nor released
						auto invalidKeyState = (pressState == pragma::platform::KeyState::Invalid) ? true : false;

						if((flags & ConVarFlags::JoystickAxisContinuous) != ConVarFlags::None && (cmdReleased == true || invalidKeyState == true))
							cmdReleased = (c_engine->IsValidAxisInput(magnitude) == false) ? true : false; // Input won't count as 'released' unless joystick axis is at home position (near 0)
						else if(invalidKeyState == true) {
							bExecutedCmd = true; // Special case; Don't execute other commands
							return false;        // Axis is held down, but command doesn't react to below-threshold inputs, and previous state wasn't pressed; Skip the command
						}
					}
					if(bActionCmd == true) {
						if(cmdReleased == true) {
							c_engine->RunConsoleCommand('-' + info.cmd.substr(1), info.argv, static_cast<KeyState>(pressState));
							return false; // Block this command, since we're calling "-" instead
						}
					}
					if(cmdReleased == false) {
						bExecutedCmd = true;
						return true;
					}
					return false;
				});
			}
			return bExecutedCmd;
		}
	case Type::Function: // Lua-function bind
		{
			if(pressState != pragma::platform::KeyState::Press && pressState != pragma::platform::KeyState::Release)
				return false;
			auto *clState = static_cast<ClientState *>(c_engine->GetClientState());
			if(clState == NULL)
				return false;
			auto *game = clState->GetGameState();
			// TODO: Check for errors?
			(*m_function)((pressState == pragma::platform::KeyState::Press) ? true : false);
			return true;
		}
		// Deprecated (Replaced by "toggle" console command)
		/*case Type::Toggle: // BindToggle
		{
			if(pressState != pragma::platform::KeyState::Press && pressState != pragma::platform::KeyState::Release)
				return false;
			std::string bind = GetBind();
			if(pressState == pragma::platform::KeyState::Press)
			{
				for(auto &info : m_cmds)
					KeyBind_CmdToggle(info.cmd,info.argv);
			}
			return true;
		}*/
	}
	return false;
}
