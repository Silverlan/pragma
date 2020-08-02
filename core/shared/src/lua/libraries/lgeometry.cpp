/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lgeometry.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/math/intersection.h>
#include "luasystem.h"
#include "pragma/math/math_seb.h"
#include "pragma/math/util_triangulate.hpp"
#include "pragma/math/vector/util_winding_order.hpp"
#include <clip2tri/clip2tri.h>
//#include <Pinocchio.h>
//#include <pinocchioApi.h>
//#include <pragma/model/mesh.h>
//#include <triangle.h>

Vector3 Lua::geometry::closest_point_on_aabb_to_point(const Vector3 &min,const Vector3 &max,const Vector3 &p)
{
	Vector3 res;
	Geometry::ClosestPointOnAABBToPoint(min,max,p,&res);
	return res;
}

void Lua::geometry::closest_points_between_lines(const Vector3 &pA,const Vector3 &qA,const Vector3 &pB,const Vector3 &qB,Vector3 &outCA,Vector3 &outCB,float &outD)
{
	float s,t;
	outD = Geometry::ClosestPointsBetweenLines(pA,qA,pB,qB,&s,&t,&outCA,&outCB);
}

Vector3 Lua::geometry::closest_point_on_plane_to_point(const Vector3 &n,float d,const Vector3 &p)
{
	Vector3 res;
	Geometry::ClosestPointOnPlaneToPoint(n,d,p,&res);
	return res;
}

Vector3 Lua::geometry::closest_point_on_triangle_to_point(const Vector3 &a,const Vector3 &b,const Vector3 &c,const Vector3 &p)
{
	Vector3 res;
	Geometry::ClosestPointOnTriangleToPoint(a,b,c,p,&res);
	return res;
}

void Lua::geometry::smallest_enclosing_sphere(lua_State *l,luabind::table<> tVerts,Vector3 &outCenter,float &outRadius)
{
	auto verts = Lua::table_to_vector<Vector3>(l,tVerts,1);
	Seb::Calculate(verts,outCenter,outRadius);
}

void Lua::geometry::generate_truncated_cone_mesh(
	lua_State *l,const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,
	luabind::object &outVerts,luabind::object &outTris,luabind::object &outNormals,
	uint32_t segmentCount,bool caps,bool generateTriangles,bool generateNormals
)
{
	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	std::vector<Vector3> normals;
	Geometry::GenerateTruncatedConeMesh(origin,static_cast<float>(startRadius),dir,static_cast<float>(dist),static_cast<float>(endRadius),verts,(generateTriangles == true) ? &triangles : nullptr,(generateNormals == true) ? &normals : nullptr,segmentCount,caps);

	outVerts = Lua::vector_to_table(l,verts);
	auto *outNext = &outTris;
	if(generateTriangles == true)
	{
		*outNext = Lua::vector_to_table(l,triangles);
		outNext = &outNormals;
	}
	if(generateNormals == true)
		*outNext = Lua::vector_to_table(l,normals);
}
double Lua::geometry::calc_volume_of_polyhedron(lua_State *l,luabind::table<> tVerts,luabind::table<> tTriangles)
{
	auto verts = Lua::table_to_vector<Vector3>(l,tVerts,1);
	auto tris = Lua::table_to_vector<uint16_t>(l,tTriangles,2);
	auto numTris = Lua::GetObjectLength(l,2);
	uint32_t idx = 0;
	return Geometry::calc_volume_of_polyhedron([l,&verts,&tris,numTris,idx](const Vector3 **v0,const Vector3 **v1,const Vector3 **v2) mutable -> bool {
		if(idx >= numTris)
			return false;
		auto idx0 = tris.at(idx);
		auto idx1 = tris.at(idx +1);
		auto idx2 = tris.at(idx +2);
		*v0 = &verts.at(idx0);
		*v1 = &verts.at(idx1);
		*v2 = &verts.at(idx2);

		idx += 3;
		return true;
	});
}
void Lua::geometry::calc_center_of_mass(lua_State *l,luabind::table<> tVerts,luabind::table<> tTriangles,Vector3 &outCom,double &outVolume)
{
	auto verts = Lua::table_to_vector<Vector3>(l,tVerts,1);
	auto tris = Lua::table_to_vector<uint16_t>(l,tTriangles,2);
	auto numTris = Lua::GetObjectLength(l,2);
	uint32_t idx = 0;
	outVolume = Geometry::calc_volume_of_polyhedron([l,&verts,&tris,numTris,idx](const Vector3 **v0,const Vector3 **v1,const Vector3 **v2) mutable -> bool {
		if(idx >= numTris)
			return false;
		auto idx0 = tris.at(idx);
		auto idx1 = tris.at(idx +1);
		auto idx2 = tris.at(idx +2);
		*v0 = &verts.at(idx0);
		*v1 = &verts.at(idx1);
		*v2 = &verts.at(idx2);

		idx += 3;
		return true;
	},&outCom);
}
Vector2 Lua::geometry::calc_barycentric_coordinates(const Vector3 &p0,const Vector3 &p1,const Vector3 &p2,const Vector3 &hitPoint)
{
	float b1,b2;
	auto r = ::Geometry::calc_barycentric_coordinates(p0,p1,p2,hitPoint,b1,b2);
	return Vector2{b1,b2};
}
Vector2 Lua::geometry::calc_barycentric_coordinates(const Vector3 &p0,const Vector2 &uv0,const Vector3 &p1,const Vector2 &uv1,const Vector3 &p2,const Vector2 &uv2,const Vector3 &hitPoint)
{
	float b1,b2;
	auto r = ::Geometry::calc_barycentric_coordinates(p0,uv0,p1,uv1,p2,uv2,hitPoint,b1,b2);
	return Vector2{b1,b2};
}

int Lua::geometry::get_outline_vertices(lua_State *l)
{
	std::vector<Vector2> vertices {};
	Lua::CheckTable(l,1);
	auto numVerts = Lua::GetObjectLength(l,1);
	vertices.reserve(numVerts);
	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		Lua::PushInt(l,i +1u);
		Lua::GetTableValue(l,1);
		vertices.push_back(*Lua::CheckVector2(l,-1));
	}
	auto indices = ::Geometry::get_outline_vertices(vertices);
	if(indices.has_value() == false)
		return 0;
	auto t = Lua::CreateTable(l);
	auto tIdx = 1;
	for(auto idx : *indices)
	{
		Lua::PushInt(l,tIdx++);
		Lua::PushInt(l,idx);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::geometry::triangulate_point_cloud(lua_State *l)
{
	std::vector<c2t::Point> polyMesh = {
		c2t::Point(15.8477,-35.3979),
		c2t::Point(16.6043,-35.0729),
		c2t::Point(17.1424,-35.5665),
		c2t::Point(17.1984,-35.1103),
		c2t::Point(15.7691,-35.0203),
		c2t::Point(15.7247,-35.3819),
		c2t::Point(15.7332,-35.313),
		c2t::Point(15.8565,-34.38),
		c2t::Point(15.8478,-34.3789),
		c2t::Point(15.8478,-34.379),
		c2t::Point(15.8477,-34.3795),
		c2t::Point(0.149531,-9.97178),
		c2t::Point(8.05614,-38.5316),
		c2t::Point(-25.0149,-33.6999),
		c2t::Point(-23.5553,-34.2234),
		c2t::Point(-23.5515,-34.1923),
		c2t::Point(-23.5466,-34.1528),
		c2t::Point(7.68747,-38.5442),
		c2t::Point(7.73416,-38.1639),
		c2t::Point(8.64026,-38.5116),
		c2t::Point(8.90279,-36.3726),
		c2t::Point(20.4594,-12.6142),
		c2t::Point(18.9703,-24.7458),
		c2t::Point(-21.1867,-7.19587),
		c2t::Point(-22.9422,-21.4985),
		c2t::Point(-25.0672,-34.1261),
		c2t::Point(-24.8355,-34.1563),
		c2t::Point(-24.4541,-34.2707),
		c2t::Point(-23.5862,-34.4754),
		c2t::Point(9.18273,-34.0919),
		c2t::Point(13.5483,-34.6524),
		c2t::Point(15.6514,-34.9149),
		c2t::Point(17.4817,-35.1531),
		c2t::Point(17.6883,-35.191),
		c2t::Point(-24.4295,-33.6156),
		c2t::Point(-24.6597,-33.5857),
		c2t::Point(-24.988,-33.4807),
		c2t::Point(-20.1976,-7.75449),
		c2t::Point(-21.6923,-7.49269),
		c2t::Point(-20.1013,-6.98903),
		c2t::Point(-20.6403,-6.9189),
		c2t::Point(-21.6064,-6.79321),
		c2t::Point(20.9442,-13.0587),
		c2t::Point(20.8524,-13.8069),
		c2t::Point(20.2776,-13.7321),
		c2t::Point(19.7516,-13.6637),
		c2t::Point(19.8401,-12.946),
		c2t::Point(19.8659,-12.1888),
		c2t::Point(21.0324,-12.3406),
		c2t::Point(20.4333,-12.2626),
		c2t::Point(-20.2858,-8.45477),
		c2t::Point(-21.2402,-8.3306),
		c2t::Point(-21.7864,-8.25954),
		c2t::Point(5.4078,-37.9981),
		c2t::Point(7.76442,-37.9173),
		c2t::Point(0.608846,-38.3313),
		c2t::Point(3.08172,-38.0778),
		c2t::Point(-23.5093,-33.8485),
		c2t::Point(-21.3212,-34.5077),
		c2t::Point(-19.0126,-35.2033),
		c2t::Point(-16.3538,-36.1175),
		c2t::Point(-13.988,-37.0078),
		c2t::Point(-11.2359,-37.4177),
		c2t::Point(-8.117,-37.8754),
		c2t::Point(-5.34107,-38.1845),
		c2t::Point(-2.22512,-38.5382)
	};

	std::vector<std::vector<c2t::Point>> inputPolygons = {polyMesh};
	std::vector<c2t::Point> outputTriangles;
	std::vector<c2t::Point> boundingPolygon;

	c2t::clip2tri clip2tri;
	clip2tri.triangulate(inputPolygons,outputTriangles,boundingPolygon);

	auto t = Lua::CreateTable(l);
	for(auto i=decltype(outputTriangles.size()){0};i<outputTriangles.size();++i)
	{
		auto &p = outputTriangles.at(i);
		Lua::PushInt(l,i +1);
		Lua::Push<Vector2>(l,Vector2(p.x,p.y));
		Lua::SetTableValue(l,t);
	}
/*
   vector<vector<Point> > inputPolygons;
   vector<Point> outputTriangles;  // Every 3 points is a triangle

   vector<Point> boundingPolygon;

   clip2tri clip2tri;
   clip2tri.triangulate(inputPolygons, outputTriangles, boundingPolygon);
*/
	return 1;
	/*int32_t tIdx = 1;
	Lua::CheckTable(l,tIdx);
	std::vector<Vector2> pointCloud;
	auto numPoints = Lua::GetObjectLength(l,tIdx);
	pointCloud.reserve(numPoints);
	for(auto i=decltype(numPoints){0};i<numPoints;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tIdx);
		auto &v = *Lua::CheckVector2(l,-1);
		pointCloud.push_back(v);

		Lua::Pop(l,1);
	}

	std::vector<uint16_t> result;
	auto r = Geometry::triangulate(pointCloud,result);
	Lua::PushBool(l,r);
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(result.size()){0};i<result.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushInt(l,result.at(i));
		Lua::SetTableValue(l,t);
	}
	return 2;*/
}

static void splitViaDelaunay(const Vector3 &points,float maxLength)
{

/*
def splitViaDelaunay(points, maxLength):
    from scipy.spatial import Delaunay
    from math import sqrt, ceil
    print "Perform Delaunay triangulation with "+str(len(points))+" points" 
    tri = Delaunay(points)
    # get set of edges from the simpleces
    edges = set()
    for simplex in tri.simplices:
        # simplex is one triangle: [ 4  5 17]
        edges.add((simplex[0], simplex[1]))
        edges.add((simplex[1], simplex[2]))
        edges.add((simplex[0], simplex[2]))
    # check if all edges are small enough
    # and add new points if not
    isFinished = True
    for edge in edges:
        p1, p2 = edge
        [x1, y1] = points[p1]
        [x2, y2] = points[p2]
        length = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))
        if length > maxLength:
            isFinished = False
            # split in how many pieces?
            nPieces = ceil(length/maxLength)
            for piece in range(1, int(nPieces)):
                points.append([x1+piece/float(nPieces)*(x2-x1), y1+piece/float(nPieces)*(y2-y1)])
    if not isFinished:
        splitViaDelaunay(points, maxLength)
		*/
}

#include "pragma/model/modelmesh.h"
extern DLLENGINE Engine *engine;

/*#include "vhcl.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBSceneListener.h>
class MyListener : public SmartBody::SBSceneListener
{
	MyListener ();
	~MyListener ();
	// triggered when a SmartBody character is created
	void OnCharacterCreate( const std::string & name, const std::string & objectClass
	);
	// triggered when a SmartBody character is deleted
	void OnCharacterDelete( const std::string & name );
	// triggered when a SmartBody character changes the skeleton (add/remove joints,
	channels)
	void OnCharacterUpdate( const std::string & name );
	// triggered when an object (character or pawn) is created
	void OnPawnCreate( const std::string & name );
	// triggered when an object (character or pawn) is deleted
	void OnPawnDelete( const std::string & name );
	// triggered when a SmartBody event occurs
	void OnEvent( const std::string & eventName, const std::string & eventParameters );
};
*/
luabind::object Lua::geometry::triangulate(lua_State *l,luabind::table<> tContour)
{
	auto contour = Lua::table_to_vector<Vector2>(l,tContour,1);
	std::vector<uint16_t> result {};
	auto r = ::Geometry::triangulate(contour,result);
	if(r == false)
		return {};
	return Lua::vector_to_table(l,result);
}
