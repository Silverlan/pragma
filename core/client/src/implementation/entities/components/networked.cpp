// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.networked;
using namespace pragma;

void CNetworkedComponent::ReceiveData(NetPacket &packet)
{
#if NETWORKED_VARS_ENABLED != 0
	auto numNwVars = packet->Read<uint32_t>();
	m_networkedVarNames.resize(numNwVars);
	for(auto i = decltype(numNwVars) {0}; i < numNwVars; ++i)
		m_networkedVarNames.push_back(packet->ReadString());
#endif
}
void CNetworkedComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CNetworkedComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
#if NETWORKED_VARS_ENABLED != 0
	if(eventId == m_netEvSetVar) {
		auto id = packet->Read<NetworkedVariable::Id>();
		auto type = packet->Read<NetworkedVariable::Type>();
		switch(type) {
		case BaseNetworkedComponent::NetworkedVariable::Type::Bool:
			{
				auto val = packet->Read<bool>();
				SetNetworkedVariable<bool>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Double:
			{
				auto val = packet->Read<double>();
				SetNetworkedVariable<double>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Float:
			{
				auto val = packet->Read<float>();
				SetNetworkedVariable<float>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Int8:
			{
				auto val = packet->Read<int8_t>();
				SetNetworkedVariable<int8_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Int16:
			{
				auto val = packet->Read<int16_t>();
				SetNetworkedVariable<int16_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Int32:
			{
				auto val = packet->Read<int32_t>();
				SetNetworkedVariable<int32_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Int64:
			{
				auto val = packet->Read<int64_t>();
				SetNetworkedVariable<int64_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
			{
				auto val = packet->Read<long double>();
				SetNetworkedVariable<long double>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::String:
			{
				auto val = packet->ReadString();
				SetNetworkedVariable<std::string>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
			{
				auto val = packet->Read<uint8_t>();
				SetNetworkedVariable<uint8_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
			{
				auto val = packet->Read<uint16_t>();
				SetNetworkedVariable<uint16_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
			{
				auto val = packet->Read<uint32_t>();
				SetNetworkedVariable<uint32_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
			{
				auto val = packet->Read<uint64_t>();
				SetNetworkedVariable<uint64_t>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
			{
				auto val = packet->Read<EulerAngles>();
				SetNetworkedVariable<EulerAngles>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Color:
			{
				auto val = packet->Read<Color>();
				SetNetworkedVariable<Color>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Vector:
			{
				auto val = packet->Read<Vector3>();
				SetNetworkedVariable<Vector3>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
			{
				auto val = packet->Read<Vector2>();
				SetNetworkedVariable<Vector2>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
			{
				auto val = packet->Read<Vector4>();
				SetNetworkedVariable<Vector4>(id, val);
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Entity:
			{
				auto *ent = pragma::networking::read_entity(packet);
				SetNetworkedVariable<EntityHandle>(id, (ent != nullptr) ? ent->GetHandle() : EntityHandle {});
				break;
			}
		case BaseNetworkedComponent::NetworkedVariable::Type::Quaternion:
			{
				auto val = packet->Read<Quat>();
				SetNetworkedVariable<Quat>(id, val);
				break;
			}
		}
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
#else
	return false;
#endif
}
