#include "stdafx_shared.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/level/level_info.hpp"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/asset_types/world.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include <pragma/math/intersection.h>
#include <sharedutils/util_file.h>
#include <unordered_set>

extern DLLENGINE Engine *engine;
#pragma optimize("",off)
static std::string invert_x_axis(std::string str)
{
	if(str.empty())
		return str;
	if(str[0] != '-')
		str = "-" +str;
	else
		str = str.substr(1,str.length());
	return str;
}

static std::string transform_angles(std::string val)
{
	val = invert_x_axis(val);
	std::vector<std::string> vdat;
	ustring::explode(val,ustring::WHITESPACE.c_str(),vdat);
	if(vdat.size() > 0)
	{
		float f = static_cast<float>(-atof(vdat[0].c_str()));
		vdat[0] = std::to_string(f);
	}
	if(vdat.size() > 1)
	{
		float f = static_cast<float>(atof(vdat[1].c_str()));
		f += 90.f;
		vdat[1] = std::to_string(f);
		val = "";
		for(char i=0;i<vdat.size();i++)
		{
			if(i > 0)
				val += " ";
			val += vdat[i];
		}
	}
	auto ang = EulerAngles(val);
	ang.Normalize();
	return std::to_string(ang.p) +" " +std::to_string(ang.y) +" " +std::to_string(ang.r);
}

static std::string swap_yz_axes(std::string str)
{
	std::vector<std::string> vdat;
	ustring::explode(str,ustring::WHITESPACE.c_str(),vdat);
	size_t l = vdat.size();
	if(l <= 1)
		return str;
	if(l >= 2)
	{
		if(vdat[1][0] != '-')
			vdat[1] = "-" +vdat[1];
		else
			vdat[1] = vdat[1].substr(1,vdat[1].length());
	}
	if(l == 2)
		return vdat[0] +" 0 " +vdat[1];
	return vdat[0] +" " +vdat[2] +" "+vdat[1];
}

static std::string transform_model_path(std::string val)
{
	size_t br = val.find_first_of("/\\");
	if(br != size_t(-1))
	{
		std::string path = val.substr(0,br);
		StringToLower(path);
		if(path == "models")
			val = val.substr(br +1,val.length());
	}
	size_t pext = val.find_last_of('.');
	if(pext != size_t(-1))
		val = val.substr(0,pext) +".wmd";
}

bool util::port_source2_map(NetworkState *nw,const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool(*)(Game&,const std::string&)>(impl::get_module_func(nw,"convert_source2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	impl::init_custom_mount_directories(*nw);
	return ptrConvertMap(*nw->GetGameState(),path);
}

bool util::port_hl2_map(NetworkState *nw,const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool(*)(Game&,const std::string&)>(impl::get_module_func(nw,"convert_hl2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	impl::init_custom_mount_directories(*nw);
	return ptrConvertMap(*nw->GetGameState(),path);
}
#pragma optimize("",on)
