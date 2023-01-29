/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/world_environment.hpp"
#include <sharedutils/property/util_property_vector.h>

std::shared_ptr<WorldEnvironment> WorldEnvironment::Create() { return std::shared_ptr<WorldEnvironment>(new WorldEnvironment()); }

WorldEnvironment::WorldEnvironment()
{
	m_ambientColor = std::make_shared<util::Vector4Property>(Vector4 {});
	m_shaderQuality = std::make_shared<util::Int32Property>(int32_t {0});
	m_bUnlit = std::make_shared<util::BoolProperty>(bool {false});
	m_shadowResolution = std::make_shared<util::UInt32Property>(uint32_t {0});
}
const Vector4 &WorldEnvironment::GetAmbientColor() const { return *m_ambientColor; }
void WorldEnvironment::SetAmbientColor(const Vector4 &col) { *m_ambientColor = col; }
const std::shared_ptr<util::Vector4Property> &WorldEnvironment::GetAmbientColorProperty() const { return m_ambientColor; }

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
