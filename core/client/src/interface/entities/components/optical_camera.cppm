// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.optical_camera;

import :rendering.shaders;

export namespace pragma {
	class DLLCLIENT COpticalCameraComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

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

		COpticalCameraComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
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

		ShaderPPDoF::Flags m_flags = ShaderPPDoF::Flags::EnableVignette;
	};
};
