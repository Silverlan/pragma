#ifndef __LAUNCHPARAMETERS_H__
#define __LAUNCHPARAMETERS_H__
#include "pragma/definitions.h"
#include <sstream>
#include <string>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLENGINE LaunchParaMap
{
public:
private:
	std::unordered_map<std::string,void(*)(int,char*[])> m_parameters;
public:
	static std::stringstream LAUNCHPARAMETERS_HELP;
	void GetParameters(std::unordered_map<std::string,void(*)(int,char*[])> **parameters);
	void RegisterParameter(std::string name,void(*function)(int,char*[]));
};
#pragma warning(pop)
DLLENGINE LaunchParaMap *GetLaunchParaMap();
DLLENGINE void RegisterLaunchParameter(std::string name,void(*function)(int,char*[]));
DLLENGINE void RegisterLaunchParameterHelp(std::string name,void(*function)(int,char*[]),std::string descCmd,std::string descHelp);

#define REGISTER_LAUNCH_PARAMETER__(lpName,function,identifier) \
	class LPM##function##identifier \
	{ \
	public: \
		LPM##function##identifier() \
		{ \
			RegisterLaunchParameter(#lpName,&function); \
		} \
	}; \
	static LPM##function##identifier g_LPM##function##identifier;

#define REGISTER_LAUNCH_PARAMETER_(lpName,function,identifier) \
	REGISTER_LAUNCH_PARAMETER__(lpName,function,identifier)

#define REGISTER_LAUNCH_PARAMETER(lpName,function) \
	REGISTER_LAUNCH_PARAMETER_(lpName,function,__COUNTER__)

#define REGISTER_LAUNCH_PARAMETER_HELP__(lpName,function,identifier,descCmd,descHelp) \
	class LPM##function##identifier \
	{ \
	public: \
		LPM##function##identifier() \
		{ \
			RegisterLaunchParameterHelp(#lpName,&function,descCmd,descHelp); \
		} \
	}; \
	static LPM##function##identifier g_LPM##function##identifier;

#define REGISTER_LAUNCH_PARAMETER_HELP_(lpName,function,identifier,descCmd,descHelp) \
	REGISTER_LAUNCH_PARAMETER_HELP__(lpName,function,identifier,descCmd,descHelp)

#define REGISTER_LAUNCH_PARAMETER_HELP(lpName,function,descCmd,descHelp) \
	REGISTER_LAUNCH_PARAMETER_HELP_(lpName,function,__COUNTER__,descCmd,descHelp)

#endif