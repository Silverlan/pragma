// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_render;

using namespace pragma;

void BaseRenderComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseRenderComponent;

	{
		using TCastShadows = bool;
		auto memberInfo = create_component_member_info<T, TCastShadows, static_cast<void (T::*)(TCastShadows)>(&T::SetCastShadows), static_cast<TCastShadows (T::*)() const>(&T::GetCastShadows)>("castShadows", true);
		registerMember(std::move(memberInfo));
	}
}
void BaseRenderComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "disableshadows", false))
			SetCastShadows(!util::to_boolean(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseRenderComponent::SetUnlit(bool b)
{
	if(b)
		m_renderFlags |= FRenderFlags::Unlit;
	else
		m_renderFlags &= ~FRenderFlags::Unlit;
}
void BaseRenderComponent::SetCastShadows(bool b)
{
	if(b)
		m_renderFlags |= FRenderFlags::CastShadows;
	else
		m_renderFlags &= ~FRenderFlags::CastShadows;
}
bool BaseRenderComponent::IsUnlit() const { return (m_renderFlags & FRenderFlags::Unlit) != FRenderFlags::None; }
bool BaseRenderComponent::GetCastShadows() const { return (m_renderFlags & FRenderFlags::CastShadows) != FRenderFlags::None; }

void BaseRenderComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm::write_flag(udm["renderFlags"], m_renderFlags, FRenderFlags::CastShadows, "castShadows");
	udm::write_flag(udm["renderFlags"], m_renderFlags, FRenderFlags::Unlit, "unlit");
	static_assert(magic_enum::enum_count<FRenderFlags>() == 2);
}
void BaseRenderComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm::read_flag(udm["renderFlags"], m_renderFlags, FRenderFlags::CastShadows, "castShadows");
	udm::read_flag(udm["renderFlags"], m_renderFlags, FRenderFlags::Unlit, "unlit");
	static_assert(magic_enum::enum_count<FRenderFlags>() == 2);
}
