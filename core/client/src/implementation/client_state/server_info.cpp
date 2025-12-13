// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :client_state;

pragma::ServerInfo::~ServerInfo() { SetDownloadPath(""); }
const std::string &pragma::ServerInfo::GetDownloadPath() const { return m_downloadPath; }
void pragma::ServerInfo::SetDownloadPath(const std::string &path)
{
	assert(path.empty() || path.back() == '\\');
	if(!m_downloadPath.empty())
		fs::remove_custom_mount_directory((m_downloadPath.substr(0, m_downloadPath.length() - 1)));
	if(!path.empty())
		fs::add_custom_mount_directory((path.substr(0, path.length() - 1)), static_cast<fs::SearchFlags>(networking::FSYS_SEARCH_RESOURCES));
	m_downloadPath = path;
}
