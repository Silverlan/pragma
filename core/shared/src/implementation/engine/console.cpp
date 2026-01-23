// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef __linux__

#include <pthread.h>

#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <cerrno>
#elif _WIN32
#include <Windows.h>
#endif

module pragma.shared;

import :engine;

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
		ppoll(&fd_stdin, 1, nullptr, nullptr);
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

extern pragma::Engine *engine;
static std::atomic_bool bCheckInput = true;
static void KeyboardInput()
{
	if(pragma::Engine::Get()->IsNonInteractiveMode())
		return;
	//TODO: Rewrite this to use non-blocking algorythms
	std::string line;
#ifdef _WIN32
	while(bCheckInput) {
		std::getline(std::cin, line);
		if(bCheckInput)
			pragma::Engine::Get()->ConsoleInput(line);
	}
#else
	int retval;
	while(bCheckInput) {
		retval = getline_async_thread_safe(line);
		if(retval > 0) {
			// Process std::string output
			// Make sure to reset string if continuing through loop
			if(bCheckInput)
				pragma::Engine::Get()->ConsoleInput(line);

			line = "";
		}
		// line = "";
	}
#endif
}

void pragma::Engine::ConsoleInput(const std::string_view &line, bool printLine) // TODO: Make sure input-thread and engine don't access m_consoleInput at the same time?
{
	std::unique_lock lock {m_consoleInputMutex};
	m_consoleInput.push({std::string {line}, printLine});
}

void pragma::Engine::ToggleConsole()
{
	if(IsConsoleOpen()) {
		CloseConsole();
		return;
	}
	OpenConsole();
}

pragma::Engine::ConsoleInstance::ConsoleInstance()
{
	console = std::make_unique<console::DebugConsole>();
	console->open();

	auto useConsoleThread = true;
#ifdef __linux__
	auto useLinenoise = true;
	if(Get()->IsNonInteractiveMode() || Get()->IsLinenoiseEnabled() == false)
		useLinenoise = false;
	if(useLinenoise) {
		useConsoleThread = false;
		console::impl::init_linenoise();
	}
#endif
	if(useConsoleThread) {
		consoleThread = std::make_unique<std::thread>(std::bind(&KeyboardInput));
		util::set_thread_name(*consoleThread, "pr_console_input_listener");
	}
}

pragma::Engine::ConsoleInstance::~ConsoleInstance()
{
#ifdef __linux__
	//In linux we most likely run from console. Relinquish our control of cin.
	bCheckInput = false;
#endif
	console->close();
#ifdef _WIN32
	if(pragma::util::get_subsystem() == pragma::util::SubSystem::Console && consoleThread) {
		// There's no way to cancel the blocking std::getline in the console thread if it is attached
		// to a parent console, so we have to force terminate the thread.
		// TODO: Do this properly by implementing an asynchronous non-blocking input method.
		TerminateThread(consoleThread->native_handle(), 0);
	}
#else
	//It is impossible to unblock KeyboardInput by putting \n. I have to cancel the thread.
	if(consoleThread) {
		auto natConsoleThread = consoleThread->native_handle();
		pthread_cancel(natConsoleThread);
	}
#endif
	if(consoleThread)
		consoleThread->join();
}

void pragma::Engine::OpenConsole()
{
	if(m_consoleInfo == nullptr) {
		m_consoleInfo = std::unique_ptr<ConsoleInstance> {new ConsoleInstance {}};
		bCheckInput = true;
	}
}

void pragma::Engine::CloseConsole()
{
	if(!m_consoleInfo)
		return;
	bCheckInput = false;
	m_consoleInfo = nullptr;
}

bool pragma::Engine::IsConsoleOpen() const { return m_consoleInfo != nullptr; }
pragma::console::DebugConsole *pragma::Engine::GetConsole() { return m_consoleInfo ? m_consoleInfo->console.get() : nullptr; }

void pragma::Engine::ProcessConsoleInput(KeyState pressState)
{
#ifdef __linux__
	if(console::impl::is_linenoise_enabled())
		console::impl::update_linenoise();
#endif

	m_consoleInputMutex.lock();
	if(m_consoleInput.empty()) {
		m_consoleInputMutex.unlock();
		return;
	}
	auto consoleInput = std::move(m_consoleInput);
	m_consoleInput = {};
	m_consoleInputMutex.unlock();

	while(consoleInput.empty() == false) {
		auto &inputInfo = consoleInput.front();

		console::set_console_color(console::ConsoleColorFlags::White | console::ConsoleColorFlags::Intensity);
		if(inputInfo.printLine)
			Con::COUT << "> " << inputInfo.line << Con::endl;
		console::reset_console_color();
		ProcessConsoleInput(inputInfo.line, pressState);

		consoleInput.pop();
	}
}

void pragma::Engine::ProcessConsoleInput(const std::string_view &line, KeyState pressState, float magnitude)
{
	string::get_sequence_commands(std::string {line}, [pressState, magnitude](std::string cmd, std::vector<std::string> &argv) { Get()->RunConsoleCommand(cmd, argv, pressState, magnitude); });
}

bool pragma::Engine::RunEngineConsoleCommand(std::string scmd, std::vector<std::string> &argv, KeyState pressState, float magnitude, const std::function<bool(console::ConConf *, float &)> &callback)
{
	auto *cv = Get()->CVarHandler::GetConVar(scmd);
	if(cv == nullptr)
		return false;
	if(callback != nullptr && callback(cv, magnitude) == false)
		return true;

	auto type = cv->GetType();
	if(type == console::ConType::Var) {
		auto *cvar = static_cast<console::ConVar *>(cv);
		if(argv.empty()) {
			cvar->Print(scmd);
			return true;
		}
		auto flags = cvar->GetFlags();
		//if((flags &ConVarFlags::Cheat) == ConVarFlags::Cheat)
		//	if(!check_cheats(scmd,this)) return true;
		SetConVar(scmd, argv[0]);
		return true;
	}
	auto *cmd = static_cast<console::ConCommand *>(cv);
	if(type == console::ConType::Cmd) {
		std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> func = nullptr;
		cmd->GetFunction(func);
		if(scmd.empty() == false && scmd.front() == '-')
			magnitude = 0.f;
		func(nullptr, nullptr, argv, magnitude);
		return true;
	}
	return false;
}

bool pragma::Engine::RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState, float magnitude, const std::function<bool(console::ConConf *, float &)> &callback)
{
	string::to_lower(cmd);
	auto *stateSv = GetServerNetworkState();
	if(stateSv == nullptr)
		return RunEngineConsoleCommand(cmd, argv, pressState, magnitude, callback);
	if(stateSv == nullptr || !stateSv->RunConsoleCommand(cmd, argv, nullptr, pressState, magnitude, callback)) {
		Con::CWAR << "Unknown console command '" << cmd << "'!" << Con::endl;
		auto similar = (stateSv != nullptr) ? stateSv->FindSimilarConVars(cmd) : FindSimilarConVars(cmd);
		if(similar.empty() == true)
			Con::COUT << "No similar matches found!" << Con::endl;
		else {
			Con::COUT << "Were you looking for one of the following?" << Con::endl;
			for(auto &sim : similar)
				Con::COUT << "- " << sim << Con::endl;
		}
		return false;
	}
	return true;
}
