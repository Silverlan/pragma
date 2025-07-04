// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/vector/wvvector3.h"
#include "pragma/console/conout.h"

void uvec::print(Vector3 *vec) { Con::cout << uvec::to_string(vec); }

Con::c_cout &operator<<(Con::c_cout &os, const Vector4 &vec)
{
	os << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
	return os;
}

Con::c_cout &operator<<(Con::c_cout &os, const Vector3 &vec)
{
	os << vec.x << " " << vec.y << " " << vec.z;
	return os;
}

Con::c_cout &operator<<(Con::c_cout &os, const Vector2 &vec)
{
	os << vec.x << " " << vec.y;
	return os;
}
