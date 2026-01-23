// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.physics;

void Lua::PhysShape::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::IShape, pragma::physics::IBase>("Shape");
	classDef.def(
	  "GetBounds", +[](pragma::physics::IShape &shape) -> std::pair<Vector3, Vector3> {
		  Vector3 min, max;
		  shape.GetAABB(min, max);
		  return {min, max};
	  });
	classDef.def("IsConvex", &pragma::physics::IShape::IsConvex);
	classDef.def("IsConvexHull", &pragma::physics::IShape::IsConvexHull);
	classDef.def("IsHeightfield", &pragma::physics::IShape::IsHeightfield);
	classDef.def("IsTriangleShape", &pragma::physics::IShape::IsTriangleShape);
	classDef.def("SetMass", &pragma::physics::IShape::SetMass);
	classDef.def("GetMass", &pragma::physics::IShape::GetMass);
	classDef.def("SetLocalPose", &pragma::physics::IShape::SetLocalPose);
	classDef.def("GetLocalPose", &pragma::physics::IShape::GetLocalPose);
	classDef.def("CalculateLocalInertia", &pragma::physics::IShape::CalculateLocalInertia, luabind::out_value<3> {});
	mod[classDef];

	auto convexClassDef = luabind::class_<pragma::physics::IConvexShape, luabind::bases<pragma::physics::IShape, pragma::physics::IBase>>("ConvexShape");
	convexClassDef.def("GetCollisionMesh", static_cast<pragma::physics::CollisionMesh *(pragma::physics::IConvexShape::*)()>(&pragma::physics::IConvexShape::GetCollisionMesh), luabind::shared_from_this_policy<0> {});
	mod[convexClassDef];

	auto capsuleShapeDef = luabind::class_<pragma::physics::ICapsuleShape, luabind::bases<pragma::physics::IConvexShape, pragma::physics::IShape, pragma::physics::IBase>>("CapsuleShape");
	capsuleShapeDef.def("GetRadius", &pragma::physics::ICapsuleShape::GetRadius);
	capsuleShapeDef.def("GetHalfHeight", &pragma::physics::ICapsuleShape::GetHalfHeight);
	mod[capsuleShapeDef];

	auto boxShapeDef = luabind::class_<pragma::physics::IBoxShape, luabind::bases<pragma::physics::IConvexShape, pragma::physics::IShape, pragma::physics::IBase>>("BoxShape");
	boxShapeDef.def("GetHalfExtents", &pragma::physics::IBoxShape::GetHalfExtents);
	mod[boxShapeDef];

	auto compoundShapeDef = luabind::class_<pragma::physics::ICompoundShape, luabind::bases<pragma::physics::IShape, pragma::physics::IBase>>("CompoundShape");
	mod[compoundShapeDef];

	auto hullClassDef = luabind::class_<pragma::physics::IConvexHullShape, luabind::bases<pragma::physics::IConvexShape, pragma::physics::IShape, pragma::physics::IBase>>("ConvexHullShape");
	hullClassDef.def("AddPoint", &pragma::physics::IConvexHullShape::AddPoint);
	hullClassDef.def("AddTriangle", &pragma::physics::IConvexHullShape::AddTriangle);
	hullClassDef.def("ReservePoints", &pragma::physics::IConvexHullShape::ReservePoints);
	hullClassDef.def("ReserveTriangles", &pragma::physics::IConvexHullShape::ReserveTriangles);
	hullClassDef.def("Build", &pragma::physics::IConvexHullShape::Build);
	mod[hullClassDef];

	auto heightfieldClassDef = luabind::class_<pragma::physics::IHeightfield, luabind::bases<pragma::physics::IShape, pragma::physics::IBase>>("Heightfield");
	heightfieldClassDef.def("GetHeight", &pragma::physics::IHeightfield::GetHeight);
	heightfieldClassDef.def("SetHeight", &pragma::physics::IHeightfield::SetHeight);
	heightfieldClassDef.def("GetWidth", &pragma::physics::IHeightfield::GetWidth);
	heightfieldClassDef.def("GetLength", &pragma::physics::IHeightfield::GetLength);
	heightfieldClassDef.def("GetMaxHeight", &pragma::physics::IHeightfield::GetMaxHeight);
	heightfieldClassDef.def("GetUpAxis", &pragma::physics::IHeightfield::GetUpAxis);
	mod[heightfieldClassDef];

	auto triangleShapeClassDef = luabind::class_<pragma::physics::ITriangleShape, luabind::bases<pragma::physics::IShape, pragma::physics::IBase>>("TriangleShape");
#if 0
	// Bullet triangle mesh deformation
	// This is unstable and can cause the game to crash
	triangleShapeClassDef.def("Test",static_cast<void(*)(lua::State*,LPhysTriangleShape&,EntityHandle&,PhysRigidBodyHandle&,std::shared_ptr<pragma::geometry::ModelSubMesh>&,const Vector3&,float,float)>([](lua::State *l,LPhysTriangleShape &shape,EntityHandle &hEnt,PhysRigidBodyHandle &hBody,std::shared_ptr<pragma::geometry::ModelSubMesh> &subMesh,const Vector3 &origin,float radius,float power) {
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
			//world->GetWorld()->addCollisionObject(colObj,pragma::math::to_integral(hBody->GetCollisionFilterGroup()),pragma::math::to_integral(hBody->GetCollisionFilterMask()));
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
		subMesh->Update(pragma::asset::ModelUpdateFlags::UpdateVertexBuffer);
	}));
#endif
	mod[triangleShapeClassDef];
}
