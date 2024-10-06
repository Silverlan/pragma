/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_LIGHT_H__
#define __ENV_LIGHT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include <mathutil/color.h>
#include <mathutil/umath_lighting.hpp>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/util_pragma.hpp>
#include <string>

namespace pragma {
	class BaseEnvLightSpotComponent;
	class BaseEnvLightPointComponent;
	class BaseEnvLightDirectionalComponent;
	class DLLNETWORK BaseEnvLightComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_CALC_LIGHT_DIRECTION_TO_POINT;
		static ComponentEventId EVENT_CALC_LIGHT_INTENSITY_AT_POINT;
		enum class SpawnFlag : uint32_t { DontCastShadows = 512 };
		enum class LightFlags : uint32_t { None = 0u, BakedLightSource = 1u };
		enum class ShadowType : uint8_t { None = 0, StaticOnly = 1, Full = StaticOnly | 2 };
		enum class LightIntensityType : uint8_t {
			Candela = 0,
			Lumen,
			Lux // Lumen per square-meter; Directional lights only
		};
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static std::string LightIntensityTypeToString(LightIntensityType type);
		static Candela GetLightIntensityCandela(float intensity, LightIntensityType type, std::optional<float> outerConeAngle = {});
		static Lumen GetLightIntensityLumen(float intensity, LightIntensityType type, std::optional<float> outerConeAngle = {});
		static float CalcDistanceFalloff(const Vector3 &lightPos, const Vector3 &point, std::optional<float> radius = {});
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		float GetFalloffExponent() const;
		virtual void SetFalloffExponent(float falloffExponent);

		bool IsBaked() const;
		virtual void SetBaked(bool baked);

		void SetLight(BaseEnvLightSpotComponent &light);
		void SetLight(BaseEnvLightPointComponent &light);
		void SetLight(BaseEnvLightDirectionalComponent &light);

		BaseEntityComponent *GetLight(pragma::LightType &outType) const;
		BaseEntityComponent *GetLight() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;

		virtual void SetLightIntensityType(LightIntensityType type);
		virtual void SetLightIntensity(float intensity, LightIntensityType type);
		LightIntensityType GetLightIntensityType() const;
		void SetLightIntensity(float intensity);
		float GetLightIntensity() const;
		Candela GetLightIntensityCandela() const;
		Lumen GetLightIntensityLumen() const;

		float CalcLightIntensityAtPoint(const Vector3 &pos) const;
		Vector3 CalcLightDirectionToPoint(const Vector3 &pos) const;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual void InitializeLight(BaseEntityComponent &component);
		ComponentHandle<BaseEntityComponent> m_hLight = {};

		pragma::LightType m_lightType = pragma::LightType::Undefined;
		ShadowType m_shadowType = ShadowType::Full;
		LightFlags m_lightFlags = LightFlags::None;
		float m_falloffExponent = 1.f;
		LightIntensityType m_lightIntensityType = LightIntensityType::Candela;
		float m_lightIntensity = 0.f;
		pragma::NetEventId m_netEvSetShadowType = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetFalloffExponent = pragma::INVALID_NET_EVENT;
	  public:
		ShadowType GetShadowType() const;
		ShadowType GetEffectiveShadowType() const;
		virtual void SetShadowType(ShadowType type);
	};
	struct DLLNETWORK CECalcLightDirectionToPoint : public ComponentEvent {
		CECalcLightDirectionToPoint(const Vector3 &pos);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &pos;
		Vector3 direction = uvec::FORWARD;
	};
	struct DLLNETWORK CECalcLightIntensityAtPoint : public ComponentEvent {
		CECalcLightIntensityAtPoint(const Vector3 &pos);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &pos;
		Candela intensity = 0.f;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseEnvLightComponent::LightFlags)

#endif
