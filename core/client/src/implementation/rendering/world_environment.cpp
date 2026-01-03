// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.world_environment;
std::shared_ptr<pragma::rendering::WorldEnvironment> pragma::rendering::WorldEnvironment::Create() { return std::shared_ptr<WorldEnvironment>(new WorldEnvironment()); }

pragma::rendering::WorldEnvironment::WorldEnvironment()
{
	m_shaderQuality = pragma::util::make_shared<util::Int32Property>(int32_t {0});
	m_bUnlit = pragma::util::make_shared<util::BoolProperty>(bool {false});
	m_shadowResolution = pragma::util::make_shared<util::UInt32Property>(uint32_t {0});
}
int32_t pragma::rendering::WorldEnvironment::GetShaderQuality() const { return *m_shaderQuality; }
void pragma::rendering::WorldEnvironment::SetShaderQuality(int32_t quality) { *m_shaderQuality = quality; }
const std::shared_ptr<pragma::util::Int32Property> &pragma::rendering::WorldEnvironment::GetShaderQualityProperty() const { return m_shaderQuality; }

bool pragma::rendering::WorldEnvironment::IsUnlit() const { return *m_bUnlit; }
void pragma::rendering::WorldEnvironment::SetUnlit(bool b) { *m_bUnlit = b; }
const std::shared_ptr<pragma::util::BoolProperty> &pragma::rendering::WorldEnvironment::GetUnlitProperty() const { return m_bUnlit; }

uint32_t pragma::rendering::WorldEnvironment::GetShadowResolution() const { return *m_shadowResolution; }
void pragma::rendering::WorldEnvironment::SetShadowResolution(uint32_t resolution) { *m_shadowResolution = resolution; }
const std::shared_ptr<pragma::util::UInt32Property> &pragma::rendering::WorldEnvironment::GetShadowResolutionProperty() const { return m_shadowResolution; }

pragma::rendering::WorldEnvironment::Fog &pragma::rendering::WorldEnvironment::GetFogSettings() { return m_fog; }
