/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_DEBUG_COMPONENT_HPP__
#define __BASE_DEBUG_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include <sharedutils/property/util_property.hpp>
#include <mathutil/uvec.h>
#include <mathutil/color.h>

namespace pragma {
	class DLLNETWORK BaseDebugTextComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
		virtual void SetText(const std::string &text);
		virtual void SetSize(float size);
	  protected:
		BaseDebugTextComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetText = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetSize = pragma::INVALID_NET_EVENT;
		std::string m_debugText = {};
		float m_size = 0.3f;
	};

	////////////////

	class DLLNETWORK BaseDebugOutlineComponent {
	  public:
		void Initialize(BaseEntityComponent &component);

		virtual void SetOutlineColor(const Color &color);
		const Color &GetOutlineColor() const;
	  protected:
		BaseDebugOutlineComponent() = default;
		pragma::NetEventId m_netEvSetOutlineColor = pragma::INVALID_NET_EVENT;
		Color m_outlineColor = {0u, 0u, 0u, 0u};
	};

	////////////////

	class DLLNETWORK BaseDebugPointComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugPointComponent(BaseEntity &ent);
		bool m_bAxis = false;
	};

	////////////////

	class DLLNETWORK BaseDebugLineComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugLineComponent(BaseEntity &ent);
		std::string m_targetEntity = {};
		Vector3 m_targetOrigin = {};
	};

	////////////////

	class DLLNETWORK BaseDebugBoxComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;
		virtual void SetBounds(const Vector3 &min, const Vector3 &max);
		const std::pair<Vector3, Vector3> &GetBounds() const;
	  protected:
		BaseDebugBoxComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetBounds = pragma::INVALID_NET_EVENT;
		std::pair<Vector3, Vector3> m_bounds = {};
	};

	////////////////

	class DLLNETWORK BaseDebugSphereComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugSphereComponent(BaseEntity &ent);
		uint32_t m_recursionLevel = 1u;
	};

	////////////////

	class DLLNETWORK BaseDebugConeComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;

		virtual void SetConeAngle(float angle);
		float GetConeAngle() const;

		virtual void SetStartRadius(float radius);
		float GetStartRadius() const;
	  protected:
		BaseDebugConeComponent(BaseEntity &ent);
		float m_coneAngle = 0.f;
		float m_startRadius = 0.f;
		uint32_t m_segmentCount = 12u;
		pragma::NetEventId m_netEvSetConeAngle = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetStartRadius = pragma::INVALID_NET_EVENT;
	};

	////////////////

	class DLLNETWORK BaseDebugCylinderComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;

		virtual void SetLength(float length);
		float GetLength() const;
	  protected:
		BaseDebugCylinderComponent(BaseEntity &ent);
		float m_length = 0.f;
		uint32_t m_segmentCount = 12u;
		pragma::NetEventId m_netEvSetLength = pragma::INVALID_NET_EVENT;
	};

	////////////////

	class DLLNETWORK BaseDebugPlaneComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugPlaneComponent(BaseEntity &ent);
	};
};

#endif
