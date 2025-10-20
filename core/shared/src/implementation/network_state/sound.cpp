// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


#include <algorithm>

module pragma.shared;

import :network_state;

Bool NetworkState::IsSoundPrecached(const std::string &snd) const
{
	std::string lsnd = snd;
	std::transform(lsnd.begin(), lsnd.end(), lsnd.begin(), ::tolower);
	lsnd = FileManager::GetCanonicalizedPath(lsnd);
	auto i = m_soundsPrecached.find(lsnd);
	return (i != m_soundsPrecached.end()) ? true : false;
}
const std::vector<ALSoundRef> &NetworkState::GetSounds() const { return m_sounds; }
std::vector<ALSoundRef> &NetworkState::GetSounds() { return m_sounds; }
