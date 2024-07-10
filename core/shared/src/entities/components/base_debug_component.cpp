/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_debug_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

BaseDebugTextComponent::BaseDebugTextComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseDebugTextComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetText = SetupNetEvent("set_text");
	m_netEvSetSize = SetupNetEvent("set_size");
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "text", false))
			m_debugText = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "size", false))
			m_size = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ioData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(ioData.input, "settext", false))
			SetText(ioData.data);
		else if(ustring::compare<std::string>(ioData.input, "setsize", false))
			SetSize(util::to_float(ioData.data));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_netEvSetText = SetupNetEvent("set_text");
}
void BaseDebugTextComponent::SetText(const std::string &text) { m_debugText = text; }
void BaseDebugTextComponent::SetSize(float size) { m_size = size; }

////////////////

void BaseDebugOutlineComponent::Initialize(BaseEntityComponent &component)
{
	m_netEvSetOutlineColor = component.SetupNetEvent("set_outline_color");
	component.BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "outline_color", false))
			m_outlineColor = Color {kvData.value};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	component.BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ioData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(ioData.input, "SetOutlineColor", false))
			m_outlineColor = Color {ioData.data};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseDebugOutlineComponent::SetOutlineColor(const Color &color) { m_outlineColor = color; }
const Color &BaseDebugOutlineComponent::GetOutlineColor() const { return m_outlineColor; }

////////////////

void BaseDebugPointComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
}
BaseDebugPointComponent::BaseDebugPointComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugLineComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "end_target", false))
			m_targetEntity = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "target_origin", false))
			m_targetOrigin = uvec::create(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
BaseDebugLineComponent::BaseDebugLineComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugBoxComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BaseDebugOutlineComponent::Initialize(*this);
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "min_bounds", false))
			m_bounds.first = uvec::create(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "max_bounds", false))
			m_bounds.second = uvec::create(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseDebugBoxComponent::SetBounds(const Vector3 &min, const Vector3 &max) { m_bounds = {min, max}; }
const std::pair<Vector3, Vector3> &BaseDebugBoxComponent::GetBounds() const { return m_bounds; }

BaseDebugBoxComponent::BaseDebugBoxComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugSphereComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BaseDebugOutlineComponent::Initialize(*this);
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("radius");
	ent.AddComponent("color");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "recursion_level", false))
			m_recursionLevel = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
BaseDebugSphereComponent::BaseDebugSphereComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugConeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BaseDebugOutlineComponent::Initialize(*this);
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("radius");
	ent.AddComponent("color");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "start_radius", false))
			m_startRadius = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "cone_angle", false))
			m_coneAngle = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "segment_count", false))
			m_segmentCount = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BaseDebugConeComponent::SetConeAngle(float angle) { m_coneAngle = angle; }
float BaseDebugConeComponent::GetConeAngle() const { return m_coneAngle; }

void BaseDebugConeComponent::SetStartRadius(float radius) { m_startRadius = radius; }
float BaseDebugConeComponent::GetStartRadius() const { return m_startRadius; }

BaseDebugConeComponent::BaseDebugConeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugCylinderComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BaseDebugOutlineComponent::Initialize(*this);
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
	ent.AddComponent("radius");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "length", false))
			m_length = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "segment_count", false))
			m_segmentCount = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BaseDebugCylinderComponent::SetLength(float length) { m_length = length; }
float BaseDebugCylinderComponent::GetLength() const { return m_length; }

BaseDebugCylinderComponent::BaseDebugCylinderComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

////////////////

void BaseDebugPlaneComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("attachment");
	ent.AddComponent("color");
}
BaseDebugPlaneComponent::BaseDebugPlaneComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
