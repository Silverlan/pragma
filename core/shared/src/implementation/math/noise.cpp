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

module pragma.shared;

import :math.noise;

namespace pragma::math {
	namespace noise {
		static void init();

		// Permutation table (doubled so that we can offset values without having to remask).
		static std::array<int32_t, PERMUTATION_COUNT * 2> PERMUTATIONS;
	};
};
float pragma::math::noise::get_noise(float x)
{
	init();
	int x0 = floor(x);
	x -= x0;
	x0 &= PERMUTATION_MASK;
	return lerp(grad(PERMUTATIONS[x0], x), grad(PERMUTATIONS[x0 + 1], x - 1.f), ease(x));
}

float pragma::math::noise::get_noise(float x, int y)
{
	init();
	int x0 = floor(x);
	x -= x0;
	x0 &= PERMUTATION_MASK;
	y &= PERMUTATION_MASK;
	return lerp(grad(PERMUTATIONS[PERMUTATIONS[x0] + y], x), grad(PERMUTATIONS[PERMUTATIONS[x0 + 1] + y], x - 1.f), ease(x));
}

float pragma::math::noise::get_noise(float x, float y)
{
	init();
	int x0 = floor(x);
	int y0 = floor(y);
	x -= x0;
	y -= y0;
	x0 &= PERMUTATION_MASK;
	y0 &= PERMUTATION_MASK;
	int y1 = y0 + 1;
	int p0 = PERMUTATIONS[x0];
	int p1 = PERMUTATIONS[x0 + 1];
	float xm1 = x - 1.f, ym1 = y - 1.f;
	float s = ease(x);
	return lerp(lerp(grad(PERMUTATIONS[p0 + y0], x, y), grad(PERMUTATIONS[p1 + y0], xm1, y), s), lerp(grad(PERMUTATIONS[p0 + y1], x, ym1), grad(PERMUTATIONS[p1 + y1], xm1, ym1), s), ease(y));
}

float pragma::math::noise::get_noise(float x, float y, int z)
{
	init();
	int x0 = floor(x);
	int y0 = floor(y);
	x -= x0;
	y -= y0;
	x0 &= PERMUTATION_MASK;
	y0 &= PERMUTATION_MASK;
	z &= PERMUTATION_MASK;
	int y1 = y0 + 1;
	int p0 = PERMUTATIONS[x0];
	int p1 = PERMUTATIONS[x0 + 1];
	float xm1 = x - 1.f, ym1 = y - 1.f;
	float s = ease(x);
	return lerp(lerp(grad(PERMUTATIONS[PERMUTATIONS[p0 + y0] + z], x, y), grad(PERMUTATIONS[PERMUTATIONS[p1 + y0] + z], xm1, y), s), lerp(grad(PERMUTATIONS[PERMUTATIONS[p0 + y1] + z], x, ym1), grad(PERMUTATIONS[PERMUTATIONS[p1 + y1] + z], xm1, ym1), s), ease(y));
}

float pragma::math::noise::get_noise(float x, float y, float z)
{
	init();
	int x0 = floor(x);
	int y0 = floor(y);
	int z0 = floor(z);
	x -= x0;
	y -= y0;
	z -= z0;
	x0 &= PERMUTATION_MASK;
	y0 &= PERMUTATION_MASK;
	z0 &= PERMUTATION_MASK;
	int y1 = y0 + 1, z1 = z0 + 1;
	int p0 = PERMUTATIONS[x0];
	int p1 = PERMUTATIONS[x0 + 1];
	int p00 = PERMUTATIONS[p0 + y0];
	int p10 = PERMUTATIONS[p1 + y0];
	int p01 = PERMUTATIONS[p0 + y1];
	int p11 = PERMUTATIONS[p1 + y1];
	float xm1 = x - 1.f, ym1 = y - 1.f, zm1 = z - 1.f;
	float s = ease(x), t = ease(y);
	return lerp(lerp(lerp(grad(PERMUTATIONS[p00 + z0], x, y, z), grad(PERMUTATIONS[p10 + z0], xm1, y, z), s), lerp(grad(PERMUTATIONS[p01 + z0], x, ym1, z), grad(PERMUTATIONS[p11 + z0], xm1, ym1, z), s), t),
	  lerp(lerp(grad(PERMUTATIONS[p00 + z1], x, y, zm1), grad(PERMUTATIONS[p10 + z1], xm1, y, zm1), s), lerp(grad(PERMUTATIONS[p01 + z1], x, ym1, zm1), grad(PERMUTATIONS[p11 + z1], xm1, ym1, zm1), s), t), ease(z));
}

float pragma::math::noise::grad(int hash, float x)
{
	// TODO: would a lookup table or the bit tests in Perlin's reference implementation
	// be more efficient?
	switch(hash & GRADIENT_MASK) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 12:
		return x;
	case 1:
	case 3:
	case 5:
	case 7:
	case 13:
		return -x;
	default:
		return 0.f;
	}
}

float pragma::math::noise::grad(int hash, float x, float y)
{
	switch(hash & GRADIENT_MASK) {
	case 0:
	case 12:
		return x + y;
	case 1:
	case 13:
		return y - x;
	case 2:
		return x - y;
	case 3:
		return -x - y;
	case 4:
	case 6:
		return x;
	case 5:
	case 7:
		return -x;
	case 8:
	case 10:
		return y;
	case 9:
	case 11:
	case 14:
	case 15:
		return -y;
	default:
		return 0.f; // unreachable
	}
}

float pragma::math::noise::grad(int hash, float x, float y, float z)
{
	switch(hash & GRADIENT_MASK) {
	case 0:
	case 12:
		return x + y;
	case 1:
	case 13:
		return y - x;
	case 2:
		return x - y;
	case 3:
		return -x - y;
	case 4:
		return x + z;
	case 5:
		return z - x;
	case 6:
		return x - z;
	case 7:
		return -x - z;
	case 8:
		return y + z;
	case 9:
	case 14:
		return z - y;
	case 10:
		return y - z;
	case 11:
	case 15:
		return -y - z;
	default:
		return 0.f; // unreachable
	}
}

float pragma::math::noise::ease(float t) { return t * t * t * (t * (6.f * t - 15.f) + 10.f); }

void pragma::math::noise::init()
{
	static auto bInitialized = false;
	if(bInitialized)
		return;
	bInitialized = true;

	for(int ii = 0; ii < PERMUTATION_COUNT; ii++)
		PERMUTATIONS[ii] = ii;
	auto rng = std::default_random_engine {};
	std::shuffle(PERMUTATIONS.begin(), PERMUTATIONS.begin() + PERMUTATION_COUNT, rng);
	std::copy(PERMUTATIONS.begin(), PERMUTATIONS.begin() + PERMUTATION_COUNT, PERMUTATIONS.begin() + PERMUTATION_COUNT);
}
