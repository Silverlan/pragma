// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DEBUG_RENDER_INFO_HPP__
#define __DEBUG_RENDER_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/transform.hpp>
#include <mathutil/color.h>
#include <optional>

struct DLLNETWORK DebugRenderInfo {
	DebugRenderInfo() = default;
	DebugRenderInfo(const Color &color, float duration = 0.f) : color {color}, duration {duration} {}
	DebugRenderInfo(const Color &color, const Color &colorOutline, float duration = 0.f) : color {color}, outlineColor {colorOutline}, duration {duration} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color) : pose {pose}, color {color} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, const Color &outlineColor) : pose {pose}, color {color}, outlineColor {outlineColor} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, float duration) : pose {pose}, color {color}, duration {duration} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, const Color &outlineColor, float duration) : pose {pose}, color {color}, outlineColor {outlineColor}, duration {duration} {}
	umath::Transform pose {};
	Color color;
	std::optional<Color> outlineColor {};
	float duration = 0.f;
	bool ignoreDepthBuffer = false;

	DebugRenderInfo &SetOrigin(const Vector3 &origin)
	{
		pose.SetOrigin(origin);
		return *this;
	}
	DebugRenderInfo &SetRotation(const Quat &rot)
	{
		pose.SetRotation(rot);
		return *this;
	}
	DebugRenderInfo &SetColor(const Color &color)
	{
		this->color = color;
		return *this;
	}
	DebugRenderInfo &SetOutlineColor(const Color &color)
	{
		this->outlineColor = color;
		return *this;
	}
	DebugRenderInfo &SetDuration(float duration)
	{
		this->duration = duration;
		return *this;
	}
	DebugRenderInfo &SetIgnoreDepthBuffer(bool ignoreDepthBuffer)
	{
		this->ignoreDepthBuffer = ignoreDepthBuffer;
		return *this;
	}
};

#endif
