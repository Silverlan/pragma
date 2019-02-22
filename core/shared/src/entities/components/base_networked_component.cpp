#include "stdafx_shared.h"
#include "pragma/entities/components/base_networked_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

BaseNetworkedComponent::BaseNetworkedComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseNetworkedComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"networkflags",false))
			m_networkFlags = static_cast<NetworkFlags>(util::to_int(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

#if NETWORKED_VARS_ENABLED != 0
	// Obsolete
	m_netEvSetVar = SetupNetEvent("set_nwvar");
#endif
}

void BaseNetworkedComponent::SetNetworkFlags(NetworkFlags flags) {m_networkFlags = flags;}
BaseNetworkedComponent::NetworkFlags BaseNetworkedComponent::GetNetworkFlags() const {return m_networkFlags;}
