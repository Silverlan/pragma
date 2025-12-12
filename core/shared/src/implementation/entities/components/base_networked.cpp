// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_networked;

using namespace pragma;

BaseNetworkedComponent::BaseNetworkedComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseNetworkedComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "networkflags", false))
			m_networkFlags = static_cast<NetworkFlags>(pragma::util::to_int(kvData.value));
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

#if NETWORKED_VARS_ENABLED != 0
	// Obsolete
	m_netEvSetVar = SetupNetEvent("set_nwvar");
#endif
}

void BaseNetworkedComponent::SetNetworkFlags(NetworkFlags flags) { m_networkFlags = flags; }
BaseNetworkedComponent::NetworkFlags BaseNetworkedComponent::GetNetworkFlags() const { return m_networkFlags; }
