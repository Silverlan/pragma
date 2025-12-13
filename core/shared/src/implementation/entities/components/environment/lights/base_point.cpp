// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.lights.base_point;

using namespace pragma;

Candela BaseEnvLightPointComponent::CalcIntensityAtPoint(const Vector3 &lightPos, Candela intensity, const Vector3 &point, std::optional<float> radius) { return intensity * BaseEnvLightComponent::CalcDistanceFalloff(lightPos, point, radius); }
void BaseEnvLightPointComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
}
util::EventReply BaseEnvLightPointComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == baseEnvLightComponent::EVENT_CALC_LIGHT_DIRECTION_TO_POINT) {
		auto &levData = static_cast<CECalcLightDirectionToPoint &>(evData);
		auto dir = levData.pos - GetEntity().GetPosition();
		uvec::normalize(&dir);
		levData.direction = dir;
		return util::EventReply::Handled;
	}
	else if(eventId == baseEnvLightComponent::EVENT_CALC_LIGHT_INTENSITY_AT_POINT) {
		auto &levData = static_cast<CECalcLightIntensityAtPoint &>(evData);
		auto *cLight = dynamic_cast<BaseEnvLightComponent *>(GetEntity().FindComponent("light").get());
		if(cLight) {
			auto *radiusC = dynamic_cast<BaseRadiusComponent *>(GetEntity().FindComponent("radius").get());
			auto radius = radiusC ? radiusC->GetRadius() : 0.f;
			static_cast<CECalcLightIntensityAtPoint &>(evData).intensity = CalcIntensityAtPoint(GetEntity().GetPosition(), cLight->GetLightIntensityCandela(), levData.pos, radius);
		}
		return util::EventReply::Handled;
	}
	return BaseEntityComponent::HandleEvent(eventId, evData);
}
float BaseEnvLightPointComponent::CalcDistanceFalloff(const Vector3 &point) const
{
	auto *radiusC = dynamic_cast<BaseRadiusComponent *>(GetEntity().FindComponent("radius").get());
	if(!radiusC)
		return 0.f;
	return BaseEnvLightComponent::CalcDistanceFalloff(GetEntity().GetPosition(), point, radiusC->GetRadius());
}
