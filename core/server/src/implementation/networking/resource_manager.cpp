// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <fsys/filesystem.h>
#include <mathutil/umath.h>
#include <sharedutils/util_file.h>

module pragma.server.networking.resource_manager;

import pragma.server.entities;
import pragma.server.game;
import pragma.server.server_state;

#undef FindResource

ResourceManager::ResourceInfo::ResourceInfo(const std::string &_fileName, bool _stream) : fileName(_fileName), stream(_stream) {}

decltype(ResourceManager::m_resources) ResourceManager::m_resources;

const std::vector<ResourceManager::ResourceInfo> &ResourceManager::GetResources() { return m_resources; }

const ResourceManager::ResourceInfo *ResourceManager::FindResource(const std::string &fileName)
{
	auto tgt = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&tgt](const ResourceInfo &info) { return (info.fileName == tgt) ? true : false; });
	if(it == m_resources.end()) {
		static const auto sndPath = std::string("sounds") + FileManager::GetDirectorySeparator();
		if(ustring::compare(fileName.c_str(), sndPath.c_str(), false, sndPath.length()) == true) {
			std::string ext;
			if(ufile::get_extension(fileName, &ext) == false) {
				for(auto &ext : engine_info::get_supported_audio_formats()) {
					auto extPath = fileName + '.' + ext;
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

bool ResourceManager::AddResource(std::string res, bool stream)
{
	if(ServerState::Get()->IsSinglePlayer())
		return false; // We don't need resources in SinglePlayer
	res = FileManager::GetCanonicalizedPath(res);
	auto checkName = res;
	std::string ext;
	if(ufile::get_extension(res, &ext) == true && ext == Lua::FILE_EXTENSION) {
		res = res.substr(0, res.length() - 3) + Lua::FILE_EXTENSION_PRECOMPILED;
		ext = Lua::FILE_EXTENSION_PRECOMPILED;
	}
	if(ext == Lua::FILE_EXTENSION_PRECOMPILED)
		checkName = res.substr(0, res.length() - 4) + Lua::FILE_EXTENSION;
	if(!IsValidResource(res)) {
		Con::cwar << "Attempted to add invalid resource '" << res << "'! Skipping..." << Con::endl;
		return false;
	}
	if(!FileManager::Exists(checkName)) {
		Con::cwar << "Unable to add resource file '" << res << "': File not found! Skipping..." << Con::endl;
		return false;
	}
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&res](const ResourceInfo &info) { return (info.fileName == res) ? true : false; });
	if(it != m_resources.end()) {
		if(stream == true)
			return true;
		it->stream = stream;
		return true;
	}
	m_resources.push_back({res, stream});

	// Send resource to all connected clients
	ServerState::Get()->SendResourceFile(res);
	//
	return true;
}

unsigned int ResourceManager::GetResourceCount() { return CUInt32(m_resources.size()); }

bool ResourceManager::IsValidResource(std::string res) { return ::IsValidResource(res); }

void ResourceManager::ClearResources() { m_resources.clear(); }
