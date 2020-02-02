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

int Lua::geometry::closest_point_on_aabb_to_point(lua_State *l)
{
	Vector3 *min = _lua_Vector_check(l,1);
	Vector3 *max = _lua_Vector_check(l,2);
	Vector3 *p = _lua_Vector_check(l,3);
	Vector3 res;
	Geometry::ClosestPointOnAABBToPoint(*min,*max,*p,&res);
	luabind::object(l,res).push(l);
	return 1;
}

int Lua::geometry::closest_points_between_lines(lua_State *l)
{
	Vector3 *pA = _lua_Vector_check(l,1);
	Vector3 *qA = _lua_Vector_check(l,2);
	Vector3 *pB = _lua_Vector_check(l,3);
	Vector3 *qB = _lua_Vector_check(l,4);
	float s,t;
	Vector3 cA,cB;
	float d = Geometry::ClosestPointsBetweenLines(*pA,*qA,*pB,*qB,&s,&t,&cA,&cB);
	luabind::object(l,cA).push(l);
	luabind::object(l,cB).push(l);
	Lua::PushNumber(l,d);
	return 3;
}

int Lua::geometry::closest_point_on_plane_to_point(lua_State *l)
{
	Vector3 *n = _lua_Vector_check(l,1);
	float d = Lua::CheckNumber<float>(l,2);
	Vector3 *p = _lua_Vector_check(l,3);
	Vector3 res;
	Geometry::ClosestPointOnPlaneToPoint(*n,d,*p,&res);
	luabind::object(l,res).push(l);
	return 1;
}

int Lua::geometry::closest_point_on_triangle_to_point(lua_State *l)
{
	Vector3 *a = _lua_Vector_check(l,1);
	Vector3 *b = _lua_Vector_check(l,2);
	Vector3 *c = _lua_Vector_check(l,3);
	Vector3 *p = _lua_Vector_check(l,4);
	Vector3 res;
	Geometry::ClosestPointOnTriangleToPoint(*a,*b,*c,*p,&res);
	luabind::object(l,res).push(l);
	return 1;
}

int Lua::geometry::smallest_enclosing_sphere(lua_State *l)
{
	std::vector<Vector3> verts;

	Lua::CheckTable(l,1);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,1) != 0)
	{
		Vector3 *v = Lua::CheckVector(l,-1);
		verts.push_back(*v);
		Lua::Pop(l,1);
	}

	Vector3 center;
	float radius;
	Seb::Calculate(verts,center,radius);
	Lua::Push<Vector3>(l,center);
	Lua::PushNumber(l,radius);
	return 2;
}

int Lua::geometry::closest_point_on_line_to_point(lua_State *l)
{
	auto *start = Lua::CheckVector(l,1);
	auto *end = Lua::CheckVector(l,2);
	auto *p = Lua::CheckVector(l,3);
	auto bClampResultToSegment = true;
	if(Lua::IsSet(l,4))
		bClampResultToSegment = Lua::CheckBool(l,4);
	auto r = Geometry::ClosestPointOnLineToPoint(*start,*end,*p,bClampResultToSegment);
	Lua::Push<Vector3>(l,r);
	return 1;
}

int Lua::geometry::closest_point_on_sphere_to_line(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto radius = Lua::CheckNumber(l,2);
	auto *start = Lua::CheckVector(l,3);
	auto *end = Lua::CheckVector(l,4);
	auto bClampResultToSegment = true;
	if(Lua::IsSet(l,5))
		bClampResultToSegment = Lua::CheckBool(l,5);
	auto r = Geometry::ClosestPointOnSphereToLine(*origin,static_cast<float>(radius),*start,*end,bClampResultToSegment);
	Lua::Push<Vector3>(l,r);
	return 1;
}

int Lua::geometry::get_triangle_winding_order(lua_State *l)
{
	if(Lua::IsVector(l,1))
	{
		auto &v0 = *Lua::CheckVector(l,1);
		auto &v1 = *Lua::CheckVector(l,2);
		auto &v2 = *Lua::CheckVector(l,3);
		auto &n = *Lua::CheckVector(l,4);
		auto windingOrder = ::Geometry::get_triangle_winding_order(v0,v1,v2,n);
		Lua::PushInt(l,umath::to_integral(windingOrder));
		return 1;
	}
	Lua::CheckVector2(l,1);
	auto &v0 = *Lua::CheckVector2(l,1);
	auto &v1 = *Lua::CheckVector2(l,2);
	auto &v2 = *Lua::CheckVector2(l,3);
	auto windingOrder = ::Geometry::get_triangle_winding_order(v0,v1,v2);
	Lua::PushInt(l,umath::to_integral(windingOrder));
	return 1;
}

int Lua::geometry::calc_face_normal(lua_State *l)
{
	auto &v0 = *Lua::CheckVector(l,1);
	auto &v1 = *Lua::CheckVector(l,2);
	auto &v2 = *Lua::CheckVector(l,3);
	auto n = Geometry::CalcFaceNormal(v0,v1,v2);
	Lua::Push<Vector3>(l,n);
	return 1;
}

int Lua::geometry::generate_truncated_cone_mesh(lua_State *l)
{
	auto &origin = *Lua::CheckVector(l,1);
	auto startRadius = Lua::CheckNumber(l,2);
	auto &dir = *Lua::CheckVector(l,3);
	auto dist = Lua::CheckNumber(l,4);
	auto endRadius = Lua::CheckNumber(l,5);
	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	std::vector<Vector3> normals;
	uint32_t segmentCount = 12;
	if(Lua::IsSet(l,6))
		segmentCount = static_cast<uint32_t>(Lua::CheckInt(l,6));

	auto bCaps = true;
	if(Lua::IsSet(l,7))
		bCaps = Lua::CheckBool(l,7);

	auto bGenerateTriangles = true;
	if(Lua::IsSet(l,8))
		bGenerateTriangles = Lua::CheckBool(l,8);

	auto bGenerateNormals = false;
	if(Lua::IsSet(l,9))
		bGenerateNormals = Lua::CheckBool(l,9);
	Geometry::GenerateTruncatedConeMesh(origin,static_cast<float>(startRadius),dir,static_cast<float>(dist),static_cast<float>(endRadius),verts,(bGenerateTriangles == true) ? &triangles : nullptr,(bGenerateNormals == true) ? &normals : nullptr,segmentCount,bCaps);

	int32_t numRet = 1;
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(verts.size()){0};i<verts.size();++i)
	{
		auto &v = verts[i];
		Lua::PushInt(l,i +1);
		Lua::Push<Vector3>(l,v);
		Lua::SetTableValue(l,t);
	}

	if(bGenerateTriangles == true)
	{
		t = Lua::CreateTable(l);
		for(auto i=decltype(triangles.size()){0};i<triangles.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::PushInt(l,triangles[i]);
			Lua::SetTableValue(l,t);
		}
		++numRet;
	}
	if(bGenerateNormals == true)
	{
		t = Lua::CreateTable(l);
		for(auto i=decltype(normals.size()){0};i<normals.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<Vector3>(l,normals[i]);
			Lua::SetTableValue(l,t);
		}
		++numRet;
	}
	return numRet;
}

int Lua::geometry::calc_volume_of_triangle(lua_State *l)
{
	auto *v0 = Lua::CheckVector(l,1);
	auto *v1 = Lua::CheckVector(l,2);
	auto *v2 = Lua::CheckVector(l,3);
	auto vol = ::Geometry::calc_volume_of_triangle(*v0,*v1,*v2);
	Lua::PushNumber(l,vol);
	return 1;
}
int Lua::geometry::calc_volume_of_polyhedron(lua_State *l)
{
	int32_t tVerts = 1;
	Lua::CheckTable(l,tVerts);
	int32_t tTriangles = tVerts +1;
	Lua::CheckTable(l,tTriangles);

	Lua::PushNil(l);
	auto volume = Geometry::calc_volume_of_polyhedron([l,tVerts,tTriangles](const Vector3 **v0,const Vector3 **v1,const Vector3 **v2) mutable -> bool {
		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v0Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v1Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v2Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v0Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v0 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v1Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v1 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v2Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v2 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);
		return true;
	});
	Lua::PushNumber(l,volume);
	return 1;
}
int Lua::geometry::calc_center_of_mass(lua_State *l)
{
	int32_t tVerts = 1;
	Lua::CheckTable(l,tVerts);
	int32_t tTriangles = tVerts +1;
	Lua::CheckTable(l,tTriangles);

	Lua::PushNil(l);
	Vector3 com {};
	auto volume = Geometry::calc_volume_of_polyhedron([l,tVerts,tTriangles](const Vector3 **v0,const Vector3 **v1,const Vector3 **v2) mutable -> bool {
		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v0Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v1Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		if(Lua::GetNextPair(l,tTriangles) == 0)
			return false;
		auto v2Idx = Lua::CheckInt(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v0Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v0 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v1Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v1 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);

		Lua::PushInt(l,v2Idx +1);
		Lua::GetTableValue(l,tVerts);
		*v2 = Lua::CheckVector(l,-1);
		Lua::Pop(l,1);
		return true;
	},&com);
	Lua::Push<Vector3>(l,com);
	Lua::PushNumber(l,volume);
	return 2;
}
int Lua::geometry::calc_triangle_area(lua_State *l)
{
	auto &p0 = Lua::Check<Vector3>(l,1);
	auto &p1 = Lua::Check<Vector3>(l,2);
	auto &p2 = Lua::Check<Vector3>(l,3);
	auto keepSign = false;
	if(Lua::IsSet(l,4))
		keepSign = Lua::CheckBool(l,4);
	Lua::PushNumber(l,Geometry::calc_triangle_area(p0,p1,p2,keepSign));
	return 1;
}
int Lua::geometry::calc_barycentric_coordinates(lua_State *l)
{
	float b1,b2;
	auto r = false;
	int32_t idx = 1;
	if(Lua::IsVector2(l,2) == true)
	{
		auto &p0 = *Lua::CheckVector(l,idx++);
		auto &uv0 = *Lua::CheckVector2(l,idx++);
		auto &p1 = *Lua::CheckVector(l,idx++);
		auto &uv1 = *Lua::CheckVector2(l,idx++);
		auto &p2 = *Lua::CheckVector(l,idx++);
		auto &uv2 = *Lua::CheckVector2(l,idx++);
		auto &hitPoint = *Lua::CheckVector(l,idx++);
		r = ::Geometry::calc_barycentric_coordinates(p0,uv0,p1,uv1,p2,uv2,hitPoint,b1,b2);
	}
	else
	{
		auto &p0 = *Lua::CheckVector(l,idx++);
		auto &p1 = *Lua::CheckVector(l,idx++);
		auto &p2 = *Lua::CheckVector(l,idx++);
		auto &hitPoint = *Lua::CheckVector(l,idx++);
		r = ::Geometry::calc_barycentric_coordinates(p0,p1,p2,hitPoint,b1,b2);
	}
	Lua::Push<Vector2>(l,Vector2{b1,b2});
	return 1;
}
int Lua::geometry::calc_rotation_between_planes(lua_State *l)
{
	int32_t idx = 1;
	auto &n0 = *Lua::CheckVector(l,idx++);
	auto &n1 = *Lua::CheckVector(l,idx++);
	auto rot = ::Geometry::calc_rotation_between_planes(n0,n1);
	Lua::Push<Quat>(l,rot);
	return 1;
}

int Lua::geometry::get_side_of_point_to_line(lua_State *l)
{
	auto &lineStart = *Lua::CheckVector2(l,1);
	auto &lineEnd = *Lua::CheckVector2(l,2);
	auto &p = *Lua::CheckVector2(l,3);
	auto side = ::Geometry::get_side_of_point_to_line(lineStart,lineEnd,p);
	Lua::PushInt(l,umath::to_integral(side));
	return 1;
}

int Lua::geometry::get_side_of_point_to_plane(lua_State *l)
{
	auto &n = *Lua::CheckVector(l,1);
	auto d = Lua::CheckNumber(l,2);
	auto &p = *Lua::CheckVector(l,3);
	auto planeSide = ::Geometry::get_side_of_point_to_plane(n,d,p);
	Lua::PushInt(l,umath::to_integral(planeSide));
	return 1;
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
int Lua::geometry::triangulate(lua_State *l)
{
	std::vector<Vector2> contour {};
	std::vector<uint16_t> result {};
	auto tContour = 1;
	Lua::CheckTable(l,tContour);
	auto numVerts = Lua::GetObjectLength(l,tContour);
	contour.reserve(numVerts);
	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tContour);
		auto &v = *Lua::CheckVector2(l,-1);
		contour.push_back(v);
		Lua::Pop(l,1);
	}
	auto r = ::Geometry::triangulate(contour,result);
	if(r == false)
		return 0;
	auto tResult = Lua::CreateTable(l);
	auto i = 1;
	for(auto idx : result)
	{
		Lua::PushInt(l,i++);
		Lua::PushInt(l,idx);
		Lua::SetTableValue(l,tResult);
	}
	return 1;
	/*MyListener* myListener = new MyListener();
	// get the SmartBody scene
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	// add the listener to the scene
	scene->addSceneListener(myListener);
	//scene->update(currentTime);

	// creates a character with a skeleton that has a single joint
	SmartBody::SBCharacter* character = scene->createCharacter("thename", "thetype");
	// creates a skeleton from an existing asset
	SmartBody::SBSkeleton* skeleton = scene->createSkeleton("mycharacter.dae");
	// attaches the skeleton to the character
	character->setSkeleton(skeleton);
	// creates a standard set of controllers such as animation, gazing, head movements,
	etc.
	character->createStandardControllers();

	SmartBody::SBSkeleton* dynamicSkeleton = scene->addSkeletonDefinition("myskeleton");
	SmartBody::SBJoint* baseJoint = skeleton->createJoint("base", NULL);
	SmartBody::SBJoint* joint1 = skeleton->createJoint("child", baseJoint);
	SmartBody::SBJoint* joint2 = skeleton->createJoint("grandchild", joint1);
	...
	// then later, create the character and a new instance of this skeleton
	SmartBody::SBSkeleton* skeleton = scene->createSkeleton("myskeleton");
	character->setSkeleton(skeleton);
	*/
	/*auto mesh = pinocchio::Mesh("C:\\Users\\Florian\\Documents\\Projects\\pinocchio\\meshes\\Model1.obj");
	auto skeleton = pinocchio::HumanSkeleton();
	auto po = pinocchio::autorig(skeleton,mesh);

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
	auto &meshVerts = subMesh->GetVertices();
	auto &meshWeights = subMesh->GetVertexWeights();
	meshVerts.reserve(mesh.vertices.size());
	meshWeights.reserve(mesh.vertices.size());
	for(auto &v : mesh.vertices)
	{
		meshVerts.push_back({});
		auto &mv = meshVerts.back();
		mv.position = {v.pos[0],v.pos[1],v.pos[2]};
		mv.normal = {v.normal[0],v.normal[1],v.normal[2]};
	}
	auto &meshTris = subMesh->GetTriangles();
	meshTris.reserve(mesh.vertices.size());
	for(auto i=decltype(mesh.vertices.size()){0};i<mesh.vertices.size();++i)
	{
		meshTris.push_back(i);
		auto weights = po.attachment->getWeights(0);
		meshWeights.push_back({});
		auto &vw = meshWeights.back();
		vw.weights = {weights[0],weights[1],weights[2],weights[3]};
	}
	Lua::Push<std::shared_ptr<ModelSubMesh>>(l,subMesh);*/
	//return 0;
}
