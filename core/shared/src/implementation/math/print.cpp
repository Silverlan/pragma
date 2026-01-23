// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :math.print;

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const EulerAngles &ang)
{
	os << ang.p << " " << ang.y << " " << ang.r;
	return os;
}

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Quat &q)
{
	os << q.w << " " << q.x << " " << q.y << " " << q.z;
	return os;
}

void uvec::print(Vector3 *vec) { Con::COUT << to_string(vec); }

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

#define MATRIX_COUT(numr, numc)                                                                                                                                                                                                                                                                  \
	os << "Matrix" #numr "x" #numc "(";                                                                                                                                                                                                                                                          \
	for(int i = 0; i < numr; i++) {                                                                                                                                                                                                                                                              \
		os << "(";                                                                                                                                                                                                                                                                               \
		for(int j = 0; j < numc; j++) {                                                                                                                                                                                                                                                          \
			os << m[i][j];                                                                                                                                                                                                                                                                       \
			if(j != numc - 1)                                                                                                                                                                                                                                                                    \
				os << " ";                                                                                                                                                                                                                                                                       \
		}                                                                                                                                                                                                                                                                                        \
		os << ")";                                                                                                                                                                                                                                                                               \
	}                                                                                                                                                                                                                                                                                            \
	os << ")";

#define MATRIX_OSTREAM_DEF(numr, numc)                                                                                                                                                                                                                                                           \
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Mat##numr##x##numc &m)                                                                                                                                                                                                             \
	{                                                                                                                                                                                                                                                                                            \
		MATRIX_COUT(numr, numc);                                                                                                                                                                                                                                                                 \
		return os;                                                                                                                                                                                                                                                                               \
	}                                                                                                                                                                                                                                                                                            \
	DLLNETWORK std::ostream &operator<<(std::ostream &os, const Mat##numr##x##numc &m)                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		MATRIX_COUT(numr, numc);                                                                                                                                                                                                                                                                 \
		return os;                                                                                                                                                                                                                                                                               \
	}

MATRIX_OSTREAM_DEF(2, 2);
MATRIX_OSTREAM_DEF(2, 3);
MATRIX_OSTREAM_DEF(2, 4);
MATRIX_OSTREAM_DEF(3, 2);
MATRIX_OSTREAM_DEF(3, 3);
MATRIX_OSTREAM_DEF(3, 4);
MATRIX_OSTREAM_DEF(4, 2);
MATRIX_OSTREAM_DEF(4, 3);
MATRIX_OSTREAM_DEF(4, 4);
