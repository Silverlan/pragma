// Clyde library - tools for developing networked games
// Copyright (C) 2005-2012 Three Rings Design, Inc.
// http://code.google.com/p/clyde/
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
module;

#include "definitions.hpp"

export module pragma.shared:math.noise;

export import std.compat;

// Source: https://github.com/threerings/clyde/blob/207d235ae1f7ba03ddcc6406febb6b7f306593bd/core/src/main/java/com/threerings/util/NoiseUtil.java
export namespace pragma::math {
	namespace noise {
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
		DLLNETWORK float get_noise(float x, int y);

		// Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x, float y);

		//Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x, float y, int z);

		// Returns the noise value at the specified coordinates.
		DLLNETWORK float get_noise(float x, float y, float z);

		// Returns the dot product of the provided value with the hashed gradient.
		DLLNETWORK float grad(int hash, float x);

		// Returns the dot product of the provided values with the hashed gradient.
		DLLNETWORK float grad(int hash, float x, float y);

		// Returns the dot product of the provided values with the hashed gradient.
		DLLNETWORK float grad(int hash, float x, float y, float z);

		// Computes the ease parameter for the given linear value.
		DLLNETWORK float ease(float t);
	};
};
