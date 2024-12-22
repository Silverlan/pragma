#include "/common/color.glsl"

vec3 mix_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, col2, t);
}

vec3 add_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, col1 +col2, t);
}

vec3 multiply_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, col1 *col2, t);
}

vec3 screen_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;
	vec3 one = vec3(1.0, 1.0, 1.0);
	vec3 tm3 = vec3(tm, tm, tm);

	return one -(tm3 +t *(one -col2)) *(one -col1);
}

vec3 overlay_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;

	vec3 outcol = col1;

	if(outcol.x < 0.5)
		outcol.x *= tm +2.0 *t *col2.x;
	else
		outcol.x = 1.0 -(tm +2.0 *t *(1.0 -col2.x)) *(1.0 -outcol.x);

	if(outcol.y < 0.5)
		outcol.y *= tm +2.0 *t *col2.y;
	else
		outcol.y = 1.0 -(tm +2.0 *t *(1.0 -col2.y)) *(1.0 -outcol.y);

	if(outcol.z < 0.5)
		outcol.z *= tm +2.0 *t *col2.z;
	else
		outcol.z = 1.0 -(tm +2.0 *t *(1.0 -col2.z)) *(1.0 -outcol.z);
	return outcol;
}

vec3 subtract_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, col1 -col2, t);
}

vec3 divide_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;
	vec3 outcol = col1;

	if(col2.x != 0.0)
		outcol.x = tm *outcol.x +t *outcol.x /col2.x;
	if(col2.y != 0.0)
		outcol.y = tm *outcol.y +t *outcol.y /col2.y;
	if(col2.z != 0.0)
		outcol.z = tm *outcol.z +t *outcol.z /col2.z;
	return outcol;
}

vec3 difference_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, abs(col1 -col2), t);
}

vec3 darken_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, min(col1, col2), t);
}

vec3 lighten_color(vec3 col1, vec3 col2, float t) {
	return mix(col1, max(col1, col2), t);
}

vec3 dodge_color(vec3 col1, vec3 col2, float t) {
	vec3 outcol = col1;

	if(outcol.x != 0.0) {
		float tmp = 1.0 -t *col2.x;
		if(tmp <= 0.0)
			outcol.x = 1.0;
		else if((tmp = outcol.x /tmp) > 1.0)
			outcol.x = 1.0;
		else
			outcol.x = tmp;
	}
	if(outcol.y != 0.0) {
		float tmp = 1.0 -t *col2.y;
		if(tmp <= 0.0)
			outcol.y = 1.0;
		else if((tmp = outcol.y /tmp) > 1.0)
			outcol.y = 1.0;
		else
			outcol.y = tmp;
	}
	if(outcol.z != 0.0) {
		float tmp = 1.0 -t *col2.z;
		if(tmp <= 0.0)
			outcol.z = 1.0;
		else if((tmp = outcol.z /tmp) > 1.0)
			outcol.z = 1.0;
		else
			outcol.z = tmp;
	}
	return outcol;
}

vec3 burn_color(vec3 col1, vec3 col2, float t) {
	float tmp, tm = 1.0 -t;

	vec3 outcol = col1;

	tmp = tm +t *col2.x;
	if(tmp <= 0.0)
		outcol.x = 0.0;
	else if((tmp = (1.0 -(1.0 -outcol.x) /tmp)) < 0.0)
		outcol.x = 0.0;
	else if(tmp > 1.0)
		outcol.x = 1.0;
	else
		outcol.x = tmp;

	tmp = tm +t *col2.y;
	if(tmp <= 0.0)
		outcol.y = 0.0;
	else if((tmp = (1.0 -(1.0 -outcol.y) /tmp)) < 0.0)
		outcol.y = 0.0;
	else if(tmp > 1.0)
		outcol.y = 1.0;
	else
		outcol.y = tmp;

	tmp = tm +t *col2.z;
	if(tmp <= 0.0)
		outcol.z = 0.0;
	else if((tmp = (1.0 -(1.0 -outcol.z) /tmp)) < 0.0)
		outcol.z = 0.0;
	else if(tmp > 1.0)
		outcol.z = 1.0;
	else
		outcol.z = tmp;
	return outcol;
}

vec3 hue_color(vec3 col1, vec3 col2, float t) {
	vec3 outcol = col1;

	vec3 hsv2 = rgb_to_hsv(col2);

	if(hsv2.y != 0.0) {
		vec3 hsv = rgb_to_hsv(outcol);
		hsv.x = hsv2.x;
		vec3 tmp = hsv_to_rgb(hsv);

		outcol = mix(outcol, tmp, t);
	}

	return outcol;
}

vec3 saturation_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;

	vec3 outcol = col1;

	vec3 hsv = rgb_to_hsv(outcol);

	if(hsv.y != 0.0) {
		vec3 hsv2 = rgb_to_hsv(col2);

		hsv.y = tm *hsv.y +t *hsv2.y;
		outcol = hsv_to_rgb(hsv);
	}

	return outcol;
}

vec3 value_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;

	vec3 hsv = rgb_to_hsv(col1);
	vec3 hsv2 = rgb_to_hsv(col2);

	hsv.z = tm *hsv.z +t *hsv2.z;

	return hsv_to_rgb(hsv);
}

vec3 color_color(vec3 col1, vec3 col2, float t) {
	vec3 outcol = col1;
	vec3 hsv2 = rgb_to_hsv(col2);

	if(hsv2.y != 0.0) {
		vec3 hsv = rgb_to_hsv(outcol);
		hsv.x = hsv2.x;
		hsv.y = hsv2.y;
		vec3 tmp = hsv_to_rgb(hsv);

		outcol = mix(outcol, tmp, t);
	}

	return outcol;
}

vec3 soft_light_color(vec3 col1, vec3 col2, float t) {
	float tm = 1.0 -t;

	vec3 one = vec3(1.0, 1.0, 1.0);
	vec3 scr = one -(one -col2) *(one -col1);

	return tm *col1 +t *((one -col1) *col2 *col1 +col1 *scr);
}

vec3 linear_light_color(vec3 col1, vec3 col2, float t) {
	return col1 +t *(2.0 *col2 +vec3(-1.0, -1.0, -1.0));
}

vec3 exclusion_color(vec3 col1, vec3 col2, float t) {
	return max(mix(col1, col1 +col2 -2.0 *col1 *col2, t), vec3(0.0, 0.0, 0.0));
}
