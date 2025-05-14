/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_networked_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/property/util_property_euler_angles.hpp>
#include <pragma/entities/entity_property.hpp>

using namespace pragma;

void SNetworkedComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
#if NETWORKED_VARS_ENABLED != 0
	auto numNwVars = m_networkedVarNames.size();
	packet->Write<uint32_t>(static_cast<uint32_t>(numNwVars));
	for(auto i = decltype(numNwVars) {0}; i < numNwVars; ++i)
		packet->WriteString(m_networkedVarNames[i]);
#endif
}

void SNetworkedComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

#if NETWORKED_VARS_ENABLED != 0
template<typename T>
SNetworkedComponent::NetworkedVariable::Type SNetworkedComponent::get_networked_variable_type()
{
	if constexpr(std::is_same_v<T, bool>)
		return NetworkedVariable::Type::Bool;
	else if constexpr(std::is_same_v<T, double>)
		return NetworkedVariable::Type::Double;
	else if constexpr(std::is_same_v<T, float>)
		return NetworkedVariable::Type::Float;
	else if constexpr(std::is_same_v<T, int8_t>)
		return NetworkedVariable::Type::Int8;
	else if constexpr(std::is_same_v<T, int16_t>)
		return NetworkedVariable::Type::Int16;
	else if constexpr(std::is_same_v<T, int32_t>)
		return NetworkedVariable::Type::Int32;
	else if constexpr(std::is_same_v<T, int64_t>)
		return NetworkedVariable::Type::Int64;
	else if constexpr(std::is_same_v<T, long double>)
		return NetworkedVariable::Type::LongDouble;
	else if constexpr(std::is_same_v<T, uint8_t>)
		return NetworkedVariable::Type::UInt8;
	else if constexpr(std::is_same_v<T, uint16_t>)
		return NetworkedVariable::Type::UInt16;
	else if constexpr(std::is_same_v<T, uint32_t>)
		return NetworkedVariable::Type::UInt32;
	else if constexpr(std::is_same_v<T, uint64_t>)
		return NetworkedVariable::Type::UInt64;
	else if constexpr(std::is_same_v<T, EulerAngles>)
		return NetworkedVariable::Type::EulerAngles;
	else if constexpr(std::is_same_v<T, Color>)
		return NetworkedVariable::Type::Color;
	else if constexpr(std::is_same_v<T, Vector3>)
		return NetworkedVariable::Type::Vector;
	else if constexpr(std::is_same_v<T, Vector2>)
		return NetworkedVariable::Type::Vector2;
	else if constexpr(std::is_same_v<T, Vector4>)
		return NetworkedVariable::Type::Vector4;
	else if constexpr(std::is_same_v<T, Quat>)
		return NetworkedVariable::Type::Quaternion;
	else
		static_assert(false, "Invalid networked variable type");
}

template<class TProperty>
void SNetworkedComponent::add_networked_variable_callback(NetworkedVariable::Id id, util::BaseProperty &prop)
{
	auto hThis = GetHandle();
	static_cast<TProperty &>(prop).AddCallback([this, hThis, id](std::reference_wrapper<const decltype(TProperty::m_value)> oldVal, std::reference_wrapper<const decltype(TProperty::m_value)> val) {
		if(hThis.expired())
			return;
		NetPacket packet {};
		packet->Write<NetworkedVariable::Id>(id);
		packet->Write<NetworkedVariable::Type>(get_networked_variable_type<decltype(TProperty::m_value)>());
		packet->Write<decltype(TProperty::m_value)>(val);
		static_cast<SBaseEntity &>(GetEntity()).SendNetEventTCP(m_netEvSetVar, packet);
	});
}

void SNetworkedComponent::OnNetworkedVariableCreated(NetworkedVariable &nwVar)
{
	BaseNetworkedComponent::OnNetworkedVariableCreated(nwVar);
	auto id = nwVar.id;
	switch(nwVar.type) {
	case NetworkedVariable::Type::Bool:
		{
			add_networked_variable_callback<util::BoolProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Double:
		{
			add_networked_variable_callback<util::DoubleProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Float:
		{
			add_networked_variable_callback<util::FloatProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Int8:
		{
			add_networked_variable_callback<util::Int8Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Int16:
		{
			add_networked_variable_callback<util::Int16Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Int32:
		{
			add_networked_variable_callback<util::Int32Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Int64:
		{
			add_networked_variable_callback<util::Int64Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::LongDouble:
		{
			add_networked_variable_callback<util::LongDoubleProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::String:
		{
			auto hThis = GetHandle();
			static_cast<util::StringProperty &>(*nwVar.data.get()).AddCallback([this, hThis, id](std::reference_wrapper<const std::string> oldVal, std::reference_wrapper<const std::string> val) {
				if(hThis.expired())
					return;
				NetPacket packet {};
				packet->Write<NetworkedVariable::Id>(id);
				packet->Write<NetworkedVariable::Type>(NetworkedVariable::Type::String);
				packet->WriteString(val);
				static_cast<SBaseEntity &>(GetEntity()).SendNetEventTCP(m_netEvSetVar, packet);
			});
			break;
		}
	case NetworkedVariable::Type::UInt8:
		{
			add_networked_variable_callback<util::UInt8Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::UInt16:
		{
			add_networked_variable_callback<util::UInt16Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::UInt32:
		{
			add_networked_variable_callback<util::UInt32Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::UInt64:
		{
			add_networked_variable_callback<util::UInt64Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::EulerAngles:
		{
			add_networked_variable_callback<util::EulerAnglesProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Color:
		{
			add_networked_variable_callback<util::ColorProperty>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Vector:
		{
			add_networked_variable_callback<util::Vector3Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Vector2:
		{
			add_networked_variable_callback<util::Vector2Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Vector4:
		{
			add_networked_variable_callback<util::Vector4Property>(id, *nwVar.data.get());
			break;
		}
	case NetworkedVariable::Type::Entity:
		{
			auto hThis = GetHandle();
			static_cast<pragma::EntityProperty &>(*nwVar.data.get()).AddCallback([this, hThis, id](std::reference_wrapper<const EntityHandle> oldVal, std::reference_wrapper<const EntityHandle> val) {
				if(hThis.expired())
					return;
				NetPacket packet {};
				packet->Write<NetworkedVariable::Id>(id);
				packet->Write<NetworkedVariable::Type>(NetworkedVariable::Type::Entity);
				nwm::write_entity(packet, val);
				static_cast<SBaseEntity &>(GetEntity()).SendNetEventTCP(m_netEvSetVar, packet);
			});
			break;
		}
	case NetworkedVariable::Type::Quaternion:
		{
			add_networked_variable_callback<util::QuatProperty>(id, *nwVar.data.get());
			break;
		}
	}
}
#endif

void SNetworkedComponent::SetNetworkFlags(NetworkFlags flags)
{
	BaseNetworkedComponent::SetNetworkFlags(flags);
	if((flags & NetworkFlags::ExcludeFromSnapshots) != NetworkFlags::None)
		static_cast<SBaseEntity &>(GetEntity()).SetSynchronized(false);
}
