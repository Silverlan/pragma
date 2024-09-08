#ifndef F_INTERSECTION_GLS
#define F_INTERSECTION_GLS

// Source: https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
bool intersect_ray_aabb(vec3 origin, vec3 dirInv, vec3 aabbMin, vec3 aabbMax)
{
	double t1 = (aabbMin[0] - origin[0]) * dirInv[0];
	double t2 = (aabbMax[0] - origin[0]) * dirInv[0];

	double tmin = min(t1, t2);
	double tmax = max(t1, t2);

	for(int i = 1; i < 3; ++i) {
		t1 = (aabbMin[i] - origin[i]) * dirInv[i];
		t2 = (aabbMax[i] - origin[i]) * dirInv[i];

		tmin = max(tmin, min(t1, t2));
		tmax = min(tmax, max(t1, t2));
	}

	return tmax > max(tmin, 0.0);
}

// Source: https://www.shadertoy.com/view/MlGcDz
vec3 intersect_ray_triangle(vec3 origin, vec3 dir, vec3 v0, vec3 v1, vec3 v2)
{
	vec3 v1v0 = v1 - v0;
	vec3 v2v0 = v2 - v0;
	vec3 rov0 = origin - v0;

#if 0
	// Cramer's rule for solcing p(t) = ro+t·rd = p(u,v) = vo + u·(v1-v0) + v·(v2-v1)
	float d = 1.0/determinant(mat3(v1v0, v2v0, -rd ));
	float u =   d*determinant(mat3(rov0, v2v0, -rd ));
	float v =   d*determinant(mat3(v1v0, rov0, -rd ));
	float t =   d*determinant(mat3(v1v0, v2v0, rov0));
#else
	// The four determinants above have lots of terms in common. Knowing the changing
	// the order of the columns/rows doesn't change the volume/determinant, and that
	// the volume is dot(cross(a,b,c)), we can precompute some common terms and reduce
	// it all to:
	vec3 n = cross(v1v0, v2v0);

	// Back-face culling
	//if (dot(dir,normalize(n)) > 0)
	//	return vec3(-1,0,0);

	vec3 q = cross(rov0, dir);
	float d = 1.0 / dot(dir, n);
	float u = d * dot(-q, v2v0);
	float v = d * dot(q, v1v0);
	float t = d * dot(-n, rov0);
#endif

	if(u < 0.0 || v < 0.0 || (u + v) > 1.0)
		t = -1.0;

	return vec3(t, u, v);
}

// Source: https://bartwronski.com/2017/04/13/cull-that-cone/
bool intersect_cone_sphere(in vec3 origin, in vec3 forward, in float size, in float angle, in vec4 testSphere)
{
	const vec3 V = testSphere.xyz - origin;
	const float VlenSq = dot(V, V);
	const float V1len = dot(V, forward);
	const float distanceClosestPoint = cos(angle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(angle);

	const bool angleCull = distanceClosestPoint > testSphere.w;
	const bool frontCull = V1len > testSphere.w + size;
	const bool backCull = V1len < -testSphere.w;
	return /*testSphere.w > 1 ||*/ !(angleCull || frontCull || backCull);
}

// See https://simoncoenen.com/blog/programming/graphics/SpotlightCulling.html
struct Cone {
	vec3 Direction;
	vec3 Tip;
	float Height;
	float Radius;
	float Angle;
};
bool intersect_cone_aabb(Cone spotlight, vec3 min, vec3 max)
{
	vec3 extents = max - min;
	vec3 center = (max + min) / 2.0;
	float sphereRadius = dot(extents, extents);
	vec3 v = center - spotlight.Tip;
	float lenSq = dot(v, v);
	float v1Len = dot(v, spotlight.Direction);
	float distanceClosestPoint = cos(spotlight.Angle / 2) * sqrt(lenSq - v1Len * v1Len) - v1Len * sin(spotlight.Angle / 2);
	bool angleCull = distanceClosestPoint > sphereRadius;
	bool frontCull = v1Len > sphereRadius + spotlight.Height;
	bool backCull = v1Len < -sphereRadius;
	return !(angleCull || frontCull || backCull);
}

#endif
