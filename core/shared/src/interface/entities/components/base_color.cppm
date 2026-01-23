// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_color;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	struct DLLNETWORK CEOnColorChanged : public ComponentEvent {
		CEOnColorChanged(const Vector4 &oldColor, const Vector4 &color);
		virtual void PushArguments(lua::State *l) override;
		const Vector4 &oldColor;
		const Vector4 &color;
	};
	namespace baseColorComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_COLOR_CHANGED)
	}
	class DLLNETWORK BaseColorComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		const Vector4 &GetColor() const;
		const util::PVector4Property &GetColorProperty() const;

		virtual ~BaseColorComponent() override;
		void SetColor(const Color &color);
		void SetColor(const Vector4 &color);
		void SetColor(const Vector3 &color);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		BaseColorComponent(ecs::BaseEntity &ent);
		util::PVector4Property m_color;
		CallbackHandle m_cbOnColorChanged = {};
		NetEventId m_netEvSetColor = INVALID_NET_EVENT;
	};
};
