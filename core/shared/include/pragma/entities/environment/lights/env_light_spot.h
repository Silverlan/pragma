/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_LIGHT_SPOT_H__
#define __ENV_LIGHT_SPOT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

namespace pragma
{
	class BaseFieldAngleComponent;
	class DLLNETWORK BaseEnvLightSpotComponent
		: public BaseEntityComponent
	{
	public:
		static float CalcInnerConeAngle(float outerConeAngle,float blendFraction);
		static float CalcBlendFraction(float outerConeAngle,float innerConeAngle);
		static float CalcConeFalloff(
			const Vector3 &lightPos,const Vector3 &lightDir,umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point
		);
		static Candela CalcIntensityFalloff(
			const Vector3 &lightPos,const Vector3 &lightDir,
			umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point,
			std::optional<float> radius={}
		);
		static Candela CalcIntensityAtPoint(
			const Vector3 &lightPos,Candela intensity,const Vector3 &lightDir,
			umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point,
			std::optional<float> radius={}
		);

		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		BaseEnvLightSpotComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void SetOuterConeAngle(umath::Degree ang);
		umath::Degree GetOuterConeAngle() const;
		void SetInnerConeAngle(umath::Degree ang);
		umath::Degree GetInnerConeAngle() const;

		umath::Fraction GetBlendFraction() const;
		virtual void SetBlendFraction(umath::Fraction fraction);

		virtual void SetConeStartOffset(float offset);
		float GetConeStartOffset() const;

		const util::PFloatProperty &GetBlendFractionProperty() const;
		const util::PFloatProperty &GetOuterConeAngleProperty() const;
		const util::PFloatProperty &GetConeStartOffsetProperty() const;
		
		float CalcConeFalloff(const Vector3 &point) const;
		float CalcDistanceFalloff(const Vector3 &point) const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c);
		util::PFloatProperty m_blendFraction = nullptr;
		util::PFloatProperty m_coneStartOffset = nullptr;
		pragma::NetEventId m_netEvSetConeStartOffset = pragma::INVALID_NET_EVENT;
		ComponentHandle<BaseFieldAngleComponent> m_fieldAngleComponent;
	};
};

#endif