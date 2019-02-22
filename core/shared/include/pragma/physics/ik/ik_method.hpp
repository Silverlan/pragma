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
