#ifndef __ENV_WIND_HPP__
#define __ENV_WIND_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseEnvWindComponent
		: public BaseEntityComponent
	{
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

#endif
