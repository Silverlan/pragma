#ifndef F_SH_CONDITIONALS_GLS
#define F_SH_CONDITIONALS_GLS

// See http://theorangeduck.com/page/avoiding-shader-conditionals

// vec4
vec4 when_eq(vec4 x, vec4 y) { return 1.0 - abs(sign(x - y)); }

vec4 when_neq(vec4 x, vec4 y) { return abs(sign(x - y)); }

vec4 when_gt(vec4 x, vec4 y) { return max(sign(x - y), 0.0); }

vec4 when_lt(vec4 x, vec4 y) { return max(sign(y - x), 0.0); }

vec4 when_ge(vec4 x, vec4 y) { return 1.0 - when_lt(x, y); }

vec4 when_le(vec4 x, vec4 y) { return 1.0 - when_gt(x, y); }

vec4 and (vec4 a, vec4 b) { return a * b; }

vec4 or (vec4 a, vec4 b) { return min(a + b, 1.0); }

vec4
  xor (vec4 a, vec4 b) { return vec4(mod((a.x + b.x), 2.0), mod((a.y + b.y), 2.0), mod((a.z + b.z), 2.0), mod((a.w + b.w), 2.0)); }

  vec4 not(vec4 a)
{
	return 1.0 - a;
}

// vec3
vec3 when_eq(vec3 x, vec3 y) { return 1.0 - abs(sign(x - y)); }

vec3 when_neq(vec3 x, vec3 y) { return abs(sign(x - y)); }

vec3 when_gt(vec3 x, vec3 y) { return max(sign(x - y), 0.0); }

vec3 when_lt(vec3 x, vec3 y) { return max(sign(y - x), 0.0); }

vec3 when_ge(vec3 x, vec3 y) { return 1.0 - when_lt(x, y); }

vec3 when_le(vec3 x, vec3 y) { return 1.0 - when_gt(x, y); }

vec3 and (vec3 a, vec3 b) { return a * b; }

vec3 or (vec3 a, vec3 b) { return min(a + b, 1.0); }

vec3
  xor (vec3 a, vec3 b) { return vec3(mod((a.x + b.x), 2.0), mod((a.y + b.y), 2.0), mod((a.z + b.z), 2.0)); }

  vec3 not(vec3 a)
{
	return 1.0 - a;
}

// vec2
vec2 when_eq(vec2 x, vec2 y) { return 1.0 - abs(sign(x - y)); }

vec2 when_neq(vec2 x, vec2 y) { return abs(sign(x - y)); }

vec2 when_gt(vec2 x, vec2 y) { return max(sign(x - y), 0.0); }

vec2 when_lt(vec2 x, vec2 y) { return max(sign(y - x), 0.0); }

vec2 when_ge(vec2 x, vec2 y) { return 1.0 - when_lt(x, y); }

vec2 when_le(vec2 x, vec2 y) { return 1.0 - when_gt(x, y); }

vec2 and (vec2 a, vec2 b) { return a * b; }

vec2 or (vec2 a, vec2 b) { return min(a + b, 1.0); }

vec2
  xor (vec2 a, vec2 b) { return vec2(mod((a.x + b.x), 2.0), mod((a.y + b.y), 2.0)); }

  vec2 not(vec2 a)
{
	return 1.0 - a;
}

// float
float when_eq(float x, float y) { return 1.0 - abs(sign(x - y)); }

float when_neq(float x, float y) { return abs(sign(x - y)); }

float when_gt(float x, float y) { return max(sign(x - y), 0.0); }

float when_lt(float x, float y) { return max(sign(y - x), 0.0); }

float when_ge(float x, float y) { return 1.0 - when_lt(x, y); }

float when_le(float x, float y) { return 1.0 - when_gt(x, y); }

float and (float a, float b) { return a * b; }

float or (float a, float b) { return min(a + b, 1.0); }

float
  xor (float a, float b) { return mod((a.x + b.x), 2.0); }

  float not(float a)
{
	return 1.0 - a;
}

// int
int when_eq(int x, int y) { return 1 - abs(sign(x - y)); }

int when_neq(int x, int y) { return abs(sign(x - y)); }

int when_gt(int x, int y) { return max(sign(x - y), 0); }

int when_lt(int x, int y) { return max(sign(y - x), 0); }

int when_ge(int x, int y) { return 1 - when_lt(x, y); }

int when_le(int x, int y) { return 1 - when_gt(x, y); }

int and (int a, int b) { return a * b; }

int or (int a, int b) { return min(a + b, 1); }

int not(int a) { return 1 - a; }

// uint
uint when_eq(uint x, uint y) { return uint(1 - abs(sign(x - y))); }

uint when_neq(uint x, uint y) { return uint(abs(sign(x - y))); }

uint when_gt(uint x, uint y) { return uint(max(sign(x - y), 0)); }

uint when_lt(uint x, uint y) { return uint(max(sign(y - x), 0)); }

uint when_ge(uint x, uint y) { return 1 - when_lt(x, y); }

uint when_le(uint x, uint y) { return 1 - when_gt(x, y); }

uint and (uint a, uint b) { return a * b; }

uint or (uint a, uint b) { return min(a + b, 1); }

uint not(uint a) { return 1 - a; }

#endif
