// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:physics.ik.method;

export import std.compat;

// See http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html for more information
export namespace pragma::physics {
	namespace ik {
		enum class Method : uint32_t {
			SelectivelyDampedLeastSquare = 0,
			DampedLeastSquares,
			DampedLeastSquaresWithSingularValueDecomposition,
			Pseudoinverse,
			JacobianTranspose,

			Default = SelectivelyDampedLeastSquare
		};
	};
};
