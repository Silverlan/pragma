/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/modelmesh.h"

extern DLLNETWORK Engine *engine;

namespace Lua
{
	namespace PhysShape
	{
		static void GetBounds(lua_State *l,pragma::physics::IShape &shape);
		static void IsConvex(lua_State *l,pragma::physics::IShape &shape);
		static void IsConvexHull(lua_State *l,pragma::physics::IShape &shape);
		static void IsHeightField(lua_State *l,pragma::physics::IShape &shape);
		static void IsTriangleShape(lua_State *l,pragma::physics::IShape &shape);
	};
	namespace PhysConvexShape
	{
		static void GetCollisionMesh(lua_State *l,pragma::physics::IConvexShape &shape);
	};
	namespace PhysConvexHullShape
	{
		static void AddPoint(lua_State *l,pragma::physics::IConvexHullShape &shape,Vector3 &point);
	};
	namespace PhysHeightfield
	{
		static void GetHeight(lua_State *l,pragma::physics::IHeightfield &shape,uint32_t x,uint32_t y);
		static void SetHeight(lua_State *l,pragma::physics::IHeightfield &shape,uint32_t x,uint32_t y,float height);
		static void GetWidth(lua_State *l,pragma::physics::IHeightfield &shape);
		static void GetLength(lua_State *l,pragma::physics::IHeightfield &shape);
		static void GetMaxHeight(lua_State *l,pragma::physics::IHeightfield &shape);
		static void GetUpAxis(lua_State *l,pragma::physics::IHeightfield &shape);
	};
};

void Lua::PhysShape::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::IShape,pragma::physics::IBase>("Shape");
	classDef.def("GetBounds",&GetBounds);
	classDef.def("IsConvex",&IsConvex);
	classDef.def("IsConvexHull",&IsConvexHull);
	classDef.def("IsHeightfield",&IsHeightField);
	classDef.def("IsTriangleShape",&IsTriangleShape);
	classDef.def("SetMass",static_cast<void(*)(lua_State*,pragma::physics::IShape&,float)>([](lua_State *l,pragma::physics::IShape &shape,float mass) {
		shape.SetMass(mass);
	}));
	classDef.def("GetMass",static_cast<void(*)(lua_State*,pragma::physics::IShape&)>([](lua_State *l,pragma::physics::IShape &shape) {
		Lua::PushNumber(l,shape.GetMass());
	}));
	classDef.def("CalculateLocalInertia",static_cast<void(*)(lua_State*,pragma::physics::IShape&,float)>([](lua_State *l,pragma::physics::IShape &shape,float mass) {
		Vector3 localInertia;
		shape.CalculateLocalInertia(mass,&localInertia);
		Lua::Push<Vector3>(l,localInertia);
	}));
	mod[classDef];

	auto convexClassDef = luabind::class_<pragma::physics::IConvexShape,luabind::bases<pragma::physics::IShape,pragma::physics::IBase>>("ConvexShape");
	convexClassDef.def("GetCollisionMesh",PhysConvexShape::GetCollisionMesh);
	mod[convexClassDef];

	auto capsuleShapeDef = luabind::class_<pragma::physics::ICapsuleShape,luabind::bases<pragma::physics::IConvexShape,pragma::physics::IShape,pragma::physics::IBase>>("CapsuleShape");
	capsuleShapeDef.def("GetRadius",static_cast<void(*)(lua_State*,pragma::physics::ICapsuleShape&)>([](lua_State *l,pragma::physics::ICapsuleShape &shape) {
		Lua::PushNumber(l,shape.GetRadius());
	}));
	capsuleShapeDef.def("GetHalfHeight",static_cast<void(*)(lua_State*,pragma::physics::ICapsuleShape&)>([](lua_State *l,pragma::physics::ICapsuleShape &shape) {
		Lua::PushNumber(l,shape.GetHalfHeight());
	}));
	mod[capsuleShapeDef];

	auto boxShapeDef = luabind::class_<pragma::physics::IBoxShape,luabind::bases<pragma::physics::IConvexShape,pragma::physics::IShape,pragma::physics::IBase>>("BoxShape");
	boxShapeDef.def("GetHalfExtents",static_cast<void(*)(lua_State*,pragma::physics::IBoxShape&)>([](lua_State *l,pragma::physics::IBoxShape &shape) {
		Lua::Push<Vector3>(l,shape.GetHalfExtents());
	}));
	mod[boxShapeDef];

	auto compoundShapeDef = luabind::class_<pragma::physics::ICompoundShape,luabind::bases<pragma::physics::IShape,pragma::physics::IBase>>("CompoundShape");
	mod[compoundShapeDef];

	auto hullClassDef = luabind::class_<pragma::physics::IConvexHullShape,luabind::bases<pragma::physics::IConvexShape,pragma::physics::IShape,pragma::physics::IBase>>("ConvexHullShape");
	hullClassDef.def("AddPoint",PhysConvexHullShape::AddPoint);
	hullClassDef.def("AddTriangle",static_cast<void(*)(lua_State*,pragma::physics::IConvexHullShape&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,pragma::physics::IConvexHullShape &shape,uint32_t idx0,uint32_t idx1,uint32_t idx2) {
		shape.AddTriangle(idx0,idx1,idx2);
	}));
	hullClassDef.def("ReservePoints",static_cast<void(*)(lua_State*,pragma::physics::IConvexHullShape&,uint32_t)>([](lua_State *l,pragma::physics::IConvexHullShape &shape,uint32_t numPoints) {
		shape.ReservePoints(numPoints);
	}));
	hullClassDef.def("ReserveTriangles",static_cast<void(*)(lua_State*,pragma::physics::IConvexHullShape&,uint32_t)>([](lua_State *l,pragma::physics::IConvexHullShape &shape,uint32_t numTris) {
		shape.ReserveTriangles(numTris);
	}));
	hullClassDef.def("Build",static_cast<void(*)(lua_State*,pragma::physics::IConvexHullShape&)>([](lua_State *l,pragma::physics::IConvexHullShape &shape) {
		shape.Build();
	}));
	mod[hullClassDef];

	auto heightfieldClassDef = luabind::class_<pragma::physics::IHeightfield,luabind::bases<pragma::physics::IShape,pragma::physics::IBase>>("Heightfield");
	heightfieldClassDef.def("GetHeight",PhysHeightfield::GetHeight);
	heightfieldClassDef.def("SetHeight",PhysHeightfield::SetHeight);
	heightfieldClassDef.def("GetWidth",PhysHeightfield::GetWidth);
	heightfieldClassDef.def("GetLength",PhysHeightfield::GetLength);
	heightfieldClassDef.def("GetMaxHeight",PhysHeightfield::GetMaxHeight);
	heightfieldClassDef.def("GetUpAxis",PhysHeightfield::GetUpAxis);
	mod[heightfieldClassDef];

	auto triangleShapeClassDef = luabind::class_<pragma::physics::ITriangleShape,luabind::bases<pragma::physics::IShape,pragma::physics::IBase>>("TriangleShape");
#if 0
	// Bullet triangle mesh deformation
	// This is unstable and can cause the game to crash
	triangleShapeClassDef.def("Test",static_cast<void(*)(lua_State*,LPhysTriangleShape&,EntityHandle&,PhysRigidBodyHandle&,std::shared_ptr<::ModelSubMesh>&,const Vector3&,float,float)>([](lua_State *l,LPhysTriangleShape &shape,EntityHandle &hEnt,PhysRigidBodyHandle &hBody,std::shared_ptr<::ModelSubMesh> &subMesh,const Vector3 &origin,float radius,float power) {
		auto *iva = static_cast<PhysTriangleShape*>(shape.get())->GetBtIndexVertexArray();
		if(iva == nullptr)
			return;
		uint8_t *vertexBase = nullptr;
		int32_t numVerts = 0;
		int32_t vertexStride = 0;
		PHY_ScalarType scalarType {};
		uint8_t *indexBase = nullptr;
		int32_t indexStride = 0;
		int32_t numFaces = 0;
		PHY_ScalarType indexType {};
		int32_t subPart = 0;
		iva->getLockedVertexIndexBase(&vertexBase,numVerts,scalarType,vertexStride,&indexBase,indexStride,numFaces,indexType,subPart);
		
		auto offset = 0u;
		for(auto i=decltype(numVerts){0};i<numVerts;++i)
		{
			switch(scalarType)
			{
				case PHY_ScalarType::PHY_FLOAT:
				{
					auto *v = reinterpret_cast<float*>(vertexBase +i *vertexStride);

					auto d = uvec::distance(origin,Vector3(v[0],v[1],v[2]) /static_cast<float>(PhysEnv::WORLD_SCALE));
					if(d < radius)
					{
						v[1] += (power *(1.f -(d /radius))) *PhysEnv::WORLD_SCALE;
					}
					//v[1] += 20.f *PhysEnv::WORLD_SCALE;
					break;
				}
				case PHY_ScalarType::PHY_DOUBLE:
				{
					auto *v = reinterpret_cast<double*>(vertexBase +i *vertexStride);

					auto d = uvec::distance(origin,Vector3(v[0],v[1],v[2]) /static_cast<float>(PhysEnv::WORLD_SCALE));
					if(d < radius)
					{
						v[1] += (power *(1.f -(d /radius))) *PhysEnv::WORLD_SCALE;
					}
					//v[1] += 20.0 *PhysEnv::WORLD_SCALE;
					break;
				}
			}
		}

		iva->unLockVertexBase(subPart);
		/*auto &verts = static_cast<PhysTriangleShape*>(shape.get())->GetVertices();
		for(auto &v : verts)
		{
			auto d = uvec::distance(origin,Vector3(v[0],v[1],v[2]) /static_cast<float>(PhysEnv::WORLD_SCALE));
			if(d < radius)
			{
				v[1] += (power *(1.f -(d /radius))) *PhysEnv::WORLD_SCALE;
			}
		}*/
		static_cast<PhysTriangleShape*>(shape.get())->GenerateInternalEdgeInfo();

		static_cast<PhysTriangleShape*>(shape.get())->Build();
		if(hBody.IsValid())
		{
			//PhysEnv *world = hEnt->GetNetworkState()->GetGameState()->GetPhysicsEnvironment();
			//auto *colObj = hBody->GetCollisionObject();
			//world->GetWorld()->removeCollisionObject(colObj);
			auto ptr = shape.GetSharedPointer();
			hBody->SetCollisionShape(ptr);
			//world->GetWorld()->addCollisionObject(colObj,umath::to_integral(hBody->GetCollisionFilterGroup()),umath::to_integral(hBody->GetCollisionFilterMask()));
		}

		auto &meshVerts = subMesh->GetVertices();
		for(auto &v : meshVerts)
		{
			auto d = uvec::distance(origin,v.position);
			if(d < radius)
			{
				v.position.y += (power *(1.f -(d /radius)));
			}
		}
		subMesh->Update(ModelUpdateFlags::UpdateVertexBuffer);
	}));
#endif
	mod[triangleShapeClassDef];
}
void Lua::PhysShape::GetBounds(lua_State *l,pragma::physics::IShape &shape)
{
	Vector3 min,max;
	shape.GetAABB(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::PhysShape::IsConvex(lua_State *l,pragma::physics::IShape &shape)
{
	Lua::PushBool(l,shape.IsConvex());
}
void Lua::PhysShape::IsConvexHull(lua_State *l,pragma::physics::IShape &shape)
{
	Lua::PushBool(l,shape.IsConvexHull());
}
void Lua::PhysShape::IsHeightField(lua_State *l,pragma::physics::IShape &shape)
{
	Lua::PushBool(l,shape.IsHeightfield());
}
void Lua::PhysShape::IsTriangleShape(lua_State *l,pragma::physics::IShape &shape)
{
	Lua::PushBool(l,shape.IsTriangleShape());
}

///////////////////////////////

void Lua::PhysConvexShape::GetCollisionMesh(lua_State *l,pragma::physics::IConvexShape &shape)
{
	auto *colMesh = shape.GetCollisionMesh();
	if(colMesh == nullptr)
		return;
	Lua::Push<std::shared_ptr<::CollisionMesh>>(l,colMesh->shared_from_this());
}

///////////////////////////////

void Lua::PhysConvexHullShape::AddPoint(lua_State*,pragma::physics::IConvexHullShape &shape,Vector3 &point)
{
	shape.AddPoint(point);
}

///////////////////////////////

void Lua::PhysHeightfield::GetHeight(lua_State *l,pragma::physics::IHeightfield &shape,uint32_t x,uint32_t y)
{
	Lua::PushInt(l,shape.GetHeight(x,y));
}
void Lua::PhysHeightfield::SetHeight(lua_State *l,pragma::physics::IHeightfield &shape,uint32_t x,uint32_t y,float height)
{
	shape.SetHeight(x,y,height);
}
void Lua::PhysHeightfield::GetWidth(lua_State *l,pragma::physics::IHeightfield &shape)
{
	Lua::PushInt(l,shape.GetWidth());
}
void Lua::PhysHeightfield::GetLength(lua_State *l,pragma::physics::IHeightfield &shape)
{
	Lua::PushInt(l,shape.GetLength());
}
void Lua::PhysHeightfield::GetMaxHeight(lua_State *l,pragma::physics::IHeightfield &shape)
{
	Lua::PushNumber(l,shape.GetMaxHeight());
}
void Lua::PhysHeightfield::GetUpAxis(lua_State *l,pragma::physics::IHeightfield &shape)
{
	Lua::PushInt(l,shape.GetUpAxis());
}
