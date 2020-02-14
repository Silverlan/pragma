#include "stdafx_client.h"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
ComponentEventId CEyeComponent::EVENT_ON_EYEBALLS_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId CEyeComponent::EVENT_ON_BLINK = INVALID_COMPONENT_ID;
void CEyeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseAnimatedComponent::RegisterEvents(componentManager);
	EVENT_ON_EYEBALLS_UPDATED = componentManager.RegisterEvent("ON_EYEBALLS_UPDATED",std::type_index(typeid(CEyeComponent)));
	EVENT_ON_BLINK = componentManager.RegisterEvent("EVENT_ON_BLINK");
}
luabind::object CEyeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CEyeComponentHandleWrapper>(l);}

CEyeComponent::CEyeComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_stateFlags{StateFlags::BlinkingEnabled | StateFlags::PrevBlinkToggle}
{}

void CEyeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnModelChanged(static_cast<pragma::CEOnModelChanged&>(evData.get()).model);
	});
	BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(static_cast<CEOnUpdateRenderData&>(evData.get()).firstUpdateThisFrame == false)
			return;
		UpdateEyeballs();
		UpdateBlink();
	});
	GetEntity().AddComponent<CAnimatedComponent>();
	GetEntity().AddComponent<CFlexComponent>();
	OnModelChanged(GetEntity().GetModel());
}

void CEyeComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CFlexComponent))
		m_flexC = component.GetHandle<CFlexComponent>();
	else if(typeid(component) == typeid(CAnimatedComponent))
		m_animC = component.GetHandle<CAnimatedComponent>();
}

void CEyeComponent::SetBlinkDuration(float dur) {m_blinkDuration = dur;}
float CEyeComponent::GetBlinkDuration() const {return m_blinkDuration;}

void CEyeComponent::SetBlinkingEnabled(bool enabled) {umath::set_flag(m_stateFlags,StateFlags::BlinkingEnabled,enabled);};
bool CEyeComponent::IsBlinkingEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::BlinkingEnabled);}

bool CEyeComponent::FindEyeballIndex(uint32_t skinMatIdx,uint32_t &outEyeballIndex) const
{
	auto it = m_skinMaterialIndexToEyeballIndex.find(skinMatIdx);
	if(it == m_skinMaterialIndexToEyeballIndex.end())
		return false;
	outEyeballIndex = it->second;
	return true;
}
bool CEyeComponent::FindEyeballIndex(CModelSubMesh &subMesh,uint32_t &outEyeballIndex) const {return FindEyeballIndex(subMesh.GetSkinTextureIndex(),outEyeballIndex);}

void CEyeComponent::OnModelChanged(const std::shared_ptr<Model> &mdl)
{
	m_eyeballData.clear();
	m_blinkFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeUpDownFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeLeftRightFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeAttachmentIndex = std::numeric_limits<uint32_t>::max();
	m_skinMaterialIndexToEyeballIndex.clear();
	auto numEyeballs = mdl ? mdl->GetEyeballCount() : 0u;
	if(mdl == nullptr || numEyeballs == 0)
		return;
	m_eyeballData.resize(mdl->GetEyeballCount());
	mdl->GetFlexControllerId("blink",m_blinkFlexController);
	mdl->GetFlexControllerId("eyes_updown",m_eyeUpDownFlexController);
	mdl->GetFlexControllerId("eyes_rightleft",m_eyeLeftRightFlexController);
	auto attEyeId = mdl->LookupAttachment("eyes");
	m_eyeAttachmentIndex = (attEyeId != -1) ? attEyeId : std::numeric_limits<uint32_t>::max();

	for(auto eyeballIndex=decltype(numEyeballs){0u};eyeballIndex<numEyeballs;++eyeballIndex)
	{
		auto &eyeball = *mdl->GetEyeball(eyeballIndex);
		m_skinMaterialIndexToEyeballIndex.insert(std::make_pair(eyeball.irisMaterialIndex,eyeballIndex));
	}
}
#pragma optimize("",on)
