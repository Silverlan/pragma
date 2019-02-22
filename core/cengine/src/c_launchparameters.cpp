#include "stdafx_cengine.h"
#include "pragma/launchparameters.h"
#include "pragma/c_engine.h"

extern DLLCENGINE CEngine *c_engine;
static void LPARAM_windowed(int,char*[])
{
	c_engine->SetWindowedMode(true);
}

static void LPARAM_refresh(int argc,char *argv[])
{
	if(argc == 0)
		return;
	int freq = atoi(argv[0]);
	if(freq > 0)
		c_engine->SetRefreshRate(freq);
}

static void LPARAM_noborder(int,char*[])
{
	c_engine->SetNoBorder(true);
}

static void LPARAM_w(int argc,char *argv[])
{
	if(argc == 0)
		return;
	auto sz = c_engine->GetWindowSize();
	c_engine->SetResolution(Vector2i(atoi(argv[0]),sz.at(1)));
}

static void LPARAM_h(int argc,char *argv[])
{
	if(argc == 0)
		return;
	auto sz = c_engine->GetWindowSize();
	c_engine->SetResolution(Vector2i(sz.at(0),atoi(argv[0])));
}

static void LPARAM_fullbright(int,char*[]) {c_engine->UseFullbrightShader(true);}

static void LPARAM_vk_enable_validation(int argc,char *argv[])
{
	c_engine->SetValidationEnabled(true);
}

REGISTER_LAUNCH_PARAMETER_HELP(-windowed,LPARAM_windowed,"-window -startwindowed -sw","start in windowed mode");
REGISTER_LAUNCH_PARAMETER(-window,LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-startwindowed,LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-sw,LPARAM_windowed);

REGISTER_LAUNCH_PARAMETER_HELP(-refresh,LPARAM_refresh,"-refreshrate -freq","monitor refresh rate in Hz. Only available in fullscreen mode");
REGISTER_LAUNCH_PARAMETER(-refreshrate,LPARAM_refresh);
REGISTER_LAUNCH_PARAMETER(-freq,LPARAM_refresh);

REGISTER_LAUNCH_PARAMETER_HELP(-noborder,LPARAM_noborder,"","When used with the game set to windowed mode, will make the game act as if in fullscreen mode (no window border).");

REGISTER_LAUNCH_PARAMETER_HELP(-w,LPARAM_w,"<width>","set the screen width");
REGISTER_LAUNCH_PARAMETER_HELP(-h,LPARAM_h,"<height>","set the screen height");

REGISTER_LAUNCH_PARAMETER_HELP(-fullbright,LPARAM_fullbright,"","start in fullbright mode");

REGISTER_LAUNCH_PARAMETER_HELP(-vk_enable_validation,LPARAM_vk_enable_validation,"<1/0>","Enables or disables vulkan validation layers.");
