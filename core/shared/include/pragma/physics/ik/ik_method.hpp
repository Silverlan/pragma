/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __IK_METHOD_HPP__
#define __IK_METHOD_HPP__

#include <cinttypes>

// See http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html for more information
namespace util
{
	namespace ik
	{
		enum class Method : uint32_t
		{
			SelectivelyDampedLeastSquare = 0,
			DampedLeastSquares,
			DampedLeastSquaresWithSingularValueDecomposition,
			Pseudoinverse,
			JacobianTranspose,

			Default = SelectivelyDampedLeastSquare
		};
	};
};

#endif
