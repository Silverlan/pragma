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
		BaseDebugTextComponent(pragma::ecs::BaseEntity &ent);
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
		BaseDebugPointComponent(pragma::ecs::BaseEntity &ent);
		bool m_bAxis = false;
	};

	////////////////

	class DLLNETWORK BaseDebugLineComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugLineComponent(pragma::ecs::BaseEntity &ent);
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
		BaseDebugBoxComponent(pragma::ecs::BaseEntity &ent);
		pragma::NetEventId m_netEvSetBounds = pragma::INVALID_NET_EVENT;
		std::pair<Vector3, Vector3> m_bounds = {};
	};

	////////////////

	class DLLNETWORK BaseDebugSphereComponent : public BaseEntityComponent, public BaseDebugOutlineComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugSphereComponent(pragma::ecs::BaseEntity &ent);
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
		BaseDebugConeComponent(pragma::ecs::BaseEntity &ent);
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
		BaseDebugCylinderComponent(pragma::ecs::BaseEntity &ent);
		float m_length = 0.f;
		uint32_t m_segmentCount = 12u;
		pragma::NetEventId m_netEvSetLength = pragma::INVALID_NET_EVENT;
	};

	////////////////

	class DLLNETWORK BaseDebugPlaneComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
	  protected:
		BaseDebugPlaneComponent(pragma::ecs::BaseEntity &ent);
	};
};
