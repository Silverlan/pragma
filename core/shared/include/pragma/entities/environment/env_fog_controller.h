#ifndef __ENV_FOG_CONTROLLER_H__
#define __ENV_FOG_CONTROLLER_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/color.h>

namespace pragma
{
	class DLLNETWORK BaseEnvFogControllerComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;

		virtual void SetFogStart(float start);
		virtual void SetFogEnd(float end);
		virtual void SetMaxDensity(float density);
		virtual void SetFogType(unsigned char type);
	protected:
		float m_kvFogStart = 500.f;
		float m_kvFogEnd = 2'000.f;
		float m_kvMaxDensity = 1.f;
		unsigned char m_kvFogType = 0;
	};
};

#endif
