// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include <pragma/networking/resources.h>

ServerInfo::~ServerInfo() { SetDownloadPath(""); }
const std::string &ServerInfo::GetDownloadPath() const { return m_downloadPath; }
void ServerInfo::SetDownloadPath(const std::string &path)
{
	assert(path.empty() || path.back() == '\\');
	if(!m_downloadPath.empty())
		FileManager::RemoveCustomMountDirectory((m_downloadPath.substr(0, m_downloadPath.length() - 1)).c_str());
	if(!path.empty())
		FileManager::AddCustomMountDirectory((path.substr(0, path.length() - 1)).c_str(), static_cast<fsys::SearchFlags>(FSYS_SEARCH_RESOURCES));
	m_downloadPath = path;
}
