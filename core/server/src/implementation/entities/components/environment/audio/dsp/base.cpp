// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.audio.dsp.base;

using namespace pragma;

void SBaseSoundDspComponent::Initialize()
{
	BaseEnvSoundDspComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(OnSetKeyValue(kvData.key, kvData.value) == false)
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void SBaseSoundDspComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvDsp);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
	packet->Write<float>(GetGain());
}

void SBaseSoundDspComponent::SetGain(float gain)
{
	BaseEnvSoundDspComponent::SetGain(gain);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsSpawned() == false)
		return;
	NetPacket p {};
	p->Write<float>(gain);
	ent.SendNetEvent(m_netEvSetGain, p, networking::Protocol::SlowReliable);
}

void SSoundDspComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDsp::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspComponent>();
}
