/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/component_member_flags.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

// ComponentEventId CEyeComponent::EVENT_ON_EYEBALLS_UPDATED = INVALID_COMPONENT_ID;
// ComponentEventId CEyeComponent::EVENT_ON_BLINK = INVALID_COMPONENT_ID;
void CEyeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseAnimatedComponent::RegisterEvents(componentManager, registerEvent);
	// EVENT_ON_EYEBALLS_UPDATED = registerEvent("ON_EYEBALLS_UPDATED",ComponentEventInfo::Type::Explicit);
	// EVENT_ON_BLINK = registerEvent("EVENT_ON_BLINK");
}
void CEyeComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CEyeComponent;

	{
		auto coordMetaData = std::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = umath::CoordinateSpace::Local;

		using TViewTarget = Vector3;
		auto memberInfo = create_component_member_info<T, TViewTarget, static_cast<void (T::*)(const Vector3 &)>(&T::SetViewTarget), [](const ComponentMemberInfo &, T &c, TViewTarget &value) { value = c.m_viewTarget; }>("viewTarget", Vector3 {});
		memberInfo.SetFlag(pragma::ComponentMemberFlags::ObjectSpace);
		memberInfo.AddTypeMetaData(coordMetaData);
		registerMember(std::move(memberInfo));
	}

	{
		using TBlinkDuration = float;
		auto memberInfo = create_component_member_info<T, TBlinkDuration, static_cast<void (T::*)(TBlinkDuration)>(&T::SetBlinkDuration), static_cast<TBlinkDuration (T::*)() const>(&T::GetBlinkDuration)>("blinkDuration", 0.2f);
		registerMember(std::move(memberInfo));
	}

	{
		using TBlinkingEnabled = bool;
		auto memberInfo = create_component_member_info<T, TBlinkingEnabled, static_cast<void (T::*)(TBlinkingEnabled)>(&T::SetBlinkingEnabled), static_cast<TBlinkingEnabled (T::*)() const>(&T::IsBlinkingEnabled)>("blinkingEnabled", true);
		registerMember(std::move(memberInfo));
	}

	{
		using TLocalViewTargetFactor = float;
		auto memberInfo = create_component_member_info<T, TLocalViewTargetFactor, static_cast<void (T::*)(TLocalViewTargetFactor)>(&T::SetLocalViewTargetFactor), static_cast<TLocalViewTargetFactor (T::*)() const>(&T::GetLocalViewTargetFactor)>("localViewTargetFactor", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
}
void CEyeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

CEyeComponent::CEyeComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_stateFlags {StateFlags::BlinkingEnabled | StateFlags::PrevBlinkToggle} {}

void CEyeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnModelChanged(static_cast<pragma::CEOnModelChanged &>(evData.get()).model); });
	BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
		if(mdlC == nullptr || mdlC->GetLOD() > 0)
			return;
		UpdateEyeballsMT();
		UpdateBlinkMT();
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

void CEyeComponent::SetBlinkDuration(float dur) { m_blinkDuration = dur; }
float CEyeComponent::GetBlinkDuration() const { return m_blinkDuration; }

void CEyeComponent::SetBlinkingEnabled(bool enabled) { umath::set_flag(m_stateFlags, StateFlags::BlinkingEnabled, enabled); };
bool CEyeComponent::IsBlinkingEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::BlinkingEnabled); }

bool CEyeComponent::FindEyeballIndex(uint32_t skinMatIdx, uint32_t &outEyeballIndex) const
{
	auto it = m_skinMaterialIndexToEyeballIndex.find(skinMatIdx);
	if(it == m_skinMaterialIndexToEyeballIndex.end())
		return false;
	outEyeballIndex = it->second;
	return true;
}
bool CEyeComponent::FindEyeballIndex(CModelSubMesh &subMesh, uint32_t &outEyeballIndex) const { return FindEyeballIndex(subMesh.GetSkinTextureIndex(), outEyeballIndex); }

void CEyeComponent::OnModelChanged(const std::shared_ptr<Model> &mdl)
{
	m_eyeballData.clear();
	m_blinkFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeUpDownFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeLeftRightFlexController = std::numeric_limits<uint32_t>::max();
	m_eyeAttachmentIndex = std::numeric_limits<uint32_t>::max();
	m_skinMaterialIndexToEyeballIndex.clear();

	if(mdl == nullptr)
		return;

	mdl->GetFlexControllerId("blink", m_blinkFlexController);
	mdl->GetFlexControllerId("eyes_updown", m_eyeUpDownFlexController);
	mdl->GetFlexControllerId("eyes_rightleft", m_eyeLeftRightFlexController);
	auto attEyeId = mdl->LookupAttachment("eyes");
	m_eyeAttachmentIndex = (attEyeId != -1) ? attEyeId : std::numeric_limits<uint32_t>::max();

	auto numEyeballs = mdl->GetEyeballCount();
	if(numEyeballs == 0)
		return;
	m_eyeballData.resize(mdl->GetEyeballCount());

	for(auto eyeballIndex = decltype(numEyeballs) {0u}; eyeballIndex < numEyeballs; ++eyeballIndex) {
		auto &eyeball = *mdl->GetEyeball(eyeballIndex);
		m_skinMaterialIndexToEyeballIndex.insert(std::make_pair(eyeball.irisMaterialIndex, eyeballIndex));
	}

	UpdateEyeMaterialData();
}
