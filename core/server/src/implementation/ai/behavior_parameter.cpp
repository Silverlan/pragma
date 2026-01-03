// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.schedule;

import :ai.schedule.behavior_parameter;

using namespace pragma;

ai::BehaviorParameter::Type ai::BehaviorParameter::GetType() const { return m_type; }
ai::BehaviorParameter::BehaviorParameter(bool b) : m_type(Type::Bool), m_data(pragma::util::make_shared<decltype(b)>(b)) {}
ai::BehaviorParameter::BehaviorParameter(int32_t i) : m_type(Type::Int), m_data(pragma::util::make_shared<decltype(i)>(i)) {}
ai::BehaviorParameter::BehaviorParameter(float f) : m_type(Type::Float), m_data(pragma::util::make_shared<decltype(f)>(f)) {}
ai::BehaviorParameter::BehaviorParameter(const std::string &s) : m_type(Type::String), m_data(pragma::util::make_shared<std::string>(s)) {}
ai::BehaviorParameter::BehaviorParameter(const ecs::BaseEntity *ent) : m_type(Type::Entity), m_data(pragma::util::make_shared<EntityHandle>((ent != nullptr) ? const_cast<ecs::BaseEntity *>(ent)->GetHandle() : EntityHandle {})) {}
ai::BehaviorParameter::BehaviorParameter(const Vector3 &vec) : m_type(Type::Vector), m_data(pragma::util::make_shared<Vector3>(vec)) {}
ai::BehaviorParameter::BehaviorParameter(const Quat &rot) : m_type(Type::Quaternion), m_data(pragma::util::make_shared<Quat>(rot)) {}
ai::BehaviorParameter::BehaviorParameter(const EulerAngles &ang) : m_type(Type::EulerAngles), m_data(pragma::util::make_shared<EulerAngles>(ang)) {}
ai::BehaviorParameter *ai::BehaviorParameter::Copy() const
{
	switch(m_type) {
	case Type::Bool:
		return new BehaviorParameter(GetBool());
	case Type::Int:
		return new BehaviorParameter(GetInt());
	case Type::Float:
		return new BehaviorParameter(GetFloat());
	case Type::String:
		return new BehaviorParameter(*GetString());
	case Type::Entity:
		return new BehaviorParameter(GetEntity());
	case Type::Vector:
		return new BehaviorParameter(*GetVector());
	case Type::Quaternion:
		return new BehaviorParameter(*GetQuaternion());
	case Type::EulerAngles:
		return new BehaviorParameter(*GetEulerAngles());
	default:
		return nullptr;
	}
}
bool ai::BehaviorParameter::GetBool() const { return (m_type == Type::Bool) ? *static_cast<bool *>(m_data.get()) : bool(); }
int32_t ai::BehaviorParameter::GetInt() const { return (m_type == Type::Int) ? *static_cast<int32_t *>(m_data.get()) : int32_t(); }
float ai::BehaviorParameter::GetFloat() const { return (m_type == Type::Float) ? *static_cast<float *>(m_data.get()) : float(); }
const std::string *ai::BehaviorParameter::GetString() const { return (m_type == Type::String) ? static_cast<std::string *>(m_data.get()) : nullptr; }
const ecs::BaseEntity *ai::BehaviorParameter::GetEntity() const { return (m_type == Type::Entity) ? static_cast<EntityHandle *>(m_data.get())->get() : nullptr; }
const Vector3 *ai::BehaviorParameter::GetVector() const { return (m_type == Type::Vector) ? static_cast<Vector3 *>(m_data.get()) : nullptr; }
const Quat *ai::BehaviorParameter::GetQuaternion() const { return (m_type == Type::Quaternion) ? static_cast<Quat *>(m_data.get()) : nullptr; }
const EulerAngles *ai::BehaviorParameter::GetEulerAngles() const { return (m_type == Type::EulerAngles) ? static_cast<EulerAngles *>(m_data.get()) : nullptr; }
std::string ai::BehaviorParameter::ToString() const
{
	switch(m_type) {
	case Type::Bool:
		return (GetBool() == true) ? "true" : "false";
	case Type::Int:
		return std::to_string(GetInt());
	case Type::Float:
		return std::to_string(GetFloat());
	case Type::String:
		{
			auto *str = GetString();
			return (str != nullptr) ? *str : "";
		}
	case Type::Entity:
	case Type::Vector:
	case Type::Quaternion:
	case Type::EulerAngles:
		{
			std::stringstream ss;
			if(m_type == Type::Entity) {
				auto *ent = GetEntity();
				if(ent != nullptr)
					ss << ent;
			}
			else if(m_type == Type::Vector) {
				auto *v = GetVector();
				ss << ((v != nullptr) ? *v : Vector3 {});
			}
			else if(m_type == Type::Quaternion) {
				auto *q = GetQuaternion();
				ss << ((q != nullptr) ? *q : uquat::identity());
			}
			else if(m_type == Type::EulerAngles) {
				auto *ang = GetEulerAngles();
				ss << ((ang != nullptr) ? *ang : EulerAngles {});
			}
			return ss.str();
		}
	}
	return "";
}

//////////////////////////////

ai::ParameterBase::ParameterInfo::ParameterInfo(BehaviorParameter *param) : parameter(param) {}
ai::ParameterBase::ParameterInfo::ParameterInfo(const ParameterInfo &other) : parameter(other.parameter->Copy()), links(other.links) {}

ai::ParameterBase::ParameterBase(const ParameterBase &other)
{
	m_params.reserve(other.m_params.size());
	for(auto &p : other.m_params)
		m_params.push_back(std::unique_ptr<ParameterInfo>(new ParameterInfo(*p)));
}
void ai::ParameterBase::SetParameter(uint8_t idx, bool b)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(b);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, int32_t i)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(i);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, float f)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(f);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, const std::string &s)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(s);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, const ecs::BaseEntity *ent)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(ent);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, const Vector3 &vec)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(vec);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, const Quat &rot)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(rot);
	OnParameterChanged(idx);
}
void ai::ParameterBase::SetParameter(uint8_t idx, const EulerAngles &ang)
{
	if(idx >= m_params.size())
		m_params.resize(idx + 1);
	auto &param = InitParameter(idx);
	param.parameter = std::make_unique<BehaviorParameter>(ang);
	OnParameterChanged(idx);
}
const ai::BehaviorParameter *ai::ParameterBase::GetParameter(uint8_t paramID) const
{
	if(paramID >= m_params.size())
		return nullptr;
	auto &param = m_params[paramID];
	if(param == nullptr)
		return nullptr;
	return param.get()->parameter.get();
}
ai::ParameterBase::ParameterInfo &ai::ParameterBase::InitParameter(uint8_t paramId)
{
	if(paramId >= m_params.size())
		m_params.resize(paramId + 1);
	if(m_params[paramId] == nullptr)
		m_params[paramId] = std::make_unique<ParameterInfo>();
	return *m_params[paramId];
}
void ai::ParameterBase::LinkParameter(uint8_t paramId, BehaviorNode &other, uint8_t paramIdOther)
{
	auto &info = InitParameter(paramId);
	info.links.push_back({&other, paramIdOther});
}
void ai::ParameterBase::LinkParameter(uint8_t paramId, BehaviorNode &other) { LinkParameter(paramId, other, paramId); }
void ai::ParameterBase::OnParameterChanged(uint8_t paramId)
{
	if(paramId >= m_params.size())
		return;
	auto &info = m_params[paramId];
	auto &param = *info->parameter;
	for(auto &link : info->links) {
		switch(param.GetType()) {
		case BehaviorParameter::Type::None:
			break;
		case BehaviorParameter::Type::Bool:
			link.first->SetParameter(link.second, param.GetBool());
			break;
		case BehaviorParameter::Type::Int:
			link.first->SetParameter(link.second, param.GetInt());
			break;
		case BehaviorParameter::Type::Float:
			link.first->SetParameter(link.second, param.GetFloat());
			break;
		case BehaviorParameter::Type::String:
			{
				auto *str = param.GetString();
				link.first->SetParameter(link.second, (str != nullptr) ? *str : "");
				break;
			}
		case BehaviorParameter::Type::Vector:
			{
				auto *v = param.GetVector();
				link.first->SetParameter(link.second, (v != nullptr) ? *v : Vector3 {});
				break;
			}
		case BehaviorParameter::Type::Quaternion:
			{
				auto *q = param.GetQuaternion();
				link.first->SetParameter(link.second, (q != nullptr) ? *q : uquat::identity());
				break;
			}
		case BehaviorParameter::Type::EulerAngles:
			{
				auto *a = param.GetEulerAngles();
				link.first->SetParameter(link.second, (a != nullptr) ? *a : EulerAngles {});
				break;
			}
		case BehaviorParameter::Type::Entity:
			link.first->SetParameter(link.second, param.GetEntity());
			break;
		}
	}
}
