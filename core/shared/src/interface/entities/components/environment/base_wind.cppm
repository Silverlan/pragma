// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.base_wind;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvWindComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		void SetWindForce(const Vector3 &force);
		void SetWindDirection(const Vector3 &dir);
		void SetWindSpeed(float speed);
		const Vector3 &GetWindForce() const;
		Vector3 GetWindDirection() const;
		float GetWindSpeed() const;
	  protected:
		Vector3 m_windForce;
	};
};
