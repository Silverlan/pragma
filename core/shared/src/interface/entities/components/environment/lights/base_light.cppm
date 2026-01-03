// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.environment.lights.base_light;

export import :entities.components.base;

export {
	namespace pragma {
		class BaseEnvLightSpotComponent;
		class BaseEnvLightPointComponent;
		class BaseEnvLightDirectionalComponent;
		namespace baseEnvLightComponent {
			REGISTER_COMPONENT_EVENT(EVENT_CALC_LIGHT_DIRECTION_TO_POINT)
			REGISTER_COMPONENT_EVENT(EVENT_CALC_LIGHT_INTENSITY_AT_POINT)
		}
		class DLLNETWORK BaseEnvLightComponent : public BaseEntityComponent {
		  public:
			enum class SpawnFlag : uint32_t { DontCastShadows = 512 };
			enum class LightFlags : uint32_t { None = 0u, BakedLightSource = 1u };
			enum class ShadowType : uint8_t { None = 0, StaticOnly = 1, Full = StaticOnly | 2 };
			enum class LightIntensityType : uint8_t {
				Candela = 0,
				Lumen,
				Lux // Lumen per square-meter; Directional lights only
			};
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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

			BaseEntityComponent *GetLight(LightType &outType) const;
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

			LightType m_lightType = LightType::Undefined;
			ShadowType m_shadowType = ShadowType::Full;
			LightFlags m_lightFlags = LightFlags::None;
			float m_falloffExponent = 1.f;
			LightIntensityType m_lightIntensityType = LightIntensityType::Candela;
			float m_lightIntensity = 0.f;
			NetEventId m_netEvSetShadowType = INVALID_NET_EVENT;
			NetEventId m_netEvSetFalloffExponent = INVALID_NET_EVENT;
		  public:
			ShadowType GetShadowType() const;
			ShadowType GetEffectiveShadowType() const;
			virtual void SetShadowType(ShadowType type);
		};
		struct DLLNETWORK CECalcLightDirectionToPoint : public ComponentEvent {
			CECalcLightDirectionToPoint(const Vector3 &pos);
			virtual void PushArguments(lua::State *l) override;
			const Vector3 &pos;
			Vector3 direction = uvec::PRM_FORWARD;
		};
		struct DLLNETWORK CECalcLightIntensityAtPoint : public ComponentEvent {
			CECalcLightIntensityAtPoint(const Vector3 &pos);
			virtual void PushArguments(lua::State *l) override;
			const Vector3 &pos;
			Candela intensity = 0.f;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEnvLightComponent::LightFlags)
};
