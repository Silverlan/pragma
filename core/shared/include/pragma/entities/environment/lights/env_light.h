#ifndef __ENV_LIGHT_H__
#define __ENV_LIGHT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include <mathutil/color.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity_handle.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvLightComponent
		: public BaseEntityComponent
	{
	public:
		enum class SpawnFlag : uint32_t
		{
			DontCastShadows = 512
		};
		enum class LightFlags : uint32_t
		{
			None = 0u,
			BakedLightSource = 1u
		};
		enum class ShadowType : uint8_t
		{
			None = 0,
			StaticOnly = 1,
			Full = StaticOnly | 2
		};
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		float GetFalloffExponent() const;
		virtual void SetFalloffExponent(float falloffExponent);
	protected:
		ShadowType m_shadowType = ShadowType::Full;
		LightFlags m_lightFlags = LightFlags::None;
		float m_falloffExponent = 1.f;
		pragma::NetEventId m_netEvSetShadowType = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetFalloffExponent = pragma::INVALID_NET_EVENT;
	public:
		ShadowType GetShadowType() const;
		virtual void SetShadowType(ShadowType type);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseEnvLightComponent::LightFlags)

#endif
