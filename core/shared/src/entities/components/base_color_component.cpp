#include "stdafx_shared.h"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

ComponentEventId BaseColorComponent::EVENT_ON_COLOR_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseColorComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_COLOR_CHANGED = componentManager.RegisterEvent("ON_COLOR_CHANGED");
}
BaseColorComponent::BaseColorComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_color(util::SimpleProperty<util::ColorProperty,Color>::Create(Color(255,255,255,255)))
{}
BaseColorComponent::~BaseColorComponent()
{
	if(m_cbOnColorChanged.IsValid())
		m_cbOnColorChanged.Remove();
}
void BaseColorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"color",false))
			*m_color = Color{kvData.value};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setcolor",false))
			*m_color = Color{inputData.data};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnColorChanged = m_color->AddCallback([this](std::reference_wrapper<const Color> oldColor,std::reference_wrapper<const Color> newColor) {
		pragma::CEOnColorChanged onColorChanged{oldColor.get(),newColor.get()};
		BroadcastEvent(EVENT_ON_COLOR_CHANGED,onColorChanged);
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	m_netEvSetColor = SetupNetEvent("set_color");
}
const Color &BaseColorComponent::GetColor() const {return *m_color;}
const util::PColorProperty &BaseColorComponent::GetColorProperty() const {return m_color;}

void BaseColorComponent::SetColor(const Color &color) {*m_color = color;}
void BaseColorComponent::SetColor(const Vector4 &color) {*m_color = color;}

/////////////////

CEOnColorChanged::CEOnColorChanged(const Color &oldColor,const Color &color)
	: oldColor{oldColor},color{color}
{}
void CEOnColorChanged::PushArguments(lua_State *l)
{
	Lua::Push<Color>(l,oldColor);
	Lua::Push<Color>(l,color);
}
