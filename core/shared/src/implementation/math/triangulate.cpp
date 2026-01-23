// ** THIS IS A CODE SNIPPET WHICH WILL EFFICIEINTLY TRIANGULATE ANY
// ** POLYGON/CONTOUR (without holes) AS A STATIC CLASS.  THIS SNIPPET
// ** IS COMPRISED OF 3 FILES, TRIANGULATE.H, THE HEADER FILE FOR THE
// ** TRIANGULATE BASE CLASS, TRIANGULATE.CPP, THE IMPLEMENTATION OF
// ** THE TRIANGULATE BASE CLASS, AND TEST.CPP, A SMALL TEST PROGRAM
// ** DEMONSTRATING THE USAGE OF THE TRIANGULATOR.  THE TRIANGULATE
// ** BASE CLASS ALSO PROVIDES TWO USEFUL HELPER METHODS, ONE WHICH
// ** COMPUTES THE AREA OF A POLYGON, AND ANOTHER WHICH DOES AN EFFICENT
// ** POINT IN A TRIANGLE TEST.
// ** SUBMITTED BY JOHN W. RATCLIFF (jratcliff@verant.com) July 22, 2000

// Source: http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
module;

module pragma.shared;

import :math.triangulate;

static const auto EPSILON = 0.0000000001f;

float Geometry::calc_area(const std::vector<Vector2> &contour)
{
	auto n = contour.size();
	auto A = 0.0f;
	for(int32_t p = n - 1, q = 0; q < n; p = q++)
		A += contour.at(p).x * contour.at(q).y - contour.at(q).x * contour.at(p).y;
	return A * 0.5f;
}

/*
	InsideTriangle decides if a point P is Inside of the triangle
	defined by A, B, C.
*/
bool Geometry::inside_triangle(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Px, float Py)
{
	float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;

	ax = Cx - Bx;
	ay = Cy - By;
	bx = Ax - Cx;
	by = Ay - Cy;
	cx = Bx - Ax;
	cy = By - Ay;
	apx = Px - Ax;
	apy = Py - Ay;
	bpx = Px - Bx;
	bpy = Py - By;
	cpx = Px - Cx;
	cpy = Py - Cy;

	aCROSSbp = ax * bpy - ay * bpx;
	cCROSSap = cx * apy - cy * apx;
	bCROSScp = bx * cpy - by * cpx;

	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

static bool snip(const std::vector<Vector2> &contour, int u, int v, int w, int n, const std::vector<int32_t> &V)
{
	int32_t p;
	float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

	Ax = contour.at(V.at(u)).x;
	Ay = contour.at(V.at(u)).y;

	Bx = contour.at(V.at(v)).x;
	By = contour.at(V.at(v)).y;

	Cx = contour.at(V.at(w)).x;
	Cy = contour.at(V.at(w)).y;

	if(EPSILON > (((Bx - Ax) * (Cy - Ay)) - ((By - Ay) * (Cx - Ax))))
		return false;

	for(p = 0; p < n; p++) {
		if((p == u) || (p == v) || (p == w))
			continue;
		Px = contour.at(V.at(p)).x;
		Py = contour.at(V.at(p)).y;
		if(Geometry::inside_triangle(Ax, Ay, Bx, By, Cx, Cy, Px, Py))
			return false;
	}
	return true;
}

bool Geometry::triangulate(const std::vector<Vector2> &contour, std::vector<uint16_t> &result)
{
	//Con::COUT<<"local contour = {"<<Con::endl;
	//for(auto &p : contour)
	//	Con::COUT<<"::Vector2("<<p.x<<","<<p.y<<"),"<<Con::endl;
	//Con::COUT<<"}"<<Con::endl;
	/* allocate and initialize list of Vertices in polygon */
	auto n = contour.size();
	if(n < 3)
		return false;

	std::vector<int32_t> V(n);

	/* we want a counter-clockwise polygon in V */
	if(0.0f < calc_area(contour)) {
		for(auto v = decltype(n) {0}; v < n; v++)
			V.at(v) = v;
	}
	else {
		for(auto v = decltype(n) {0}; v < n; v++)
			V.at(v) = (n - 1) - v;
	}
	auto nv = n;

	/*  remove nv-2 Vertices, creating 1 triangle every time */
	int32_t count = 2 * nv; /* error detection */

	for(uint32_t m = 0, v = nv - 1; nv > 2;) {
		/* if we loop, it is probably a non-simple polygon */
		if(count-- <= 0) {
			//** Triangulate: ERROR - probable bad polygon!
			return false;
		}

		/* three consecutive vertices in current polygon, <u,v,w> */
		auto u = v;
		if(nv <= u)
			u = 0; /* previous */
		v = u + 1;
		if(nv <= v)
			v = 0; /* new v    */

		auto w = v + 1;
		if(nv <= w)
			w = 0; /* next     */

		if(snip(contour, u, v, w, nv, V)) {
			int32_t a, b, c, s, t;

			/* true names of the vertices */
			a = V.at(u);
			b = V.at(v);
			c = V.at(w);

			/* output Triangle */
			result.push_back(a);
			result.push_back(b);
			result.push_back(c);

			m++;
			/* remove v from remaining polygon */
			for(s = v, t = v + 1; t < nv; s++, t++)
				V.at(s) = V.at(t);
			nv--;

			/* resest error detection counter */
			count = 2 * nv;
		}
	}
	return true;
}
