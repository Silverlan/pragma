// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :engine;
import :client_state;

pragma::ClientState *CEngine::OpenClientState()
{
	CloseClientState();
	m_clInstance->state = std::make_unique<pragma::ClientState>();
	auto *cl = GetClientState();
	cl->Initialize();
	UpdateAssetMultiThreadedLoadingEnabled();
	return static_cast<pragma::ClientState *>(cl);
}

void CEngine::CloseClientState()
{
	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	cl->Close();

	m_inputBindingLayers.clear();

	m_clInstance->state = nullptr;
}

pragma::NetworkState *CEngine::GetClientState() const
{
	if(m_clInstance == nullptr)
		return nullptr;
	return static_cast<pragma::ClientState *>(m_clInstance->state.get());
}
