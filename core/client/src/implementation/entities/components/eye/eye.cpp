// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.eye;
import :engine;
import :entities.components.render;

using namespace pragma;

// ComponentEventId CEyeComponent::EVENT_ON_EYEBALLS_UPDATED = INVALID_COMPONENT_ID;
// ComponentEventId CEyeComponent::EVENT_ON_BLINK = INVALID_COMPONENT_ID;
void CEyeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseAnimatedComponent::RegisterEvents(componentManager, registerEvent);
	// EVENT_ON_EYEBALLS_UPDATED = registerEvent("ON_EYEBALLS_UPDATED",ComponentEventInfo::Type::Explicit);
	// EVENT_ON_BLINK = registerEvent("EVENT_ON_BLINK");
}
void CEyeComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CEyeComponent;

	{
		auto coordMetaData = pragma::util::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = math::CoordinateSpace::Local;

		using TViewTarget = Vector3;
		auto memberInfo = create_component_member_info<T, TViewTarget, static_cast<void (T::*)(const Vector3 &)>(&T::SetViewTarget), [](const ComponentMemberInfo &, T &c, TViewTarget &value) { value = c.m_viewTarget; }>("viewTarget", Vector3 {});
		memberInfo.SetFlag(ComponentMemberFlags::ObjectSpace);
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
void CEyeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

CEyeComponent::CEyeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_stateFlags {StateFlags::BlinkingEnabled | StateFlags::PrevBlinkToggle} {}

void CEyeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnModelChanged(static_cast<CEOnModelChanged &>(evData.get()).model); });
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT, [this](std::reference_wrapper<ComponentEvent> evData) {
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

void CEyeComponent::SetBlinkingEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::BlinkingEnabled, enabled); };
bool CEyeComponent::IsBlinkingEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::BlinkingEnabled); }

bool CEyeComponent::FindEyeballIndex(uint32_t skinMatIdx, uint32_t &outEyeballIndex) const
{
	auto it = m_skinMaterialIndexToEyeballIndex.find(skinMatIdx);
	if(it == m_skinMaterialIndexToEyeballIndex.end())
		return false;
	outEyeballIndex = it->second;
	return true;
}
bool CEyeComponent::FindEyeballIndex(geometry::CModelSubMesh &subMesh, uint32_t &outEyeballIndex) const { return FindEyeballIndex(subMesh.GetSkinTextureIndex(), outEyeballIndex); }

void CEyeComponent::OnModelChanged(const std::shared_ptr<asset::Model> &mdl)
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
