// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_color;
import :util.rgb_csv;

using namespace pragma;

void BaseColorComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseColorComponent::EVENT_ON_COLOR_CHANGED = registerEvent("ON_COLOR_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseColorComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseColorComponent;
	{
		auto memberInfo = create_component_member_info<T, Vector3, [](const ComponentMemberInfo &, T &component, const Vector3 &value) { component.SetColor(value); }, [](const ComponentMemberInfo &, T &component, Vector3 &value) { value = component.GetColor(); }>("color",
		  colors::White.ToVector3(), AttributeSpecializationType::Color);
		memberInfo.SetInterpolationFunction<T, Vector3, [](const Vector3 &col0, const Vector3 &col1, double t, Vector3 &vOut) {
			double h0, s0, v0;
			math::rgb_to_hsv(col0, h0, s0, v0);

			double h1, s1, v1;
			math::rgb_to_hsv(col1, h1, s1, v1);

			math::lerp_hsv(h0, s0, v0, h1, s1, v1, t);

			vOut = Vector3 {math::hsv_to_rgb(h0, s0, v0)};
		}>();
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float,
		  [](const ComponentMemberInfo &, T &component, const float &value) {
			  auto col = component.GetColor();
			  col.a = value;
			  component.SetColor(col);
		  },
		  [](const ComponentMemberInfo &, T &component, float &value) { value = component.GetColor().a; }>("alpha", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
}
BaseColorComponent::BaseColorComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_color(util::SimpleProperty<util::Vector4Property, Vector4>::Create(colors::White.ToVector4())) {}
BaseColorComponent::~BaseColorComponent()
{
	if(m_cbOnColorChanged.IsValid())
		m_cbOnColorChanged.Remove();
}
void BaseColorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "color", false)) {
			Vector4 r;
			auto n = pragma::string::string_to_array<glm::vec4::value_type>(kvData.value, &r[0], pragma::string::cstring_to_number<float>, 4);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				r[i] /= static_cast<float>(std::numeric_limits<uint8_t>::max());
			if(n < 4)
				r.a = 1.f;
			*m_color = r;
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setcolor", false)) {
			Vector4 r;
			auto n = pragma::string::string_to_array<glm::vec4::value_type>(inputData.data, &r[0], pragma::string::cstring_to_number<float>, 4);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				r[i] /= static_cast<float>(std::numeric_limits<uint8_t>::max());
			if(n < 4)
				r.a = 1.f;
			*m_color = r;
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnColorChanged = m_color->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> newColor) {
		CEOnColorChanged onColorChanged {oldColor.get(), newColor.get()};
		BroadcastEvent(baseColorComponent::EVENT_ON_COLOR_CHANGED, onColorChanged);
	});

	auto &ent = GetEntity();
	//ent.AddComponent("io");
	m_netEvSetColor = SetupNetEvent("set_color");
}
void BaseColorComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["color"] = m_color->GetValue();
}
void BaseColorComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	Vector4 color;
	udm["color"](color);
	(*m_color) = color;
}
const Vector4 &BaseColorComponent::GetColor() const { return *m_color; }
const util::PVector4Property &BaseColorComponent::GetColorProperty() const { return m_color; }

void BaseColorComponent::SetColor(const Color &color) { *m_color = color.ToVector4(); }
void BaseColorComponent::SetColor(const Vector4 &color) { *m_color = color; }
void BaseColorComponent::SetColor(const Vector3 &color) { *m_color = Vector4 {color, (*m_color)->a}; }

/////////////////

CEOnColorChanged::CEOnColorChanged(const Vector4 &oldColor, const Vector4 &color) : oldColor {oldColor}, color {color} {}
void CEOnColorChanged::PushArguments(lua::State *l)
{
	Lua::Push<Vector4>(l, oldColor);
	Lua::Push<Vector4>(l, color);
}
