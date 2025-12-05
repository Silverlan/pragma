// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :network_state;

Bool NetworkState::IsSoundPrecached(const std::string &snd) const
{
	std::string lsnd = snd;
	ustring::to_lower(lsnd);
	lsnd = FileManager::GetCanonicalizedPath(lsnd);
	auto i = m_soundsPrecached.find(lsnd);
	return (i != m_soundsPrecached.end()) ? true : false;
}
const std::vector<pragma::audio::ALSoundRef> &NetworkState::GetSounds() const { return m_sounds; }
std::vector<pragma::audio::ALSoundRef> &NetworkState::GetSounds() { return m_sounds; }
