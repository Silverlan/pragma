// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :rendering.world_environment;
std::shared_ptr<WorldEnvironment> WorldEnvironment::Create() { return std::shared_ptr<WorldEnvironment>(new WorldEnvironment()); }

WorldEnvironment::WorldEnvironment()
{
	m_shaderQuality = std::make_shared<util::Int32Property>(int32_t {0});
	m_bUnlit = std::make_shared<util::BoolProperty>(bool {false});
	m_shadowResolution = std::make_shared<util::UInt32Property>(uint32_t {0});
}
int32_t WorldEnvironment::GetShaderQuality() const { return *m_shaderQuality; }
void WorldEnvironment::SetShaderQuality(int32_t quality) { *m_shaderQuality = quality; }
const std::shared_ptr<util::Int32Property> &WorldEnvironment::GetShaderQualityProperty() const { return m_shaderQuality; }

bool WorldEnvironment::IsUnlit() const { return *m_bUnlit; }
void WorldEnvironment::SetUnlit(bool b) { *m_bUnlit = b; }
const std::shared_ptr<util::BoolProperty> &WorldEnvironment::GetUnlitProperty() const { return m_bUnlit; }

uint32_t WorldEnvironment::GetShadowResolution() const { return *m_shadowResolution; }
void WorldEnvironment::SetShadowResolution(uint32_t resolution) { *m_shadowResolution = resolution; }
const std::shared_ptr<util::UInt32Property> &WorldEnvironment::GetShadowResolutionProperty() const { return m_shadowResolution; }

WorldEnvironment::Fog &WorldEnvironment::GetFogSettings() { return m_fog; }
