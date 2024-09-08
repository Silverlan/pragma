#ifndef F_SH_GEOMETRY_GLS
#define F_SH_GEOMETRY_GLS

vec2 get_closest_point_on_line_to_point(vec2 a, vec2 b, vec2 p)
{
	vec2 ap = p - a;
	vec2 ab = b - a;
	float magAb2 = ab.x * ab.x + ab.y * ab.y;
	float abDotAp = ab.x * ap.x + ab.y * ap.y;
	float t = abDotAp / magAb2;
	vec2 r;
	if(t < 0)
		r = a;
	else if(t > 1)
		r = b;
	else
		r = vec2(a.x + ab.x * t, a.y + ab.y * t);
	return r;
}

bool get_aabb_line_intersection(vec2 p0, vec2 p1, vec2 p2, vec2 p3, out vec2 intersection)
{
	vec2 s1 = p1 - p0;
	vec2 s2 = p3 - p2;

	float s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
	float t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);
	if(s >= 0 && s <= 1 && t >= 0 && t <= 1) {
		intersection = vec2(p0.x + (t * s1.x), p0.y + (t * s1.y));
		return true;
	}
	return false;
}

#endif
