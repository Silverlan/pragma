// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.lights.base_directional;

using namespace pragma;

Candela BaseEnvLightDirectionalComponent::CalcIntensityAtPoint(Candela intensity, const Vector3 &point) { return intensity; }
BaseEnvLightDirectionalComponent::BaseEnvLightDirectionalComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_ambientColor(util::ColorProperty::Create(Color(255, 255, 255, 200))) {}
void BaseEnvLightDirectionalComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "color_ambient", false))
			*m_ambientColor = Color(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "max_exposure", false))
			m_maxExposure = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	auto lightC = ent.AddComponent("light");
	m_netEvSetAmbientColor = SetupNetEvent("set_ambient_color");

	if(lightC.valid()) {
		auto *pLightC = static_cast<BaseEnvLightComponent *>(lightC.get());
		pLightC->SetLightIntensityType(BaseEnvLightComponent::LightIntensityType::Lux);
		pLightC->SetLightIntensity(30.f);
	}
}

void BaseEnvLightDirectionalComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["maxExposure"] = m_maxExposure;
	udm["ambientColor"] = (*m_ambientColor)->ToVector4();
}
void BaseEnvLightDirectionalComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["maxExposure"](m_maxExposure);
	Vector4 color;
	udm["ambientColor"](color);
	*m_ambientColor = Color {color};
}
util::EventReply BaseEnvLightDirectionalComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == baseEnvLightComponent::EVENT_CALC_LIGHT_DIRECTION_TO_POINT) {
		static_cast<CECalcLightDirectionToPoint &>(evData).direction = GetEntity().GetForward();
		return util::EventReply::Handled;
	}
	else if(eventId == baseEnvLightComponent::EVENT_CALC_LIGHT_INTENSITY_AT_POINT) {
		auto *cLight = dynamic_cast<BaseEnvLightComponent *>(GetEntity().FindComponent("light").get());
		if(cLight) {
			auto intensity = cLight->GetLightIntensityCandela();
			auto &levData = static_cast<CECalcLightIntensityAtPoint &>(evData);
			static_cast<CECalcLightIntensityAtPoint &>(evData).intensity = CalcIntensityAtPoint(intensity, levData.pos);
		}
		return util::EventReply::Handled;
	}
	return BaseEntityComponent::HandleEvent(eventId, evData);
}

void BaseEnvLightDirectionalComponent::SetAmbientColor(const Color &color) { *m_ambientColor = color; }
const Color &BaseEnvLightDirectionalComponent::GetAmbientColor() const { return *m_ambientColor; }
const util::PColorProperty &BaseEnvLightDirectionalComponent::GetAmbientColorProperty() const { return m_ambientColor; }
