// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.lights.base_spot;

export import :entities.components.base;
import :entities.components.base_field_angle;

export namespace pragma {
	class DLLNETWORK BaseEnvLightSpotComponent : public BaseEntityComponent {
	  public:
		static float CalcInnerConeAngle(float outerConeAngle, float blendFraction);
		static float CalcBlendFraction(float outerConeAngle, float innerConeAngle);
		static float CalcConeFalloff(const Vector3 &lightPos, const Vector3 &lightDir, pragma::math::Degree outerConeAngle, pragma::math::Degree innerConeAngle, const Vector3 &point);
		static Candela CalcIntensityFalloff(const Vector3 &lightPos, const Vector3 &lightDir, pragma::math::Degree outerConeAngle, pragma::math::Degree innerConeAngle, const Vector3 &point, std::optional<float> radius = {});
		static Candela CalcIntensityAtPoint(const Vector3 &lightPos, Candela intensity, const Vector3 &lightDir, pragma::math::Degree outerConeAngle, pragma::math::Degree innerConeAngle, const Vector3 &point, std::optional<float> radius = {});

		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		BaseEnvLightSpotComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void SetOuterConeAngle(pragma::math::Degree ang);
		pragma::math::Degree GetOuterConeAngle() const;
		void SetInnerConeAngle(pragma::math::Degree ang);
		pragma::math::Degree GetInnerConeAngle() const;

		pragma::math::Fraction GetBlendFraction() const;
		virtual void SetBlendFraction(pragma::math::Fraction fraction);

		virtual void SetConeStartOffset(float offset);
		float GetConeStartOffset() const;

		const pragma::util::PFloatProperty &GetBlendFractionProperty() const;
		const pragma::util::PFloatProperty &GetOuterConeAngleProperty() const;
		const pragma::util::PFloatProperty &GetConeStartOffsetProperty() const;

		float CalcConeFalloff(const Vector3 &point) const;
		float CalcDistanceFalloff(const Vector3 &point) const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual pragma::util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c);
		pragma::util::PFloatProperty m_blendFraction = nullptr;
		pragma::util::PFloatProperty m_coneStartOffset = nullptr;
		pragma::NetEventId m_netEvSetConeStartOffset = pragma::INVALID_NET_EVENT;
		ComponentHandle<BaseFieldAngleComponent> m_fieldAngleComponent;
	};
};
