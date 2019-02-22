#include "stdafx_shared.h"
#include "pragma/entities/components/base_observable_component.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;


BaseObservableComponent::BaseObservableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_firstPersonObserverOffset(util::Vector3Property::Create()),
	m_thirdPersonObserverOffset(util::Vector3Property::Create()),m_bFirstPersonEnabled(util::BoolProperty::Create(false)),
	m_bThirdPersonEnabled(util::BoolProperty::Create(false))
{}

void BaseObservableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetFirstPersonObserverOffset = SetupNetEvent("set_first_person_observer_offset");
	m_netEvSetThirdPersonObserverOffset = SetupNetEvent("set_third_person_observer_offset");
}

void BaseObservableComponent::SetFirstPersonObserverOffsetEnabled(bool bEnabled) {*m_bFirstPersonEnabled = bEnabled;}
void BaseObservableComponent::SetThirdPersonObserverOffsetEnabled(bool bEnabled) {*m_bThirdPersonEnabled = bEnabled;}
bool BaseObservableComponent::IsFirstPersonObserverOffsetEnabled() const {return *m_bFirstPersonEnabled;}
bool BaseObservableComponent::IsThirdPersonObserverOffsetEnabled() const {return *m_bThirdPersonEnabled;}

void BaseObservableComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->Write<bool>(*m_bFirstPersonEnabled);
	if(*m_bFirstPersonEnabled)
		ds->Write<Vector3>(*m_firstPersonObserverOffset);
	ds->Write<bool>(*m_bThirdPersonEnabled);
	if(*m_bThirdPersonEnabled)
		ds->Write<Vector3>(*m_thirdPersonObserverOffset);
}

void BaseObservableComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto bFirstPersonObserverOffset = ds->Read<bool>();
	if(bFirstPersonObserverOffset == true)
	{
		auto offset = ds->Read<Vector3>();
		SetFirstPersonObserverOffset(offset);
	}

	auto bThirdPersonObserverOffset = ds->Read<bool>();
	if(bThirdPersonObserverOffset == true)
	{
		auto offset = ds->Read<Vector3>();
		SetThirdPersonObserverOffset(offset);
	}
}

void BaseObservableComponent::SetFirstPersonObserverOffset(const Vector3 &offset) {*m_firstPersonObserverOffset = offset;}
void BaseObservableComponent::SetThirdPersonObserverOffset(const Vector3 &offset) {*m_thirdPersonObserverOffset = offset;}
void BaseObservableComponent::SetObserverOffset(const Vector3 &offset)
{
	SetFirstPersonObserverOffset(offset);
	SetThirdPersonObserverOffset(offset);
}
const Vector3 &BaseObservableComponent::GetFirstPersonObserverOffset() const
{
	if(*m_bFirstPersonEnabled == false)
		return uvec::ORIGIN;
	return *m_firstPersonObserverOffset;
}
const Vector3 &BaseObservableComponent::GetThirdPersonObserverOffset() const
{
	if(*m_bThirdPersonEnabled == false)
		return uvec::ORIGIN;
	return *m_thirdPersonObserverOffset;
}
bool BaseObservableComponent::GetFirstPersonObserverOffset(Vector3 &offset) const
{
	if(*m_bFirstPersonEnabled == false)
		return false;
	offset = *m_firstPersonObserverOffset;
	return true;
}
bool BaseObservableComponent::GetThirdPersonObserverOffset(Vector3 &offset) const
{
	if(*m_bThirdPersonEnabled == false)
		return false;
	offset = *m_thirdPersonObserverOffset;
	return true;
}
void BaseObservableComponent::ResetFirstPersonObserverOffset()
{
	*m_bFirstPersonEnabled = false;
}
void BaseObservableComponent::ResetThirdPersonObserverOffset()
{
	*m_bThirdPersonEnabled = false;
}
void BaseObservableComponent::ResetObserverOffset()
{
	ResetFirstPersonObserverOffset();
	ResetThirdPersonObserverOffset();
}

const util::PBoolProperty &BaseObservableComponent::GetFirstPersonModeEnabledProperty() const {return m_bFirstPersonEnabled;}
const util::PBoolProperty &BaseObservableComponent::GetThirdPersonModeEnabledProperty() const {return m_bThirdPersonEnabled;}
bool BaseObservableComponent::IsFirstPersonModeEnabled() const {return *m_bFirstPersonEnabled;}
bool BaseObservableComponent::IsThirdPersonModeEnabled() const {return *m_bThirdPersonEnabled;}

const util::PVector3Property &BaseObservableComponent::GetFirstPersonObserverOffsetProperty() const {return m_firstPersonObserverOffset;}
const util::PVector3Property &BaseObservableComponent::GetThirdPersonObserverOffsetProperty() const {return m_thirdPersonObserverOffset;}
