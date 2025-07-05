// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/input/input_binding_layer.hpp>
#include <pragma/clientstate/clientstate.h>

extern DLLCLIENT ClientState *client;

ClientState *CEngine::OpenClientState()
{
	CloseClientState();
	m_clInstance->state = std::make_unique<ClientState>();
	auto *cl = GetClientState();
	cl->Initialize();
	UpdateAssetMultiThreadedLoadingEnabled();
	return static_cast<ClientState *>(cl);
}

void CEngine::CloseClientState()
{
	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	cl->Close();

	m_inputBindingLayers.clear();

	m_clInstance->state = nullptr;
	client = nullptr;
}

NetworkState *CEngine::GetClientState() const
{
	if(m_clInstance == nullptr)
		return nullptr;
	return static_cast<ClientState *>(m_clInstance->state.get());
}
