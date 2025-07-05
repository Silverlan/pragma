// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WVVECTOR3_H__
#define __WVVECTOR3_H__
#include "pragma/definitions.h"
#include <mathutil/uvec.h>
#include "pragma/console/conout.h"

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector4 &vec);
DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector3 &vec);
DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector2 &vec);

namespace uvec {
	DLLNETWORK void print(Vector3 *vec);
};

#endif // __VECTOR3_H__
