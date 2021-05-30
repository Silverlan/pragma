/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include <pragma/networking/resources.h>

ServerInfo::~ServerInfo()
{
	SetDownloadPath("");
}
const std::string &ServerInfo::GetDownloadPath() const {return m_downloadPath;}
void ServerInfo::SetDownloadPath(const std::string &path)
{
	assert(path.empty() || path.back() == '\\');
	if(!m_downloadPath.empty())
		FileManager::RemoveCustomMountDirectory((m_downloadPath.substr(0,m_downloadPath.length() -1)).c_str());
	if(!path.empty())
		FileManager::AddCustomMountDirectory((path.substr(0,path.length() -1)).c_str(),static_cast<fsys::SearchFlags>(FSYS_SEARCH_RESOURCES));
	m_downloadPath = path;
}
