/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include <mathutil/color.h>
#include "pragma/physics/visual_debugger.hpp"

//////////////

void pragma::physics::IVisualDebugger::DrawLine(const Vector3 &from,const Vector3 &to,const Color &color)
{
	DrawLine(from,to,color,color);
}

void pragma::physics::IVisualDebugger::SetDebugMode(DebugMode debugMode) {}
