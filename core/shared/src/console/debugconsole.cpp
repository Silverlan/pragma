/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/console/debugconsole.h"
#include "pragma/engine.h"
#include <pragma/serverstate/serverstate.h>
#include <sharedutils/util_string.h>
#include <pragma/console/convars.h>
#include <atomic>
#ifdef __linux__

#include <pthread.h>

#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#endif

#ifdef __linux__
//https://stackoverflow.com/a/76104592
// Returns 1 on success, 0 when not done, and -1 on failure (check errno)
// str is initially expected to be an empty string and should only altered by this function.
static int getline_async_thread_safe(std::string &str, const int &fd = 0, char delim = '\n')
{
	int chars_read;
	do {
		char buf[2] = {0};
		pollfd fd_stdin {0, POLLIN, 0};

		//sigemptyset(&signalset);
		ppoll(&fd_stdin, 1, NULL, nullptr);
		chars_read = (int)read(fd, buf, 1);
		if(chars_read == 1) {
			if(*buf == delim) {
				return 1;
			}
			str.append(buf);
		}
		else {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				chars_read = 0;
				break;
			}
		}
	} while(chars_read > 0);

	return chars_read;
}
#endif

extern Engine *engine;
static std::atomic_bool bCheckInput = true;
static void KeyboardInput()
{
	//TODO: Rewrite this to use non-blocking algorythms
	std::string line;
#ifdef _WIN32
	while(bCheckInput) {
		std::getline(std::cin, line);
		if(bCheckInput)
			engine->ConsoleInput(line);
	}
#else
	int retval;
	while(bCheckInput) {
		retval = getline_async_thread_safe(line);
		if(retval > 0) {
			// Process std::string output
			// Make sure to reset string if continuing through loop
			if(bCheckInput)
				engine->ConsoleInput(line);

			line = "";
		}
		// line = "";
	}
#endif
}

void Engine::ConsoleInput(const std::string_view &line) // TODO: Make sure input-thread and engine don't access m_consoleInput at the same time?
{
	m_consoleInput.push(std::string {line});
}

void Engine::ToggleConsole()
{
	if(IsConsoleOpen()) {
		CloseConsole();
		return;
	}
	OpenConsole();
}

Engine::ConsoleInstance::ConsoleInstance()
{
	console = std::make_unique<DebugConsole>();
	console->open();
	consoleThread = std::make_unique<std::thread>(std::bind(&KeyboardInput));
	util::set_thread_name(*consoleThread, "pr_console_input_listener");
}

#ifdef _WIN32
bool is_console_subsystem();
#endif
Engine::ConsoleInstance::~ConsoleInstance()
{
#ifdef __linux__
	//In linux we most likely run from console. Relinquish our control of cin.
	bCheckInput = false;
#endif
	console->close();
#ifdef _WIN32
	if(is_console_subsystem() && consoleThread) {
		// There's no way to cancel the blocking std::getline in the console thread if it is attached
		// to a parent console, so we have to force terminate the thread.
		// TODO: Do this properly by implementing an asynchronous non-blocking input method.
		TerminateThread(consoleThread->native_handle(), 0);
	}
#else
	//It is impossible to unblock KeyboardInput by putting \n. I have to cancel the thread.
	auto natConsoleThread = consoleThread->native_handle();
	pthread_cancel(natConsoleThread);
#endif
	consoleThread->join();
}

void Engine::OpenConsole()
{
	if(m_consoleInfo == nullptr) {
		m_consoleInfo = std::unique_ptr<ConsoleInstance> {new ConsoleInstance {}};
		bCheckInput = true;
	}
}

void Engine::CloseConsole()
{
	if(!m_consoleInfo)
		return;
	bCheckInput = false;
	m_consoleInfo = nullptr;
}

bool Engine::IsConsoleOpen() const { return m_consoleInfo != NULL; }
DebugConsole *Engine::GetConsole() { return m_consoleInfo ? m_consoleInfo->console.get() : nullptr; }

void Engine::ProcessConsoleInput(KeyState pressState)
{
	while(m_consoleInput.empty() == false) {
		auto &l = m_consoleInput.front();

		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "> " << l << Con::endl;
		util::reset_console_color();
		ProcessConsoleInput(l, pressState);

		m_consoleInput.pop();
	}
}

void Engine::ProcessConsoleInput(const std::string_view &line, KeyState pressState, float magnitude)
{
	ustring::get_sequence_commands(std::string {line}, [pressState, magnitude](std::string cmd, std::vector<std::string> &argv) { engine->RunConsoleCommand(cmd, argv, pressState, magnitude); });
}

bool Engine::RunEngineConsoleCommand(std::string scmd, std::vector<std::string> &argv, KeyState pressState, float magnitude, const std::function<bool(ConConf *, float &)> &callback)
{
	auto *cv = engine->CVarHandler::GetConVar(scmd);
	if(cv == nullptr)
		return false;
	if(callback != nullptr && callback(cv, magnitude) == false)
		return true;

	auto type = cv->GetType();
	if(type == ConType::Var) {
		auto *cvar = static_cast<ConVar *>(cv);
		if(argv.empty()) {
			cvar->Print(scmd);
			return true;
		}
		auto flags = cvar->GetFlags();
		//if((flags &ConVarFlags::Cheat) == ConVarFlags::Cheat)
		//	CHECK_CHEATS(scmd,this,true);
		SetConVar(scmd, argv[0]);
		return true;
	}
	auto *cmd = static_cast<ConCommand *>(cv);
	if(type == ConType::Cmd) {
		std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> func = nullptr;
		cmd->GetFunction(func);
		if(scmd.empty() == false && scmd.front() == '-')
			magnitude = 0.f;
		func(nullptr, nullptr, argv, magnitude);
		return true;
	}
	return false;
}

bool Engine::RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState, float magnitude, const std::function<bool(ConConf *, float &)> &callback)
{
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
	ServerState *stateSv = GetServerState();
	if(stateSv == nullptr)
		return RunEngineConsoleCommand(cmd, argv, pressState, magnitude, callback);
	if(stateSv == NULL || !stateSv->RunConsoleCommand(cmd, argv, nullptr, pressState, magnitude, callback)) {
		Con::cwar << "Unknown console command '" << cmd << "'!" << Con::endl;
		auto similar = (stateSv != nullptr) ? stateSv->FindSimilarConVars(cmd) : FindSimilarConVars(cmd);
		if(similar.empty() == true)
			Con::cout << "No similar matches found!" << Con::endl;
		else {
			Con::cout << "Were you looking for one of the following?" << Con::endl;
			for(auto &sim : similar)
				Con::cout << "- " << sim << Con::endl;
		}
		return false;
	}
	return true;
}
