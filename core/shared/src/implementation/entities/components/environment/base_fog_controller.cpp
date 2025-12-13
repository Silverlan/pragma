// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_fog_controller;

using namespace pragma;

void BaseEnvFogControllerComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvFogControllerComponent;

	{
		using TStart = float;
		auto memberInfo = create_component_member_info<T, TStart, static_cast<void (T::*)(TStart)>(&T::SetFogStart), static_cast<TStart (T::*)() const>(&T::GetFogStart)>("start", 500.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TEnd = float;
		auto memberInfo = create_component_member_info<T, TEnd, static_cast<void (T::*)(TEnd)>(&T::SetFogEnd), static_cast<TEnd (T::*)() const>(&T::GetFogEnd)>("end", 2'000.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TDensity = float;
		auto memberInfo = create_component_member_info<T, TDensity, static_cast<void (T::*)(TDensity)>(&T::SetMaxDensity), static_cast<TDensity (T::*)() const>(&T::GetMaxDensity)>("density", 1.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TType = util::FogType;
		auto memberInfo = create_component_member_info<T, TType, static_cast<void (T::*)(TType)>(&T::SetFogType), static_cast<TType (T::*)() const>(&T::GetFogType)>("type", util::FogType::Linear);
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvFogControllerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "fogcolor", false))
			GetEntity().SetKeyValue("color", kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fogstart", false))
			m_kvFogStart = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fogend", false))
			m_kvFogEnd = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fogmaxdensity", false))
			m_kvMaxDensity = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fogtype", false))
			m_kvFogType = static_cast<util::FogType>(util::to_int(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setstartdist", false))
			m_kvFogStart = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setenddist", false))
			m_kvFogEnd = util::to_float(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setmaxdensity", false))
			m_kvMaxDensity = util::to_float(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	ent.AddComponent("toggle");
	ent.AddComponent("color");
}

void BaseEnvFogControllerComponent::SetFogStart(float start) { m_kvFogStart = start; }
void BaseEnvFogControllerComponent::SetFogEnd(float end) { m_kvFogEnd = end; }
void BaseEnvFogControllerComponent::SetMaxDensity(float density) { m_kvMaxDensity = density; }
void BaseEnvFogControllerComponent::SetFogType(util::FogType type) { m_kvFogType = type; }
