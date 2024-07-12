/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_observable_component.hpp"
#include "pragma/entities/components/base_observer_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

pragma::ObserverCameraData::ObserverCameraData() : enabled {util::BoolProperty::Create(false)}, offset {util::Vector3Property::Create()} {}

/////////

ComponentEventId BaseObservableComponent::EVENT_ON_OBSERVER_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseObservableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_OBSERVER_CHANGED = registerEvent("ON_OBSERVER_CHANGED", ComponentEventInfo::Type::Broadcast); }
BaseObservableComponent::BaseObservableComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_viewOffset(0, 0, 0) {}

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
	BroadcastEvent(EVENT_ON_OBSERVER_CHANGED);
	m_observer = pragma::ComponentHandle<pragma::BaseObserverComponent> {};
}

void BaseObservableComponent::SetObserver(BaseObserverComponent *observer)
{
	if(observer == GetObserver())
		return;
	ClearObserver();
	if(!observer) {
		m_observer = pragma::ComponentHandle<BaseObserverComponent> {};
		BroadcastEvent(EVENT_ON_OBSERVER_CHANGED);
		return;
	}
	m_observer = observer->GetHandle<BaseObserverComponent>();
	BroadcastEvent(EVENT_ON_OBSERVER_CHANGED);
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
ObserverCameraData &BaseObservableComponent::GetCameraData(CameraType type) { return m_cameraData.at(umath::to_integral(type)); }

void BaseObservableComponent::SetCameraEnabled(CameraType type, bool enabled) { *GetCameraData(type).enabled = enabled; }
bool BaseObservableComponent::IsCameraEnabled(CameraType type) const { return *GetCameraData(type).enabled; }

const util::PBoolProperty &BaseObservableComponent::GetCameraEnabledProperty(CameraType type) const { return GetCameraData(type).enabled; }
const util::PVector3Property &BaseObservableComponent::GetCameraOffsetProperty(CameraType type) const { return GetCameraData(type).offset; }

void BaseObservableComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
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

	constexpr auto numTypes = umath::to_integral(CameraType::Count);
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
