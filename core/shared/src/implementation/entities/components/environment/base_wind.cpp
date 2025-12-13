// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_wind;

using namespace pragma;

void BaseEnvWindComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "wind_direction", false))
			SetWindDirection(uvec::create(kvData.value));
		else if(pragma::string::compare<std::string>(kvData.key, "wind_speed", false))
			SetWindSpeed(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BaseEnvWindComponent::SetWindForce(const Vector3 &force) { m_windForce = force; }
void BaseEnvWindComponent::SetWindDirection(const Vector3 &dir) { SetWindForce(dir * GetWindSpeed()); }
void BaseEnvWindComponent::SetWindSpeed(float speed) { SetWindForce(GetWindDirection() * speed); }
const Vector3 &BaseEnvWindComponent::GetWindForce() const { return m_windForce; }
Vector3 BaseEnvWindComponent::GetWindDirection() const { return uvec::get_normal(GetWindForce()); }
float BaseEnvWindComponent::GetWindSpeed() const { return uvec::length(GetWindForce()); }
