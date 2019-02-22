#ifndef __UTIL_NOISE_HPP__
#define __UTIL_NOISE_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <array>

// Source: https://github.com/threerings/clyde/blob/207d235ae1f7ba03ddcc6406febb6b7f306593bd/core/src/main/java/com/threerings/util/NoiseUtil.java
namespace util
{
	namespace noise
	{
		// The number of permutation bits.
		const int32_t PERMUTATION_BITS = 8;

		// he number of permutations stored.
		const int32_t PERMUTATION_COUNT = 1 << PERMUTATION_BITS;

		// Mask for permutation table entries.
		const int32_t PERMUTATION_MASK = PERMUTATION_COUNT - 1;

		// Mask for gradient values.
		const int32_t GRADIENT_MASK = 15;

		// Returns the noise value at the specified coordinate.
		DLLNETWORK float get_noise(float x);

		//Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x,int y);

		// Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x,float y);

		//Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x,float y,int z);

		// Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x,float y,float z);

		// Returns the dot product of the provided value with the hashed gradient.
		DLLNETWORK float grad(int hash,float x);
    
		// Returns the dot product of the provided values with the hashed gradient.
		DLLNETWORK float grad(int hash,float x,float y);

		// Returns the dot product of the provided values with the hashed gradient.
		DLLNETWORK float grad(int hash,float x,float y,float z);

		// Computes the ease parameter for the given linear value.
		DLLNETWORK float ease (float t);
	};
};

#endif
