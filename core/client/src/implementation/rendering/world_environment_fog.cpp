// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;



module pragma.client;


import :rendering.world_environment;
WorldEnvironment::Fog::Fog()
{
	m_color = std::make_shared<util::ColorProperty>(colors::White);
	m_start = std::make_shared<util::FloatProperty>(0.f);
	m_end = std::make_shared<util::FloatProperty>(0.f);
	m_maxDensity = std::make_shared<util::FloatProperty>(0.f);
	m_type = std::make_shared<util::UInt8Property>(umath::to_integral(util::FogType::Linear));
	m_bEnabled = std::make_shared<util::BoolProperty>(false);
}
void WorldEnvironment::Fog::SetColor(const Color &col) { *m_color = col; }
const Color &WorldEnvironment::Fog::GetColor() const { return *m_color; }
const std::shared_ptr<util::ColorProperty> &WorldEnvironment::Fog::GetColorProperty() const { return m_color; }

void WorldEnvironment::Fog::SetStart(float start) { *m_start = start; }
float WorldEnvironment::Fog::GetStart() const { return *m_start; }
const std::shared_ptr<util::FloatProperty> &WorldEnvironment::Fog::GetStartProperty() const { return m_start; }

void WorldEnvironment::Fog::SetEnd(float end) { *m_end = end; }
float WorldEnvironment::Fog::GetEnd() const { return *m_end; }
const std::shared_ptr<util::FloatProperty> &WorldEnvironment::Fog::GetEndProperty() const { return m_end; }

void WorldEnvironment::Fog::SetMaxDensity(float density) { *m_maxDensity = density; }
float WorldEnvironment::Fog::GetMaxDensity() const { return *m_maxDensity; }
const std::shared_ptr<util::FloatProperty> &WorldEnvironment::Fog::GetMaxDensityProperty() const { return m_maxDensity; }

void WorldEnvironment::Fog::SetType(util::FogType type) { *m_type = umath::to_integral(type); }
util::FogType WorldEnvironment::Fog::GetType() const { return static_cast<util::FogType>(**m_type); }
const std::shared_ptr<util::UInt8Property> &WorldEnvironment::Fog::GetTypeProperty() const { return m_type; }

void WorldEnvironment::Fog::SetEnabled(bool bEnabled) { *m_bEnabled = bEnabled; }
bool WorldEnvironment::Fog::IsEnabled() const { return *m_bEnabled; }
const std::shared_ptr<util::BoolProperty> &WorldEnvironment::Fog::GetEnabledProperty() const { return m_bEnabled; }

Vector2 WorldEnvironment::Fog::GetRange() const { return Vector2 {GetStart(), GetEnd()}; }
void WorldEnvironment::Fog::SetRange(const Vector2 &range) { SetRange(range.x, range.y); }
void WorldEnvironment::Fog::SetRange(float start, float end)
{
	SetStart(start);
	SetEnd(end);
}
float WorldEnvironment::Fog::GetFarDistance() const
{
	switch(GetType()) {
	case util::FogType::Exponential:
		return logf(1.f / 0.0019f) / GetMaxDensity();
	case util::FogType::Exponential2:
		return sqrtf(logf(1.f / 0.0019f)) / GetMaxDensity();
	}
	return GetEnd();
}
