/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_OPTICAL_CAMERA_COMPONENT_HPP__
#define __C_OPTICAL_CAMERA_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

namespace pragma
{
	class DLLCLIENT COpticalCameraComponent final
		: public BaseEntityComponent
	{
	public:
		enum class Flags : uint32_t
		{
			None = 0,
			DebugShowFocus = 1,
			EnableVignette = DebugShowFocus<<1u,
			PentagonBokehShape = EnableVignette<<1u,
			DebugShowDepth = PentagonBokehShape<<1u
		};

		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);

		void SetDebugShowFocus(bool enabled);
		bool GetDebugShowFocus() const;
		void SetVignetteEnabled(bool enabled);
		bool IsVignetteEnabled() const;
		void SetPentagonBokehShape(bool pentagonShape);
		bool GetPentagonBokehShape() const;
		void SetDebugShowDepth(bool debugShowDepth);
		bool GetDebugShowDepth() const;

		void SetFocalDistance(float dist);
		float GetFocalDistance() const;
		void SetFocalLength(float len);
		float GetFocalLength() const;
		void SetFStop(float fstop);
		float GetFStop() const;
		void SetApertureBokehRatio(float ratio);
		float GetApertureBokehRatio() const;
		void SetApertureBladeCount(uint32_t count);
		uint32_t GetApertureBladeCount() const;
		void SetApertureBladesRotation(float rotation);
		float GetApertureBladesRotation() const;
		void SetSensorSize(float sensorSize);
		float GetSensorSize() const;

		void SetRingCount(uint32_t numRings);
		uint32_t GetRingCount() const;
		void SetRingSamples(uint32_t samples);
		uint32_t GetRingSamples() const;
		void SetCircleOfConfusionSize(float coc);
		float GetCircleOfConfusionSize() const;
		void SetMaxBlur(float maxBlur);
		float GetMaxBlur() const;
		void SetDitherAmount(float dither);
		float GetDitherAmount() const;
		void SetVignettingInnerBorder(float vignettingInner);
		float GetVignettingInnerBorder() const;
		void SetVignettingOuterBorder(float vignettingOuter);
		float GetVignettingOuterBorder() const;
		void SetPentagonShapeFeather(float pentagonShapeFeather);
		float GetPentagonShapeFeather() const;

		COpticalCameraComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	private:
		float m_focalDistance = 200.f;
		float m_focalLength = 24.f;
		float m_fstop = 0.5f;
		float m_apertureBokehRatio = 1.f;
		uint32_t m_apertureBladeCount = 0;
		float m_apertureBladesRotation = 0.f;
		float m_sensorSize = 36.f;

		uint32_t m_numRings = 3;
		uint32_t m_ringSamples = 3;
		float m_coc = 0.03f;
		float m_maxBlur = 1.f;
		float m_ditherAmount = 0.0001f;
		float m_vignetteInnerBorder = 0.f;
		float m_vignetteOuterBorder = 1.3f;
		float m_pentagonShapeFeather = 0.4f;

		Flags m_flags = Flags::EnableVignette;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::COpticalCameraComponent::Flags)

#endif
