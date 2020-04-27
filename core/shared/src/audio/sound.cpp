/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/scripts.h"
#include <fsys/filesystem.h>
#include "pragma/serverdefinitions.h"
class ServerState;
extern ServerState *server;
Bool NetworkState::IsSoundPrecached(const std::string &snd) const
{
	std::string lsnd = snd;
	std::transform(lsnd.begin(),lsnd.end(),lsnd.begin(),::tolower);
	lsnd = FileManager::GetCanonicalizedPath(lsnd);
	auto i = m_soundsPrecached.find(lsnd);
	return (i != m_soundsPrecached.end()) ? true : false;
}
const std::vector<ALSoundRef> &NetworkState::GetSounds() const {return m_sounds;}
std::vector<ALSoundRef> &NetworkState::GetSounds() {return m_sounds;}
