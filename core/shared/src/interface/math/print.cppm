// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.print;

export import :console.output;

#define MATRIX_OSTREAM_DEC(type)                                                                                                                                                                                                                                                                 \
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Mat##type &m);                                                                                                                                                                                                                     \
	DLLNETWORK std::ostream &operator<<(std::ostream &os, const Mat##type &m);

export {
	MATRIX_OSTREAM_DEC(2x2);
	MATRIX_OSTREAM_DEC(2x3);
	MATRIX_OSTREAM_DEC(2x4);
	MATRIX_OSTREAM_DEC(3x2);
	MATRIX_OSTREAM_DEC(3x3);
	MATRIX_OSTREAM_DEC(3x4);
	MATRIX_OSTREAM_DEC(4x2);
	MATRIX_OSTREAM_DEC(4x3);
	MATRIX_OSTREAM_DEC(4x4);
	inline DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Color &col)
	{
		os << col.r << " " << col.g << " " << col.b << " " << col.a;
		return os;
	}

	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const EulerAngles &ang);
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Quat &q);

	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector4 &vec);
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector3 &vec);
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Vector2 &vec);

	namespace uvec {
		DLLNETWORK void print(Vector3 *vec);
	};
};
