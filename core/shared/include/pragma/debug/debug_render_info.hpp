/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __DEBUG_RENDER_INFO_HPP__
#define __DEBUG_RENDER_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/transform.hpp>
#include <mathutil/color.h>
#include <optional>

struct DLLNETWORK DebugRenderInfo {
	DebugRenderInfo() = default;
	DebugRenderInfo(const umath::Transform &pose, const Color &color) : pose {pose}, color {color} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, const Color &outlineColor) : pose {pose}, color {color}, outlineColor {outlineColor} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, float duration) : pose {pose}, color {color}, duration {duration} {}
	DebugRenderInfo(const umath::Transform &pose, const Color &color, const Color &outlineColor, float duration) : pose {pose}, color {color}, outlineColor {outlineColor}, duration {duration} {}
	umath::Transform pose {};
	Color color;
	std::optional<Color> outlineColor {};
	float duration = 0.f;

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
};

#endif
