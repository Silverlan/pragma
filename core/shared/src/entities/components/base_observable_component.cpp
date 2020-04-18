#include "stdafx_shared.h"
#include "pragma/entities/components/base_observable_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;


pragma::ObserverCameraData::ObserverCameraData()
	: enabled{util::BoolProperty::Create(false)},
	offset{util::Vector3Property::Create()}
{}

/////////

BaseObservableComponent::BaseObservableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void BaseObservableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netSetObserverOffset = SetupNetEvent("set_observer_offset");
}

void BaseObservableComponent::SetLocalCameraOrigin(CameraType type,const Vector3 &origin)
{
	GetCameraData(type).localOrigin = origin;
}
void BaseObservableComponent::ClearLocalCameraOrigin(CameraType type)
{
	GetCameraData(type).localOrigin = {};
}
Vector3 BaseObservableComponent::GetLocalCameraOrigin(CameraType type) const
{
	auto &localOrigin = GetCameraData(type).localOrigin;
	return localOrigin.has_value() ? *localOrigin : Vector3{};
}
void BaseObservableComponent::SetLocalCameraOffset(CameraType type,const Vector3 &offset)
{
	*GetCameraData(type).offset = offset;
}
const Vector3 &BaseObservableComponent::GetLocalCameraOffset(CameraType type) const {return *GetCameraData(type).offset;}

const ObserverCameraData &BaseObservableComponent::GetCameraData(CameraType type) const {return const_cast<BaseObservableComponent*>(this)->GetCameraData(type);}
ObserverCameraData &BaseObservableComponent::GetCameraData(CameraType type) {return m_cameraData.at(umath::to_integral(type));}

void BaseObservableComponent::SetCameraEnabled(CameraType type,bool enabled) {*GetCameraData(type).enabled = enabled;}
bool BaseObservableComponent::IsCameraEnabled(CameraType type) const {return *GetCameraData(type).enabled;}

const util::PBoolProperty &BaseObservableComponent::GetCameraEnabledProperty(CameraType type) const {return GetCameraData(type).enabled;}
const util::PVector3Property &BaseObservableComponent::GetCameraOffsetProperty(CameraType type) const {return GetCameraData(type).offset;}

void BaseObservableComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
	for(auto i=0u;i<numTypes;++i)
	{
		auto &data = GetCameraData(static_cast<CameraType>(i));
		ds->Write<bool>(*data.enabled);
		ds->Write<Vector3>(*data.localOrigin);
		ds->Write<Vector3>(*data.offset);
		ds->Write<bool>(data.rotateWithObservee);
		auto hasLimits = data.angleLimits.has_value();
		ds->Write<bool>(hasLimits);
		if(hasLimits)
		{
			ds->Write<EulerAngles>(data.angleLimits->first);
			ds->Write<EulerAngles>(data.angleLimits->second);
		}
	}
}

void BaseObservableComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
	for(auto i=0u;i<numTypes;++i)
	{
		auto &data = GetCameraData(static_cast<CameraType>(i));
		*data.enabled = ds->Read<bool>();
		*data.localOrigin = ds->Read<Vector3>();
		*data.offset = ds->Read<Vector3>();
		data.rotateWithObservee = ds->Read<bool>();
		auto hasLimits = ds->Read<bool>();
		if(hasLimits)
		{
			auto minLimits = ds->Read<EulerAngles>();
			auto maxLimits = ds->Read<EulerAngles>();
			data.angleLimits = {minLimits,maxLimits};
		}
	}
}

