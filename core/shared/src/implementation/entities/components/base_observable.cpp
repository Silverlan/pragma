// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_observable;

using namespace pragma;

ObserverCameraData::ObserverCameraData() : enabled {util::BoolProperty::Create(false)}, offset {util::Vector3Property::Create()} {}

/////////

void BaseObservableComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseObservableComponent::EVENT_ON_OBSERVER_CHANGED = registerEvent("ON_OBSERVER_CHANGED", ComponentEventInfo::Type::Broadcast); }
BaseObservableComponent::BaseObservableComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_viewOffset(0, 0, 0) {}

void BaseObservableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netSetObserverOffset = SetupNetEvent("set_observer_offset");
	m_netSetObserverOrigin = SetupNetEvent("set_observer_origin");
	m_netSetViewOffset = SetupNetEvent("set_view_offset");
}

void BaseObservableComponent::ClearObserver()
{
	if(m_observer.IsValid() == false)
		return;
	BroadcastEvent(baseObservableComponent::EVENT_ON_OBSERVER_CHANGED);
	m_observer = pragma::ComponentHandle<BaseObserverComponent> {};
}

void BaseObservableComponent::SetObserver(BaseObserverComponent *observer)
{
	if(observer == GetObserver())
		return;
	ClearObserver();
	if(!observer) {
		m_observer = pragma::ComponentHandle<BaseObserverComponent> {};
		BroadcastEvent(baseObservableComponent::EVENT_ON_OBSERVER_CHANGED);
		return;
	}
	m_observer = observer->GetHandle<BaseObserverComponent>();
	BroadcastEvent(baseObservableComponent::EVENT_ON_OBSERVER_CHANGED);
}
BaseObserverComponent *BaseObservableComponent::GetObserver() { return m_observer.get(); }

void BaseObservableComponent::SetLocalCameraOrigin(CameraType type, const Vector3 &origin) { GetCameraData(type).localOrigin = origin; }
void BaseObservableComponent::ClearLocalCameraOrigin(CameraType type) { GetCameraData(type).localOrigin = {}; }
Vector3 BaseObservableComponent::GetLocalCameraOrigin(CameraType type) const
{
	auto &localOrigin = GetCameraData(type).localOrigin;
	return localOrigin.has_value() ? *localOrigin : Vector3 {};
}
void BaseObservableComponent::SetLocalCameraOffset(CameraType type, const Vector3 &offset) { *GetCameraData(type).offset = offset; }
const Vector3 &BaseObservableComponent::GetLocalCameraOffset(CameraType type) const { return *GetCameraData(type).offset; }

const ObserverCameraData &BaseObservableComponent::GetCameraData(CameraType type) const { return const_cast<BaseObservableComponent *>(this)->GetCameraData(type); }
ObserverCameraData &BaseObservableComponent::GetCameraData(CameraType type) { return m_cameraData.at(math::to_integral(type)); }

void BaseObservableComponent::SetCameraEnabled(CameraType type, bool enabled) { *GetCameraData(type).enabled = enabled; }
bool BaseObservableComponent::IsCameraEnabled(CameraType type) const { return *GetCameraData(type).enabled; }

const util::PBoolProperty &BaseObservableComponent::GetCameraEnabledProperty(CameraType type) const { return GetCameraData(type).enabled; }
const util::PVector3Property &BaseObservableComponent::GetCameraOffsetProperty(CameraType type) const { return GetCameraData(type).offset; }

void BaseObservableComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	constexpr auto numTypes = math::to_integral(CameraType::Count);
	auto fWriteCameraData = [](udm::LinkedPropertyWrapperArg udm, ObserverCameraData &camData) {
		udm["enabled"] = **camData.enabled;
		if(camData.localOrigin.has_value())
			udm["localOrigin"] = *camData.localOrigin;
		udm["offset"] = **camData.offset;
		udm["rotateWithObservee"] = camData.rotateWithObservee;
		if(camData.angleLimits.has_value()) {
			udm["limits"]["min"] = camData.angleLimits->first;
			udm["limits"]["max"] = camData.angleLimits->second;
		}
	};
	auto &dataFp = GetCameraData(CameraType::FirstPerson);
	auto &dataTp = GetCameraData(CameraType::ThirdPerson);
	fWriteCameraData(udm["firstPerson"], dataFp);
	fWriteCameraData(udm["thirdPerson"], dataTp);
}

void BaseObservableComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);

	constexpr auto numTypes = math::to_integral(CameraType::Count);
	auto fReadCameraData = [](udm::LinkedPropertyWrapperArg udm, ObserverCameraData &camData) {
		udm["enabled"](**camData.enabled);
		camData.localOrigin = udm["localOrigin"].ToValue<Vector3>();
		udm["offset"](**camData.offset);
		udm["rotateWithObservee"](camData.rotateWithObservee);
		udm["limits"]["min"](camData.angleLimits->first);
		udm["limits"]["max"](camData.angleLimits->second);
	};
	auto &dataFp = GetCameraData(CameraType::FirstPerson);
	auto &dataTp = GetCameraData(CameraType::ThirdPerson);
	fReadCameraData(udm["firstPerson"], dataFp);
	fReadCameraData(udm["thirdPerson"], dataTp);
}

Vector3 &BaseObservableComponent::GetViewOffset() { return m_viewOffset; }
void BaseObservableComponent::SetViewOffset(const Vector3 &offset) { m_viewOffset = offset; }
