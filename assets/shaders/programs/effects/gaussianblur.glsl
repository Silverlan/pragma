#ifndef F_GAUSSIANBLUR_GLS
#define F_GAUSSIANBLUR_GLS

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants
{
	vec4 colorScale;
	float blurSize;
	int kernelSize;
}
u_pushConstants;

float get_blur_size() { return u_pushConstants.blurSize; }

// Default values generated with radius = 6, sigma = 10
layout(constant_id = 0) const int NUM_SAMPLES = 7;
layout(constant_id = 1) const float OFFSET0 = -5.4862649103797;
layout(constant_id = 2) const float OFFSET1 = -3.4912581799744;
layout(constant_id = 3) const float OFFSET2 = -1.4962531940808;
layout(constant_id = 4) const float OFFSET3 = 0.49875104391628;
layout(constant_id = 5) const float OFFSET4 = 2.4937555312288;
layout(constant_id = 6) const float OFFSET5 = 4.4887612648697;
layout(constant_id = 7) const float OFFSET6 = 6;
layout(constant_id = 8) const float OFFSET7 = 0;
layout(constant_id = 9) const float OFFSET8 = 0;
layout(constant_id = 10) const float OFFSET9 = 0;
layout(constant_id = 11) const float OFFSET10 = 0;
layout(constant_id = 12) const float OFFSET11 = 0;
layout(constant_id = 13) const float OFFSET12 = 0;
layout(constant_id = 14) const float OFFSET13 = 0;
layout(constant_id = 15) const float OFFSET14 = 0;

layout(constant_id = 16) const float WEIGHT0 = 0.14145779303823;
layout(constant_id = 17) const float WEIGHT1 = 0.15473312090618;
layout(constant_id = 18) const float WEIGHT2 = 0.1626393862735;
layout(constant_id = 19) const float WEIGHT3 = 0.1642684709161;
layout(constant_id = 20) const float WEIGHT4 = 0.15942952446961;
layout(constant_id = 21) const float WEIGHT5 = 0.14868574334213;
layout(constant_id = 22) const float WEIGHT6 = 0.068785961054241;
layout(constant_id = 23) const float WEIGHT7 = 0;
layout(constant_id = 24) const float WEIGHT8 = 0;
layout(constant_id = 25) const float WEIGHT9 = 0;
layout(constant_id = 26) const float WEIGHT10 = 0;
layout(constant_id = 27) const float WEIGHT11 = 0;
layout(constant_id = 28) const float WEIGHT12 = 0;
layout(constant_id = 29) const float WEIGHT13 = 0;
layout(constant_id = 30) const float WEIGHT14 = 0;

float OFFSETS[15] = float[15](OFFSET0, OFFSET1, OFFSET2, OFFSET3, OFFSET4, OFFSET5, OFFSET6, OFFSET7, OFFSET8, OFFSET9, OFFSET10, OFFSET11, OFFSET12, OFFSET13, OFFSET14);

float WEIGHTS[15] = float[15](WEIGHT0, WEIGHT1, WEIGHT2, WEIGHT3, WEIGHT4, WEIGHT5, WEIGHT6, WEIGHT7, WEIGHT8, WEIGHT9, WEIGHT10, WEIGHT11, WEIGHT12, WEIGHT13, WEIGHT14);

float blurRadius = 0.5;
float Intensity = 1.02;
vec4 gaussianBlur(sampler2D tex, vec2 coord, vec2 dir)
{
	vec4 result = vec4(0.0);
	vec2 size = textureSize(tex, 0);
	for(int i = 0; i < NUM_SAMPLES; ++i) {
		vec2 offset = dir * OFFSETS[i] / size;
		float weight = WEIGHTS[i];
		result += texture(tex, coord + offset) * weight;
	}
	result.a = 1;
	return result * Intensity;
}

vec4 get_gaussian_blur_horizontal(sampler2D tex, vec2 UV) { return gaussianBlur(tex, UV, vec2(1, 0)); }

vec4 get_gaussian_blur_vertical(sampler2D tex, vec2 UV) { return gaussianBlur(tex, UV, vec2(0, 1)); }

#endif
