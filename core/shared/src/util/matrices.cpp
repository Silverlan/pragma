// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/util/matrices.h"

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
