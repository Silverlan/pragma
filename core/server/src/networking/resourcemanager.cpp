#include "stdafx_server.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/networking/resources.h"
#include <fsys/filesystem.h>
#include <mathutil/umath.h>
#include "pragma/entities/player.h"
#include <sharedutils/util_file.h>

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

ResourceManager::ResourceInfo::ResourceInfo(const std::string &_fileName,bool _stream)
	: fileName(_fileName),stream(_stream)
{}

decltype(ResourceManager::m_resources) ResourceManager::m_resources;

const std::vector<ResourceManager::ResourceInfo> &ResourceManager::GetResources() {return m_resources;}

const ResourceManager::ResourceInfo *ResourceManager::FindResource(const std::string &fileName)
{
	auto tgt = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find_if(m_resources.begin(),m_resources.end(),[&tgt](const ResourceInfo &info) {
		return (info.fileName == tgt) ? true : false;
	});
	if(it == m_resources.end())
	{
		static const auto sndPath = std::string("sounds") +FileManager::GetDirectorySeparator();
		if(ustring::compare(fileName.c_str(),sndPath.c_str(),false,sndPath.length()) == true)
		{
			std::string ext;
			if(ufile::get_extension(fileName,&ext) == false)
			{
				for(auto &ext : engine_info::get_supported_audio_formats())
				{
					auto extPath = fileName +'.' +ext;
					auto *r = FindResource(extPath);
					if(r != nullptr)
						return r;
				}
			}
		}
		return nullptr;
	}
	return &(*it);
}

bool ResourceManager::AddResource(std::string res,bool stream)
{
	res = FileManager::GetCanonicalizedPath(res);
	auto checkName = res;
	std::string ext;
	if(ufile::get_extension(res,&ext) == true && ext == "lua")
	{
		res = res.substr(0,res.length() -3) +"clua";
		ext = "clua";
	}
	if(ext == "clua")
		checkName = res.substr(0,res.length() -4) +"lua";
	if(!IsValidResource(res))
	{
		Con::cwar<<"WARNING: Attempted to add invalid resource '"<<res<<"'! Skipping..."<<Con::endl;
		return false;
	}
	if(!FileManager::Exists(checkName))
	{
		Con::cwar<<"WARNING: Unable to add resource file '"<<res<<"': File not found! Skipping..."<<Con::endl;
		return false;
	}
	auto it = std::find_if(m_resources.begin(),m_resources.end(),[&res](const ResourceInfo &info) {
		return (info.fileName == res) ? true : false;
	});
	if(it != m_resources.end())
	{
		if(stream == true)
			return true;
		it->stream = stream;
		return true;
	}
	m_resources.push_back({res,stream});

	// Send resource to all connected clients
	server->SendResourceFile(res);
	//
	return true;
}

unsigned int ResourceManager::GetResourceCount() {return CUInt32(m_resources.size());}

bool ResourceManager::IsValidResource(std::string res) {return ::IsValidResource(res);}

void ResourceManager::ClearResources() {m_resources.clear();}