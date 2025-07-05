// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <mathutil/color.h>
#include "pragma/physics/visual_debugger.hpp"

//////////////

void pragma::physics::IVisualDebugger::DrawLine(const Vector3 &from, const Vector3 &to, const Color &color) { DrawLine(from, to, color, color); }

void pragma::physics::IVisualDebugger::SetDebugMode(DebugMode debugMode) {}
