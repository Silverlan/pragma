#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/lua/luafunction_call.h"
#include "pragma/entities/components/base_networked_component.hpp"
#include "pragma/entities/entity_property.hpp"
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_color.hpp>
#include <sharedutils/property/util_property_quat.hpp>
#include <sharedutils/property/util_property_euler_angles.hpp>

using namespace pragma;

// Obsolete
#if 0
BaseNetworkedComponent::NetworkedVariable::NetworkedVariable(NetworkedVariable::Id id,util::VarType type,const std::shared_ptr<util::BaseProperty> &data)
	: id{id},type{type},data{data}
{}

BaseNetworkedComponent::NetworkedVariable::Id BaseNetworkedComponent::CreateNetworkedVariable(const std::string &name,NetworkedVariable::Type type)
{
	auto lname = name;
	ustring::to_lower(lname);
	BaseNetworkedComponent::NetworkedVariable::Id nwId;
	auto it = std::find(m_networkedVarNames.begin(),m_networkedVarNames.end(),lname);
	if(it != m_networkedVarNames.end())
	{
		auto idx = it -m_networkedVarNames.begin();
		auto &nwVar = m_networkedVars.at(idx);
		if(nwVar.type != type)
			throw std::logic_error("Attempted to create networked variable '" +name +"' with type " +std::to_string(umath::to_integral(type)) +" which has previously already been created with type " +std::to_string(umath::to_integral(nwVar.type)) +"!");
		nwId = idx;
	}
	else
	{
		std::shared_ptr<util::BaseProperty> prop = nullptr;
		switch(type)
		{
			case NetworkedVariable::Type::Bool:
				prop = util::BoolProperty::Create();
				break;
			case NetworkedVariable::Type::Double:
				prop = util::DoubleProperty::Create();
				break;
			case NetworkedVariable::Type::Float:
				prop = util::FloatProperty::Create();
				break;
			case NetworkedVariable::Type::Int8:
				prop = util::Int8Property::Create();
				break;
			case NetworkedVariable::Type::Int16:
				prop = util::Int16Property::Create();
				break;
			case NetworkedVariable::Type::Int32:
				prop = util::Int32Property::Create();
				break;
			case NetworkedVariable::Type::Int64:
				prop = util::Int64Property::Create();
				break;
			case NetworkedVariable::Type::LongDouble:
				prop = util::LongDoubleProperty::Create();
				break;
			case NetworkedVariable::Type::String:
				prop = util::StringProperty::Create();
				break;
			case NetworkedVariable::Type::UInt8:
				prop = util::UInt8Property::Create();
				break;
			case NetworkedVariable::Type::UInt16:
				prop = util::UInt16Property::Create();
				break;
			case NetworkedVariable::Type::UInt32:
				prop = util::UInt32Property::Create();
				break;
			case NetworkedVariable::Type::UInt64:
				prop = util::UInt64Property::Create();
				break;
			case NetworkedVariable::Type::EulerAngles:
				prop = util::EulerAnglesProperty::Create();
				break;
			case NetworkedVariable::Type::Color:
				prop = util::ColorProperty::Create();
				break;
			case NetworkedVariable::Type::Vector:
				prop = util::Vector3Property::Create();
				break;
			case NetworkedVariable::Type::Vector2:
				prop = util::Vector2Property::Create();
				break;
			case NetworkedVariable::Type::Vector4:
				prop = util::Vector4Property::Create();
				break;
			case NetworkedVariable::Type::Entity:
				prop = pragma::EntityProperty::Create();
				break;
			case NetworkedVariable::Type::Quaternion:
				prop = util::QuatProperty::Create();
				break;
			default:
				throw std::logic_error("Attempted to create networked variable of invalid type " +std::to_string(umath::to_integral(type)) +"!");
		}
		m_networkedVarNames.push_back(name);
		nwId = m_networkedVarNames.size() -1u;
		m_networkedVars.push_back({nwId,type,prop});
		OnNetworkedVariableCreated(m_networkedVars.back());
	}
	return nwId;
}

void BaseNetworkedComponent::OnNetworkedVariableCreated(NetworkedVariable &nwVar) {}

template<typename T>
	T BaseNetworkedComponent::GetNetworkedVariable(const std::string &name,NetworkedVariable::Type type) const
{
	auto it = std::find_if(m_networkedVarNames.begin(),m_networkedVarNames.end(),[&name](const std::string &nameOther) {
		return ustring::compare(name,nameOther,false);
	});
	if(it == m_networkedVars.end() || it->second->type != type)
		return T();
	return *static_cast<T*>(it->second->data.get());
}

const std::shared_ptr<util::BaseProperty> &BaseNetworkedComponent::GetNetworkedVariableProperty(NetworkedVariable::Id id) const
{
	if(id >= m_networkedVars.size())
	{
		static std::shared_ptr<util::BaseProperty> nptr = nullptr;
		return nptr;
	}
	return m_networkedVars.at(id).data;
}

BaseNetworkedComponent::NetworkedVariable::Type BaseNetworkedComponent::GetNetworkedVariableType(NetworkedVariable::Id id) const
{
	if(id >= m_networkedVars.size())
		return NetworkedVariable::Type::Invalid;
	return m_networkedVars.at(id).type;
}

const std::shared_ptr<util::BaseProperty> &BaseNetworkedComponent::GetNetworkedVariableProperty(const std::string &name) const
{
	auto it = std::find_if(m_networkedVarNames.begin(),m_networkedVarNames.end(),[&name](const std::string &nameOther) {
		return ustring::compare(name,nameOther,false);
	});
	if(it == m_networkedVarNames.end())
	{
		static std::shared_ptr<util::BaseProperty> nptr = nullptr;
		return nptr;
	}
	return GetNetworkedVariableProperty(it -m_networkedVarNames.begin());
}

const std::vector<BaseNetworkedComponent::NetworkedVariable> &BaseNetworkedComponent::GetNetworkedVariables() const {return m_networkedVars;}
#endif
