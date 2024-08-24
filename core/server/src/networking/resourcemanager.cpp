/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/networking/resources.h"
#include <fsys/filesystem.h>
#include <mathutil/umath.h>
#include "pragma/entities/player.h"
#include <sharedutils/util_file.h>

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

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
	if(server->IsSinglePlayer())
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
	server->SendResourceFile(res);
	//
	return true;
}

unsigned int ResourceManager::GetResourceCount() { return CUInt32(m_resources.size()); }

bool ResourceManager::IsValidResource(std::string res) { return ::IsValidResource(res); }

void ResourceManager::ClearResources() { m_resources.clear(); }
