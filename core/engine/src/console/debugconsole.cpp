#include "stdafx_engine.h"
#include "pragma/console/debugconsole.h"
#include "pragma/engine.h"
#include <pragma/serverstate/serverstate.h>
#include <sharedutils/util_string.h>
#include <pragma/console/convars.h>

#pragma optimize("",off)
extern Engine *engine;
static bool bCheckInput = true;
static void KeyboardInput()
{
	std::string line;
	while(bCheckInput)
	{
		std::getline(std::cin,line);
		if(bCheckInput)
			engine->ConsoleInput(line);
	}
}

void Engine::ConsoleInput(const std::string &line) // TODO: Make sure input-thread and engine don't access m_consoleInput at the same time?
{
	m_consoleInput.push(line);
}

void Engine::OpenConsole()
{
	if(m_console == NULL)
	{
		m_console = new DebugConsole();
		m_console->open();
		bCheckInput = true;
		m_consoleThread = new std::thread(std::bind(&KeyboardInput));
	}
}

void Engine::CloseConsole()
{
	if(m_console == NULL) return;
	bCheckInput = false;
	m_console->close();
	m_console = NULL;
	m_consoleThread->join();
	delete m_consoleThread;
}

bool Engine::IsConsoleOpen() {return m_console != NULL;}
DebugConsole *Engine::GetConsole() {return m_console;}

void Engine::ProcessConsoleInput(KeyState pressState)
{
	while(m_consoleInput.empty() == false)
	{
		auto &l = m_consoleInput.front();

#ifdef _WIN32
		Con::attr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
		Con::cout<<"\033[1m";
#endif
		Con::cout<<"> "<<l<<Con::endl;
#ifndef _WIN32
		Con::cout<<"\033[0m";
#endif
		ProcessConsoleInput(l,pressState);

		m_consoleInput.pop();
	}
}

void Engine::ProcessConsoleInput(const std::string &line,KeyState pressState,float magnitude)
{
	ustring::get_sequence_commands(line,[pressState,magnitude](std::string cmd,std::vector<std::string> &argv) {
		engine->RunConsoleCommand(cmd,argv,pressState,magnitude);
	});
}

bool Engine::RunEngineConsoleCommand(std::string scmd,std::vector<std::string> &argv,KeyState pressState,float magnitude,const std::function<bool(ConConf*,float&)> &callback)
{
	auto *cv = engine->CVarHandler::GetConVar(scmd);
	if(cv == nullptr)
		return false;
	if(callback != nullptr && callback(cv,magnitude) == false)
		return true;

	auto type = cv->GetType();
	if(type == ConType::Var)
	{
		auto *cvar = static_cast<ConVar*>(cv);
		if(argv.empty())
		{
			cvar->Print(scmd);
			return true;
		}
		auto flags = cvar->GetFlags();
		//if((flags &ConVarFlags::Cheat) == ConVarFlags::Cheat)
		//	CHECK_CHEATS(scmd,this,true);
		SetConVar(scmd,argv[0]);
		return true;
	}
	auto *cmd = static_cast<ConCommand*>(cv);
	if(type == ConType::Cmd)
	{
		std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> func = nullptr;
		cmd->GetFunction(func);
		if(scmd.empty() == false && scmd.front() == '-')
			magnitude = 0.f;
		func(nullptr,nullptr,argv,magnitude);
		return true;
	}
	return false;
}

bool Engine::RunConsoleCommand(std::string cmd,std::vector<std::string> &argv,KeyState pressState,float magnitude,const std::function<bool(ConConf*,float&)> &callback)
{
	std::transform(cmd.begin(),cmd.end(),cmd.begin(),::tolower);
	ServerState *stateSv = GetServerState();
	if(stateSv == nullptr)
		return RunEngineConsoleCommand(cmd,argv,pressState,magnitude,callback);
	if(stateSv == NULL || !stateSv->RunConsoleCommand(cmd,argv,nullptr,pressState,magnitude,callback))
	{
		Con::cwar<<"WARNING: Unknown console command '"<<cmd<<"'!"<<Con::endl;
		auto similar = (stateSv != nullptr) ? stateSv->FindSimilarConVars(cmd) : FindSimilarConVars(cmd);
		if(similar.empty() == true)
			Con::cout<<"No similar matches found!"<<Con::endl;
		else
		{
			Con::cout<<"Were you looking for one of the following?"<<Con::endl;
			for(auto &sim : similar)
				Con::cout<<"- "<<sim<<Con::endl;
		}
		return false;
	}
	return true;
}
#pragma optimize("",on)
