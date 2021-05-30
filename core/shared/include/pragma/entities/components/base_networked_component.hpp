/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_NETWORKED_COMPONENT_HPP__
#define __BASE_NETWORKED_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/util/util_variable_type.hpp"
#include <sharedutils/property/util_property.hpp>

#define NETWORKED_VARS_ENABLED 0

namespace pragma
{
	class DLLNETWORK BaseNetworkedComponent
		: public BaseEntityComponent
	{
	public:
		enum class NetworkFlags : uint32_t
		{
			None = 0,
			ExcludeFromSnapshots = 1
		};
		virtual void Initialize() override;

		virtual void SetNetworkFlags(NetworkFlags flags);
		NetworkFlags GetNetworkFlags() const;
#if NETWORKED_VARS_ENABLED != 0
		struct DLLNETWORK NetworkedVariable
		{
			using Id = size_t;
			NetworkedVariable(Id id,util::VarType type,const std::shared_ptr<util::BaseProperty> &data);
			Id id;
			std::shared_ptr<util::BaseProperty> data;
			util::VarType type;
		};
		const std::shared_ptr<util::BaseProperty> &GetNetworkedVariableProperty(const std::string &name) const;
		const std::shared_ptr<util::BaseProperty> &GetNetworkedVariableProperty(NetworkedVariable::Id id) const;

		util::VarType GetNetworkedVariableType(NetworkedVariable::Id id) const;
		NetworkedVariable::Id CreateNetworkedVariable(const std::string &name,util::VarType type);

		template<typename T>
			bool SetNetworkedVariable(const std::string &name,const T &val);
		template<typename T>
			bool SetNetworkedVariable(NetworkedVariable::Id id,const T &val);

		const std::vector<NetworkedVariable> &GetNetworkedVariables() const;
#endif
	protected:
		BaseNetworkedComponent(BaseEntity &ent);
		NetworkFlags m_networkFlags = NetworkFlags::None;
#if NETWORKED_VARS_ENABLED != 0
		virtual void OnNetworkedVariableCreated(NetworkedVariable &nwVar);
		template<typename T>
			T GetNetworkedVariable(const std::string &name,util::VarType type) const;

		std::vector<NetworkedVariable> m_networkedVars = {};
		std::vector<std::string> m_networkedVarNames = {};
		pragma::NetEventId m_netEvSetVar = pragma::INVALID_NET_EVENT;
#endif
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseNetworkedComponent::NetworkFlags)

#if NETWORKED_VARS_ENABLED != 0
template<typename T>
	bool pragma::BaseNetworkedComponent::SetNetworkedVariable(NetworkedVariable::Id id,const T &val)
{
	if(id >= m_networkedVars.size())
		return false;
	auto &nwVar = m_networkedVars.at(id);
	if constexpr (std::is_same_v<T,bool>)
	{
		if(nwVar.type != NetworkedVariable::Type::Bool)
			return false;
		static_cast<util::BoolProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,double>)
	{
		if(nwVar.type != NetworkedVariable::Type::Double)
			return false;
		static_cast<util::DoubleProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,float>)
	{
		if(nwVar.type != NetworkedVariable::Type::Float)
			return false;
		static_cast<util::FloatProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,int8_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::Int8)
			return false;
		static_cast<util::Int8Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,int16_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::Int16)
			return false;
		static_cast<util::Int16Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,int32_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::Int32)
			return false;
		static_cast<util::Int32Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,int64_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::Int64)
			return false;
		static_cast<util::Int64Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,long double>)
	{
		if(nwVar.type != NetworkedVariable::Type::LongDouble)
			return false;
		static_cast<util::LongDoubleProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,std::string>)
	{
		if(nwVar.type != NetworkedVariable::Type::String)
			return false;
		static_cast<util::StringProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,uint8_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::UInt8)
			return false;
		static_cast<util::UInt8Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,uint16_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::UInt16)
			return false;
		static_cast<util::UInt16Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,uint32_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::UInt32)
			return false;
		static_cast<util::UInt32Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,uint64_t>)
	{
		if(nwVar.type != NetworkedVariable::Type::UInt64)
			return false;
		static_cast<util::UInt64Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,EulerAngles>)
	{
		if(nwVar.type != NetworkedVariable::Type::EulerAngles)
			return false;
		static_cast<typename util::EulerAnglesProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,Color>)
	{
		if(nwVar.type != NetworkedVariable::Type::Color)
			return false;
		static_cast<util::ColorProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,Vector3>)
	{
		if(nwVar.type != NetworkedVariable::Type::Vector)
			return false;
		static_cast<util::Vector3Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,Vector2>)
	{
		if(nwVar.type != NetworkedVariable::Type::Vector2)
			return false;
		static_cast<util::Vector2Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,Vector4>)
	{
		if(nwVar.type != NetworkedVariable::Type::Vector4)
			return false;
		static_cast<util::Vector4Property&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,EntityHandle>)
	{
		if(nwVar.type != NetworkedVariable::Type::Entity)
			return false;
		static_cast<typename pragma::EntityProperty&>(*nwVar.data.get()) = val;
	}
	else if constexpr (std::is_same_v<T,Quat>)
	{
		if(nwVar.type != NetworkedVariable::Type::Quaternion)
			return false;
		static_cast<util::QuatProperty&>(*nwVar.data.get()) = val;
	}
	else
		return false;
	return true;
}

template<typename T>
	bool pragma::BaseNetworkedComponent::SetNetworkedVariable(const std::string &name,const T &val)
{
	auto it = std::find_if(m_networkedVarNames.begin(),m_networkedVarNames.end(),[&name](const std::string &nameOther) {
		return ustring::compare(name,nameOther,false);
	});
	if(it == m_networkedVarNames.end())
		return false;
	return SetNetworkedVariable(it -m_networkedVarNames.begin(),val);
}
#endif

#endif
