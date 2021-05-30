/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/clientstate/clientstate.h>

extern DLLCLIENT ClientState *client;

ClientState *CEngine::OpenClientState()
{
	CloseClientState();
	m_clInstance->state = std::make_unique<ClientState>();
	auto *cl = GetClientState();
	cl->Initialize();
	return static_cast<ClientState*>(cl);
}

void CEngine::CloseClientState()
{
	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	cl->Close();
	ClearKeyMappings();
	m_clInstance->state = nullptr;
	client = nullptr;
}

NetworkState *CEngine::GetClientState() const
{
	if(m_clInstance == nullptr)
		return nullptr;
	return static_cast<ClientState*>(m_clInstance->state.get());
}
