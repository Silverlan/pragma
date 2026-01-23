// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.optical_camera;
import :engine;

using namespace pragma;

void COpticalCameraComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = COpticalCameraComponent;

	{
		using TFocalDistance = float;
		auto memberInfo = create_component_member_info<T, TFocalDistance, static_cast<void (T::*)(TFocalDistance)>(&T::SetFocalDistance), static_cast<TFocalDistance (T::*)() const>(&T::GetFocalDistance)>("focalDistance", 200.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TFocalLength = float;
		auto memberInfo = create_component_member_info<T, TFocalLength, static_cast<void (T::*)(TFocalLength)>(&T::SetFocalLength), static_cast<TFocalLength (T::*)() const>(&T::GetFocalLength)>("focalLength", 24.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TFStop = float;
		auto memberInfo = create_component_member_info<T, TFStop, static_cast<void (T::*)(TFStop)>(&T::SetFStop), static_cast<TFStop (T::*)() const>(&T::GetFStop)>("fstop", 0.5f);
		registerMember(std::move(memberInfo));
	}

	{
		using TApertureBokehRatio = float;
		auto memberInfo = create_component_member_info<T, TApertureBokehRatio, static_cast<void (T::*)(TApertureBokehRatio)>(&T::SetApertureBokehRatio), static_cast<TApertureBokehRatio (T::*)() const>(&T::GetApertureBokehRatio)>("apertureBokehRatio", 1.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TApertureBladeCount = uint32_t;
		auto memberInfo = create_component_member_info<T, TApertureBladeCount, static_cast<void (T::*)(TApertureBladeCount)>(&T::SetApertureBladeCount), static_cast<TApertureBladeCount (T::*)() const>(&T::GetApertureBladeCount)>("apertureBladeCount", 0);
		registerMember(std::move(memberInfo));
	}

	{
		using TApertureBladesRotation = float;
		auto memberInfo = create_component_member_info<T, TApertureBladesRotation, static_cast<void (T::*)(TApertureBladesRotation)>(&T::SetApertureBladesRotation), static_cast<TApertureBladesRotation (T::*)() const>(&T::GetApertureBladesRotation)>("apertureBladesRotation", 0.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TSensorSize = float;
		auto memberInfo = create_component_member_info<T, TSensorSize, static_cast<void (T::*)(TSensorSize)>(&T::SetSensorSize), static_cast<TSensorSize (T::*)() const>(&T::GetSensorSize)>("sensorSize", 36.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TRingCount = uint32_t;
		auto memberInfo = create_component_member_info<T, TRingCount, static_cast<void (T::*)(TRingCount)>(&T::SetRingCount), static_cast<TRingCount (T::*)() const>(&T::GetRingCount)>("ringCount", 3);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(10.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TRingSamples = uint32_t;
		auto memberInfo = create_component_member_info<T, TRingSamples, static_cast<void (T::*)(TRingSamples)>(&T::SetRingSamples), static_cast<TRingSamples (T::*)() const>(&T::GetRingSamples)>("ringSamples", 3);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(10.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TCoc = float;
		auto memberInfo = create_component_member_info<T, TCoc, static_cast<void (T::*)(TCoc)>(&T::SetCircleOfConfusionSize), static_cast<TCoc (T::*)() const>(&T::GetCircleOfConfusionSize)>("circleOfConfusion", 0.03f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(0.1f);
		registerMember(std::move(memberInfo));
	}

	{
		using TMaxBlur = float;
		auto memberInfo = create_component_member_info<T, TMaxBlur, static_cast<void (T::*)(TMaxBlur)>(&T::SetMaxBlur), static_cast<TMaxBlur (T::*)() const>(&T::GetMaxBlur)>("maxBlur", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(10.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TDitherAmount = float;
		auto memberInfo = create_component_member_info<T, TDitherAmount, static_cast<void (T::*)(TDitherAmount)>(&T::SetDitherAmount), static_cast<TDitherAmount (T::*)() const>(&T::GetDitherAmount)>("ditherAmount", 0.0001f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(0.01f);
		registerMember(std::move(memberInfo));
	}

	{
		using TVignetteBorder = float;
		auto memberInfoIn = create_component_member_info<T, TVignetteBorder, static_cast<void (T::*)(TVignetteBorder)>(&T::SetVignettingInnerBorder), static_cast<TVignetteBorder (T::*)() const>(&T::GetVignettingInnerBorder)>("vignetteInnerBorder", 0.f);
		memberInfoIn.SetMin(0.f);
		memberInfoIn.SetMax(5.f);
		registerMember(std::move(memberInfoIn));

		auto memberInfoOut = create_component_member_info<T, TVignetteBorder, static_cast<void (T::*)(TVignetteBorder)>(&T::SetVignettingOuterBorder), static_cast<TVignetteBorder (T::*)() const>(&T::GetVignettingOuterBorder)>("vignetteOuterBorder", 1.3f);
		memberInfoOut.SetMin(0.f);
		memberInfoOut.SetMax(5.f);
		registerMember(std::move(memberInfoOut));
	}

	{
		using TPentagonShapeFeather = float;
		auto memberInfo = create_component_member_info<T, TPentagonShapeFeather, static_cast<void (T::*)(TPentagonShapeFeather)>(&T::SetPentagonShapeFeather), static_cast<TPentagonShapeFeather (T::*)() const>(&T::GetPentagonShapeFeather)>("pentagonShapeFeather", 0.4f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(2.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TVignetteEnabled = bool;
		auto memberInfo = create_component_member_info<T, TVignetteEnabled, static_cast<void (T::*)(TVignetteEnabled)>(&T::SetVignetteEnabled), static_cast<TVignetteEnabled (T::*)() const>(&T::IsVignetteEnabled)>("enableVignette", true);
		registerMember(std::move(memberInfo));
	}

	{
		using TPentagonBokehShape = bool;
		auto memberInfo = create_component_member_info<T, TPentagonBokehShape, static_cast<void (T::*)(TPentagonBokehShape)>(&T::SetPentagonBokehShape), static_cast<TPentagonBokehShape (T::*)() const>(&T::GetPentagonBokehShape)>("usePentagonBokehShape", false);
		registerMember(std::move(memberInfo));
	}

	{
		using TShowDebugDepth = bool;
		auto memberInfo = create_component_member_info<T, TShowDebugDepth, static_cast<void (T::*)(TShowDebugDepth)>(&T::SetDebugShowDepth), static_cast<TShowDebugDepth (T::*)() const>(&T::GetDebugShowDepth)>("showDebugDepth", false);
		registerMember(std::move(memberInfo));
	}

	{
		using TShowDebugFocus = bool;
		auto memberInfo = create_component_member_info<T, TShowDebugFocus, static_cast<void (T::*)(TShowDebugFocus)>(&T::SetDebugShowFocus), static_cast<TShowDebugFocus (T::*)() const>(&T::GetDebugShowFocus)>("showDebugFocus", false);
		registerMember(std::move(memberInfo));
	}
}

void COpticalCameraComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void COpticalCameraComponent::Initialize() { BaseEntityComponent::Initialize(); }

void COpticalCameraComponent::SetDebugShowFocus(bool enabled) { math::set_flag(m_flags, ShaderPPDoF::Flags::DebugShowFocus, enabled); }
bool COpticalCameraComponent::GetDebugShowFocus() const { return math::is_flag_set(m_flags, ShaderPPDoF::Flags::DebugShowFocus); }
void COpticalCameraComponent::SetVignetteEnabled(bool enabled) { math::set_flag(m_flags, ShaderPPDoF::Flags::EnableVignette, enabled); }
bool COpticalCameraComponent::IsVignetteEnabled() const { return math::is_flag_set(m_flags, ShaderPPDoF::Flags::EnableVignette); }
void COpticalCameraComponent::SetPentagonBokehShape(bool pentagonShape) { math::set_flag(m_flags, ShaderPPDoF::Flags::PentagonBokehShape, pentagonShape); }
bool COpticalCameraComponent::GetPentagonBokehShape() const { return math::is_flag_set(m_flags, ShaderPPDoF::Flags::PentagonBokehShape); }
void COpticalCameraComponent::SetDebugShowDepth(bool debugShowDepth) { math::set_flag(m_flags, ShaderPPDoF::Flags::DebugShowDepth, debugShowDepth); }
bool COpticalCameraComponent::GetDebugShowDepth() const { return math::is_flag_set(m_flags, ShaderPPDoF::Flags::DebugShowDepth); }

void COpticalCameraComponent::SetFocalDistance(float dist) { m_focalDistance = dist; }
float COpticalCameraComponent::GetFocalDistance() const { return m_focalDistance; }
void COpticalCameraComponent::SetFocalLength(float len) { m_focalLength = len; }
float COpticalCameraComponent::GetFocalLength() const { return m_focalLength; }
void COpticalCameraComponent::SetFStop(float fstop) { m_fstop = fstop; }
float COpticalCameraComponent::GetFStop() const { return m_fstop; }
void COpticalCameraComponent::SetApertureBokehRatio(float ratio) { m_apertureBokehRatio = ratio; }
float COpticalCameraComponent::GetApertureBokehRatio() const { return m_apertureBokehRatio; }
void COpticalCameraComponent::SetApertureBladeCount(uint32_t count) { m_apertureBladeCount = count; }
uint32_t COpticalCameraComponent::GetApertureBladeCount() const { return m_apertureBladeCount; }
void COpticalCameraComponent::SetApertureBladesRotation(float rotation) { m_apertureBladesRotation = rotation; }
float COpticalCameraComponent::GetApertureBladesRotation() const { return m_apertureBladesRotation; }
void COpticalCameraComponent::SetSensorSize(float sensorSize) { m_sensorSize = sensorSize; }
float COpticalCameraComponent::GetSensorSize() const { return m_sensorSize; }

void COpticalCameraComponent::SetRingCount(uint32_t numRings) { m_numRings = numRings; }
uint32_t COpticalCameraComponent::GetRingCount() const { return m_numRings; }
void COpticalCameraComponent::SetRingSamples(uint32_t samples) { m_ringSamples = samples; }
uint32_t COpticalCameraComponent::GetRingSamples() const { return m_ringSamples; }
void COpticalCameraComponent::SetCircleOfConfusionSize(float coc) { m_coc = coc; }
float COpticalCameraComponent::GetCircleOfConfusionSize() const { return m_coc; }
void COpticalCameraComponent::SetMaxBlur(float maxBlur) { m_maxBlur = maxBlur; }
float COpticalCameraComponent::GetMaxBlur() const { return m_maxBlur; }
void COpticalCameraComponent::SetDitherAmount(float dither) { m_ditherAmount = dither; }
float COpticalCameraComponent::GetDitherAmount() const { return m_ditherAmount; }
void COpticalCameraComponent::SetVignettingInnerBorder(float vignettingInner) { m_vignetteInnerBorder = vignettingInner; }
float COpticalCameraComponent::GetVignettingInnerBorder() const { return m_vignetteInnerBorder; }
void COpticalCameraComponent::SetVignettingOuterBorder(float vignettingOuter) { m_vignetteOuterBorder = vignettingOuter; }
float COpticalCameraComponent::GetVignettingOuterBorder() const { return m_vignetteOuterBorder; }
void COpticalCameraComponent::SetPentagonShapeFeather(float pentagonShapeFeather) { m_pentagonShapeFeather = pentagonShapeFeather; }
float COpticalCameraComponent::GetPentagonShapeFeather() const { return m_pentagonShapeFeather; }
