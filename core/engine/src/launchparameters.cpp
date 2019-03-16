#include "stdafx_engine.h"
#include "pragma/launchparameters.h"
#include "pragma/engine.h"

Engine::LaunchCommand::LaunchCommand(const std::string &cmd,const std::vector<std::string> &_args)
	: command(cmd),args(_args)
{}

std::stringstream LaunchParaMap::LAUNCHPARAMETERS_HELP;

void LaunchParaMap::GetParameters(std::unordered_map<std::string,void(*)(int,char*[])> **parameters) {*parameters = &m_parameters;}

void LaunchParaMap::RegisterParameter(std::string name,void(*function)(int,char*[]))
{
	m_parameters.insert(std::unordered_map<std::string,void(*)(int,char*[])>::value_type(name,function));
}

DLLENGINE LaunchParaMap *g_LaunchParameters = NULL;

DLLENGINE LaunchParaMap *GetLaunchParaMap() {return g_LaunchParameters;}

DLLENGINE void RegisterLaunchParameter(std::string name,void(*function)(int,char*[]))
{
	if(g_LaunchParameters == NULL)
	{
		static LaunchParaMap map;
		g_LaunchParameters = &map;
	}
	g_LaunchParameters->RegisterParameter(name,function);
}

DLLENGINE void RegisterLaunchParameterHelp(std::string name,void(*function)(int,char*[]),std::string descCmd,std::string descHelp)
{
	RegisterLaunchParameter(name,function);
	std::stringstream pre;
	pre<<"\t"<<name<<" "<<descCmd;
	std::string sp = "";
	int l = 22 -static_cast<int>(pre.str().length());
	if(l < 3)
	{
		pre<<"\n\t";
		l = 21;
	}
	for(int j=0;j<l;j++)
		sp += " ";
	g_LaunchParameters->LAUNCHPARAMETERS_HELP<<pre.str()<<sp<<descHelp<<"\n";
}

void Engine::InitLaunchOptions(int argc,char *argv[])
{
	std::unordered_map<std::string,void(*)(int,char*[])> *parameters;
	g_LaunchParameters->GetParameters(&parameters);
	int pargcprev = argc -1;
	for(int i=argc-1;i>=0;i--)
	{
		std::string argraw(argv[i]);
		std::string arg(argraw);
		std::transform(arg.begin(),arg.end(),arg.begin(),::tolower);
		if(arg.front() == '+')
		{
			std::vector<std::string> cmdArgs;
			cmdArgs.reserve(pargcprev -i);
			for(auto k=i+1;k<=pargcprev;k++)
				cmdArgs.push_back(argv[k]);
			m_launchCommands.push_back({arg.substr(1),cmdArgs});
		}
		else
		{
			auto j = parameters->find(arg);
			if(j != parameters->end())
			{
				char *pargv[16];
				int pargc = pargcprev -i;
				int narg = 0;
				for(int k=i+1;k<=pargcprev;k++)
				{
					pargv[narg] = argv[k];
					narg++;
				}
				j->second(pargc,pargv);
				pargcprev = i-1;
			}
		}
	}
}

//////////////////////////////

extern Engine *engine;
DLLENGINE void LPARAM_console(int,char*[]) {engine->OpenConsole();}

DLLENGINE void LPARAM_log(int argc,char *argv[])
{
	std::string log;
	if(argc == 2)
		log = argv[1];
	else log = "3";
	if(argc > 0)
		engine->AddLaunchConVar("log_file",argv[0]);
	engine->AddLaunchConVar("log_enabled",log);
}

std::string __lp_map = "";
std::string __lp_gamemode = "";
DLLENGINE void LPARAM_map(int argc,char *argv[])
{
	if(argc == 0) return;
	__lp_map = argv[0];
}

DLLENGINE void LPARAM_gamemode(int argc, char *argv[])
{
	if(argc == 0) return;
	__lp_gamemode = argv[0];
}

DLLENGINE void LPARAM_tcpport(int argc,char *argv[])
{
	if(argc == 0) return;
	engine->AddLaunchConVar("net_port_tcp",argv[0]);//WEAVETODO
}

DLLENGINE void LPARAM_udpport(int argc,char *argv[])
{
	if(argc == 0) return;
	engine->AddLaunchConVar("net_port_udp",argv[0]); //WEAVETODO
}

DLLENGINE void LPARAM_tcponly(int,char*[]) {}//m_bTCPOnly = true;}//WEAVETODO

DLLENGINE void LPARAM_connect(int argc,char *argv[])
{
	if(argc == 0) return;
	std::string arg = argv[0];
	std::string ip,port;
	size_t i = arg.find(':');
	if(i == size_t(-1))
	{
		ip = arg;
		port = "29150";
	}
	else
	{
		ip = arg.substr(0,i);
		port = arg.substr(i +1,arg.length());
	}
	//Connect(ip,port);//WEAVETODO
}

DLLENGINE void LPARAM_help(int,char*[])
{
	engine->OpenConsole();
	LaunchParaMap *map = GetLaunchParaMap();
	if(map == NULL)
		return;
	Con::cout<<"Available parameters:"<<Con::endl<<map->LAUNCHPARAMETERS_HELP.str()<<Con::endl;
}

static void LPARAM_luaext(int argc,char *argv[])
{
	Lua::set_extended_lua_modules_enabled(true);
	// Lua::set_precompiled_files_enabled(false);
}

static void LPARAM_verbose(int argc,char *argv[])
{
	engine->SetVerbose(true);
}

REGISTER_LAUNCH_PARAMETER_HELP(-console,LPARAM_console,"","start with the console open");
REGISTER_LAUNCH_PARAMETER_HELP(-log,LPARAM_log,"[<logfile>][<errorlevel>]","enable logging. Default log file is log.txt");
REGISTER_LAUNCH_PARAMETER_HELP(-map,LPARAM_map,"<map>","load this map on start");
REGISTER_LAUNCH_PARAMETER_HELP(-gamemode,LPARAM_gamemode,"<gamemode>","load this gamemode on start");
REGISTER_LAUNCH_PARAMETER_HELP(-luaext,LPARAM_luaext,"","enables several additional lua modules (e.g. package and io)");
REGISTER_LAUNCH_PARAMETER_HELP(-verbose,LPARAM_verbose,"","Enables additional debug messages.");

REGISTER_LAUNCH_PARAMETER_HELP(-tcpport,LPARAM_tcpport,"<port>","set TCP port");
REGISTER_LAUNCH_PARAMETER_HELP(-udpport,LPARAM_udpport,"<port>","set UDP port");
REGISTER_LAUNCH_PARAMETER_HELP(-tcponly,LPARAM_tcponly,"","use TCP for all communication");
REGISTER_LAUNCH_PARAMETER_HELP(-connect,LPARAM_connect,"<ip>[:<port>]","connect to server immediately. Default port is 29150");

REGISTER_LAUNCH_PARAMETER_HELP(-help,LPARAM_help,"-? /?","show this help message");
REGISTER_LAUNCH_PARAMETER(/?,LPARAM_help);
REGISTER_LAUNCH_PARAMETER(-?,LPARAM_help);