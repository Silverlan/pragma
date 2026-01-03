// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :client_state;

pragma::ClientState *pragma::CEngine::OpenClientState()
{
	CloseClientState();
	m_clInstance->state = std::make_unique<ClientState>();
	auto *cl = GetClientState();
	cl->Initialize();
	UpdateAssetMultiThreadedLoadingEnabled();
	return static_cast<ClientState *>(cl);
}

void pragma::CEngine::CloseClientState()
{
	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	cl->Close();

	m_inputBindingLayers.clear();

	m_clInstance->state = nullptr;
}

pragma::NetworkState *pragma::CEngine::GetClientState() const
{
	if(m_clInstance == nullptr)
		return nullptr;
	return static_cast<ClientState *>(m_clInstance->state.get());
}
