#ifndef __ENV_LIGHT_DIRECTIONAL_H__
#define __ENV_LIGHT_DIRECTIONAL_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <sharedutils/property/util_property_color.hpp>
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvLightDirectionalComponent
		: public BaseEntityComponent
	{
	public:
		BaseEnvLightDirectionalComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void SetAmbientColor(const Color &color);
		const Color &GetAmbientColor() const;
		const util::PColorProperty &GetAmbientColorProperty() const;
	protected:
		util::PColorProperty m_ambientColor = nullptr;
		Float m_maxExposure = 8.f;
		pragma::NetEventId m_netEvSetAmbientColor = pragma::INVALID_NET_EVENT;
	};
};

#endif
