#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 v_uv; 
layout(location = 0) out vec4 fs_color;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants {GUI_BASE_PUSH_CONSTANTS} u_pushConstants;

layout(std140, LAYOUT_ID(GUI, RECT_STYLE)) uniform RectStyle 
{
	vec4 gradientColors[4];
	vec4 gradientColorStops; /* Position of each color (0.0 to 1.0) */
	vec4 borderColor;
	vec4 cornerRadii;
	vec2 gradientStart;
	vec2 gradientEnd;
	float borderThickness;
	int gradientColorMode; /* 1 = Solid, 2-4 = Gradient */
	int gradientType; /* 0 = Linear, 1 = Radial */
	float _padding;
} u_style;

vec4 getGradientColor(vec2 uv) {
	if (u_style.gradientColorMode <= 1)
		return u_style.gradientColors[0]; // Solid color
	
	float t = 0.0;
	// Linear Gradient
	if (u_style.gradientType == 0) {
		vec2 dir = u_style.gradientEnd -u_style.gradientStart;
		float lenSq = dot(dir, dir);
		if (lenSq > 0.00001) {
			// Project current UV onto the gradient line
			t = dot(uv -u_style.gradientStart, dir) /lenSq;
		}
	} 
	else if (u_style.gradientType == 1) { // Radial Gradient
		float d = distance(uv, u_style.gradientStart);
		float maxD = distance(u_style.gradientStart, u_style.gradientEnd);
		if (maxD > 0.00001)
			t = d / maxD;
	}
	
	t = clamp(t, 0.0, 1.0);
	vec4 stops = u_style.gradientColorStops;
	
	if (t <= stops.x)
		return u_style.gradientColors[0];
	
	if (t < stops.y) {
		float factor = (t -stops.x) /max(stops.y -stops.x, 0.00001);
		return mix(u_style.gradientColors[0], u_style.gradientColors[1], factor);
	}
	
	if (t < stops.z && u_style.gradientColorMode >= 3) {
		float factor = (t -stops.y) /max(stops.z -stops.y, 0.00001);
		return mix(u_style.gradientColors[1], u_style.gradientColors[2], factor);
	}
	
	if (t < stops.w && u_style.gradientColorMode == 4) {
		float factor = (t -stops.z) /max(stops.w -stops.z, 0.00001);
		return mix(u_style.gradientColors[2], u_style.gradientColors[3], factor);
	}
	return u_style.gradientColors[u_style.gradientColorMode - 1];
}

vec2 get_element_size() {
	return vec2(
		length(u_pushConstants.modelMatrix[0]),
		length(u_pushConstants.modelMatrix[1])
	);
}

void main() {
	vec4 baseColor = getGradientColor(v_uv) *u_pushConstants.color;

	vec2 size = get_element_size();
	vec2 halfSize = size *0.5;
	vec2 pixelPos = (v_uv -0.5) * size;
	
	// Determine corner radius quadrant
	vec4 radii = u_style.cornerRadii;
	vec2 r_x = (pixelPos.x > 0.0) ? radii.yz : radii.xw; // right (yz) or left (xw)
	float radius = (pixelPos.y > 0.0) ? r_x.y : r_x.x; // bottom (y) or top (x)
	
	// Clamp to prevent artifacts
	radius = min(radius, min(halfSize.x, halfSize.y));
	
	// SDF to the edge
	vec2 q = abs(pixelPos) -halfSize +radius;
	float dist = min(max(q.x, q.y), 0.0) +length(max(q, 0.0)) -radius;
	
	// Anti-aliasing
	float outerAlpha = 1.0 -smoothstep(-0.5, 0.5, dist);
	
	if (outerAlpha <= 0.0)
		discard;
	
	// Border
	vec4 finalColor;
	if (u_style.borderThickness > 0.0) {
		float innerDist = dist +u_style.borderThickness;
		float borderAlpha = smoothstep(-0.5, 0.5, innerDist);
		
		finalColor = mix(baseColor, u_style.borderColor, borderAlpha);
	} else
		finalColor = baseColor;
	
	finalColor.a *= outerAlpha;
	
	fs_color = finalColor;
}
