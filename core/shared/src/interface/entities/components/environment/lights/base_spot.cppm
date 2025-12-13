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
		static float CalcConeFalloff(const Vector3 &lightPos, const Vector3 &lightDir, math::Degree outerConeAngle, math::Degree innerConeAngle, const Vector3 &point);
		static Candela CalcIntensityFalloff(const Vector3 &lightPos, const Vector3 &lightDir, math::Degree outerConeAngle, math::Degree innerConeAngle, const Vector3 &point, std::optional<float> radius = {});
		static Candela CalcIntensityAtPoint(const Vector3 &lightPos, Candela intensity, const Vector3 &lightDir, math::Degree outerConeAngle, math::Degree innerConeAngle, const Vector3 &point, std::optional<float> radius = {});

		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		BaseEnvLightSpotComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void SetOuterConeAngle(math::Degree ang);
		math::Degree GetOuterConeAngle() const;
		void SetInnerConeAngle(math::Degree ang);
		math::Degree GetInnerConeAngle() const;

		math::Fraction GetBlendFraction() const;
		virtual void SetBlendFraction(math::Fraction fraction);

		virtual void SetConeStartOffset(float offset);
		float GetConeStartOffset() const;

		const util::PFloatProperty &GetBlendFractionProperty() const;
		const util::PFloatProperty &GetOuterConeAngleProperty() const;
		const util::PFloatProperty &GetConeStartOffsetProperty() const;

		float CalcConeFalloff(const Vector3 &point) const;
		float CalcDistanceFalloff(const Vector3 &point) const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c);
		util::PFloatProperty m_blendFraction = nullptr;
		util::PFloatProperty m_coneStartOffset = nullptr;
		NetEventId m_netEvSetConeStartOffset = INVALID_NET_EVENT;
		ComponentHandle<BaseFieldAngleComponent> m_fieldAngleComponent;
	};
};
