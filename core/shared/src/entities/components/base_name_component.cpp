#include "stdafx_shared.h"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

BaseNameComponent::BaseNameComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_name(util::StringProperty::Create())
{}
void BaseNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"name",false) || ustring::compare(kvData.key,"targetname",false))
			*m_name = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setname",false))
			*m_name = inputData.data;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	GetEntity().AddComponent("io");
}

const std::string &BaseNameComponent::GetName() const {return *m_name;}
void BaseNameComponent::SetName(std::string name) {*m_name = name;}
const util::PStringProperty &BaseNameComponent::GetNameProperty() const {return m_name;}

void BaseNameComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->WriteString(*m_name);
}

void BaseNameComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto name = ds->ReadString();
	SetName(name);
}
