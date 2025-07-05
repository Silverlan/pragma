// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __MATRICES_H__
#define __MATRICES_H__
#include "pragma/definitions.h"
#include <iostream>
#include <mathutil/glmutil.h>
#include "pragma/console/conout.h"

#define MATRIX_OSTREAM_DEC(type)                                                                                                                                                                                                                                                                 \
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Mat##type &m);                                                                                                                                                                                                                     \
	DLLNETWORK std::ostream &operator<<(std::ostream &os, const Mat##type &m);

MATRIX_OSTREAM_DEC(2x2);
MATRIX_OSTREAM_DEC(2x3);
MATRIX_OSTREAM_DEC(2x4);
MATRIX_OSTREAM_DEC(3x2);
MATRIX_OSTREAM_DEC(3x3);
MATRIX_OSTREAM_DEC(3x4);
MATRIX_OSTREAM_DEC(4x2);
MATRIX_OSTREAM_DEC(4x3);
MATRIX_OSTREAM_DEC(4x4);

#endif
