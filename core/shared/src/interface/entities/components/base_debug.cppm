// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_debug;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseDebugTextComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
		virtual void SetText(const std::string &text);
		virtual void SetSize(float size);
	  protected:
		BaseDebugTextComponent(ecs::BaseEntity &ent);
		NetEventId m_netEvSetText = INVALID_NET_EVENT;
		NetEventId m_netEvSetSize = INVALID_NET_EVENT;
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
		NetEventId m_netEvSetOutlineColor = INVALID_NET_EVENT;
		Color m_outlineColor = {0u, 0u, 0u, 0u};
	};

	////////////////

	class DLLNETWORK BaseDebugPointComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugPointComponent(ecs::BaseEntity &ent);
		bool m_bAxis = false;
	};

	////////////////

	class DLLNETWORK BaseDebugLineComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugLineComponent(ecs::BaseEntity &ent);
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
		BaseDebugBoxComponent(ecs::BaseEntity &ent);
		NetEventId m_netEvSetBounds = INVALID_NET_EVENT;
		std::pair<Vector3, Vector3> m_bounds = {};
	};

	////////////////

	class DLLNETWORK BaseDebugSphereComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugSphereComponent(ecs::BaseEntity &ent);
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
		BaseDebugConeComponent(ecs::BaseEntity &ent);
		float m_coneAngle = 0.f;
		float m_startRadius = 0.f;
		uint32_t m_segmentCount = 12u;
		NetEventId m_netEvSetConeAngle = INVALID_NET_EVENT;
		NetEventId m_netEvSetStartRadius = INVALID_NET_EVENT;
	};

	////////////////

	class DLLNETWORK BaseDebugCylinderComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;

		virtual void SetLength(float length);
		float GetLength() const;
	  protected:
		BaseDebugCylinderComponent(ecs::BaseEntity &ent);
		float m_length = 0.f;
		uint32_t m_segmentCount = 12u;
		NetEventId m_netEvSetLength = INVALID_NET_EVENT;
	};

	////////////////

	class DLLNETWORK BaseDebugPlaneComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugPlaneComponent(ecs::BaseEntity &ent);
	};
};
