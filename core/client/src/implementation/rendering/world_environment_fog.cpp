// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :rendering.world_environment;
pragma::rendering::WorldEnvironment::Fog::Fog()
{
	m_color = ::util::make_shared<util::ColorProperty>(colors::White);
	m_start = ::util::make_shared<util::FloatProperty>(0.f);
	m_end = ::util::make_shared<util::FloatProperty>(0.f);
	m_maxDensity = ::util::make_shared<util::FloatProperty>(0.f);
	m_type = ::util::make_shared<util::UInt8Property>(umath::to_integral(util::FogType::Linear));
	m_bEnabled = ::util::make_shared<util::BoolProperty>(false);
}
void pragma::rendering::WorldEnvironment::Fog::SetColor(const Color &col) { *m_color = col; }
const Color &pragma::rendering::WorldEnvironment::Fog::GetColor() const { return *m_color; }
const std::shared_ptr<util::ColorProperty> &pragma::rendering::WorldEnvironment::Fog::GetColorProperty() const { return m_color; }

void pragma::rendering::WorldEnvironment::Fog::SetStart(float start) { *m_start = start; }
float pragma::rendering::WorldEnvironment::Fog::GetStart() const { return *m_start; }
const std::shared_ptr<util::FloatProperty> &pragma::rendering::WorldEnvironment::Fog::GetStartProperty() const { return m_start; }

void pragma::rendering::WorldEnvironment::Fog::SetEnd(float end) { *m_end = end; }
float pragma::rendering::WorldEnvironment::Fog::GetEnd() const { return *m_end; }
const std::shared_ptr<util::FloatProperty> &pragma::rendering::WorldEnvironment::Fog::GetEndProperty() const { return m_end; }

void pragma::rendering::WorldEnvironment::Fog::SetMaxDensity(float density) { *m_maxDensity = density; }
float pragma::rendering::WorldEnvironment::Fog::GetMaxDensity() const { return *m_maxDensity; }
const std::shared_ptr<util::FloatProperty> &pragma::rendering::WorldEnvironment::Fog::GetMaxDensityProperty() const { return m_maxDensity; }

void pragma::rendering::WorldEnvironment::Fog::SetType(util::FogType type) { *m_type = umath::to_integral(type); }
util::FogType pragma::rendering::WorldEnvironment::Fog::GetType() const { return static_cast<util::FogType>(**m_type); }
const std::shared_ptr<util::UInt8Property> &pragma::rendering::WorldEnvironment::Fog::GetTypeProperty() const { return m_type; }

void pragma::rendering::WorldEnvironment::Fog::SetEnabled(bool bEnabled) { *m_bEnabled = bEnabled; }
bool pragma::rendering::WorldEnvironment::Fog::IsEnabled() const { return *m_bEnabled; }
const std::shared_ptr<util::BoolProperty> &pragma::rendering::WorldEnvironment::Fog::GetEnabledProperty() const { return m_bEnabled; }

Vector2 pragma::rendering::WorldEnvironment::Fog::GetRange() const { return Vector2 {GetStart(), GetEnd()}; }
void pragma::rendering::WorldEnvironment::Fog::SetRange(const Vector2 &range) { SetRange(range.x, range.y); }
void pragma::rendering::WorldEnvironment::Fog::SetRange(float start, float end)
{
	SetStart(start);
	SetEnd(end);
}
float pragma::rendering::WorldEnvironment::Fog::GetFarDistance() const
{
	switch(GetType()) {
	case util::FogType::Exponential:
		return logf(1.f / 0.0019f) / GetMaxDensity();
	case util::FogType::Exponential2:
		return sqrtf(logf(1.f / 0.0019f)) / GetMaxDensity();
	}
	return GetEnd();
}
