// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_COLOR_COMPONENT_HPP__
#define __BASE_COLOR_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>

namespace pragma {
	struct DLLNETWORK CEOnColorChanged : public ComponentEvent {
		CEOnColorChanged(const Vector4 &oldColor, const Vector4 &color);
		virtual void PushArguments(lua_State *l) override;
		const Vector4 &oldColor;
		const Vector4 &color;
	};
	class DLLNETWORK BaseColorComponent : public BaseEntityComponent {
	  public:
		static pragma::ComponentEventId EVENT_ON_COLOR_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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
		BaseColorComponent(BaseEntity &ent);
		util::PVector4Property m_color;
		CallbackHandle m_cbOnColorChanged = {};
		pragma::NetEventId m_netEvSetColor = pragma::INVALID_NET_EVENT;
	};
};

#endif
