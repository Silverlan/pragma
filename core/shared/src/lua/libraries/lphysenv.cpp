#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/classes/lphysobj.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/raytraces.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/lphyssoftbodyinfo.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/physics/ik/util_ik.hpp"
#include "pragma/buss_ik/Tree.h"
#include "pragma/buss_ik/Jacobian.h"
#include "pragma/physics/ik/ik_controller.hpp"
#include <mathutil/color.h>
#include <luainterface.hpp>
#include <glm/gtx/matrix_decompose.hpp>

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace physenv
	{
		static int raycast(lua_State *l);
		static int sweep(lua_State *l);
		static int overlap(lua_State *l);
		static int create_rigid_body(lua_State *l);
		static int create_convex_hull_shape(lua_State *l);
		static int create_box_shape(lua_State *l);
		static int create_capsule_shape(lua_State *l);
		static int create_sphere_shape(lua_State *l);
		static int create_cylinder_shape(lua_State *l);
		static int create_torus_shape(lua_State *l);
		static int create_heightfield_terrain_shape(lua_State *l);
		static int create_character_controller(lua_State *l);
		static int create_fixed_constraint(lua_State *l);
		static int create_ball_socket_constraint(lua_State *l);
		static int create_hinge_constraint(lua_State *l);
		static int create_slider_constraint(lua_State *l);
		static int create_cone_twist_constraint(lua_State *l);
		static int create_DoF_constraint(lua_State *l);
		static int create_dof_spring_constraint(lua_State *l);
		static int create_surface_material(lua_State *l);
		static int create_ghost_object(lua_State *l);
		static int get_surface_material(lua_State *l);
		static int get_surface_materials(lua_State *l);

		static int calc_torque_from_angular_velocity(lua_State *l);
		static int calc_angular_velocity_from_torque(lua_State *l);
		static int calc_force_from_linear_velocity(lua_State *l);
		static int calc_linear_velocity_from_force(lua_State *l);
	};
};

void Lua::physenv::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	const auto *libName = "phys";
	Lua::RegisterLibrary(l,libName,{
		//{"create_character_controller",create_character_controller},
		{"create_convex_hull_shape",create_convex_hull_shape},
		{"create_box_shape",create_box_shape},
		{"create_capsule_shape",create_capsule_shape},
		{"create_sphere_shape",create_sphere_shape},
		{"create_cylinder_shape",create_cylinder_shape},
		{"create_torus_shape",create_torus_shape},
		{"create_heightfield_terrain_shape",create_heightfield_terrain_shape},
		{"create_rigid_body",create_rigid_body},
		{"create_ghost_object",create_ghost_object},
		{"create_fixed_constraint",create_fixed_constraint},
		{"create_ballsocket_constraint",create_ball_socket_constraint},
		{"create_hinge_constraint",create_hinge_constraint},
		{"create_slider_constraint",create_slider_constraint},
		{"create_conetwist_constraint",create_cone_twist_constraint},
		{"create_dof_constraint",create_DoF_constraint},
		{"create_dof_spring_constraint",create_dof_spring_constraint},
		{"create_surface_material",create_surface_material},
		{"get_surface_material",get_surface_material},
		{"get_surface_materials",get_surface_materials},
		{"raycast",raycast},
		{"sweep",sweep},
		{"overlap",overlap},

		{"calc_torque_from_angular_velocity",calc_torque_from_angular_velocity},
		{"calc_angular_velocity_from_torque",calc_angular_velocity_from_torque},
		{"calc_force_from_linear_velocity",calc_force_from_linear_velocity},
		{"calc_linear_velocity_from_force",calc_linear_velocity_from_force},
	});

	Lua::RegisterLibraryEnums(l,libName,{
		{"ACTIVATION_STATE_ACTIVE_TAG",ACTIVE_TAG},
		{"ACTIVATION_STATE_DISABLE_DEACTIVATION",DISABLE_DEACTIVATION},
		{"ACTIVATION_STATE_DISABLE_SIMULATION",DISABLE_SIMULATION},
		{"ACTIVATION_STATE_ISLAND_SLEEPING",ISLAND_SLEEPING},
		{"ACTIVATION_STATE_WANTS_DEACTIVATION",WANTS_DEACTIVATION}
	});

	Lua::RegisterLibraryEnums(l,libName,{
		{"TYPE_NONE",umath::to_integral(PHYSICSTYPE::NONE)},
		{"TYPE_DYNAMIC",umath::to_integral(PHYSICSTYPE::DYNAMIC)},
		{"TYPE_STATIC",umath::to_integral(PHYSICSTYPE::STATIC)},
		{"TYPE_BOXCONTROLLER",umath::to_integral(PHYSICSTYPE::BOXCONTROLLER)},
		{"TYPE_CAPSULECONTROLLER",umath::to_integral(PHYSICSTYPE::CAPSULECONTROLLER)},
		{"TYPE_SOFTBODY",umath::to_integral(PHYSICSTYPE::SOFTBODY)},

		{"COLLISIONMASK_NONE",umath::to_integral(CollisionMask::None)},
		{"COLLISIONMASK_STATIC",umath::to_integral(CollisionMask::Static)},
		{"COLLISIONMASK_DYNAMIC",umath::to_integral(CollisionMask::Dynamic)},
		{"COLLISIONMASK_GENERIC",umath::to_integral(CollisionMask::Generic)},
		{"COLLISIONMASK_PLAYER",umath::to_integral(CollisionMask::Player)},
		{"COLLISIONMASK_NPC",umath::to_integral(CollisionMask::NPC)},
		{"COLLISIONMASK_VEHICLE",umath::to_integral(CollisionMask::Vehicle)},
		{"COLLISIONMASK_ITEM",umath::to_integral(CollisionMask::Item)},
		{"COLLISIONMASK_ALL",umath::to_integral(CollisionMask::All)},
		{"COLLISIONMASK_DEFAULT",umath::to_integral(CollisionMask::Default)},
		{"COLLISIONMASK_PARTICLE",umath::to_integral(CollisionMask::Particle)},
		{"COLLISIONMASK_CHARACTER",umath::to_integral(CollisionMask::Character)},
		{"COLLISIONMASK_TRIGGER",umath::to_integral(CollisionMask::Trigger)},
		{"COLLISIONMASK_WATER",umath::to_integral(CollisionMask::Water)},
		{"COLLISIONMASK_WATER_SURFACE",umath::to_integral(CollisionMask::WaterSurface)},
		{"COLLISIONMASK_PLAYER_HITBOX",umath::to_integral(CollisionMask::PlayerHitbox)},
		{"COLLISIONMASK_NPC_HITBOX",umath::to_integral(CollisionMask::NPCHitbox)},
		{"COLLISIONMASK_CHARACTER_HITBOX",umath::to_integral(CollisionMask::CharacterHitbox)},
		{"COLLISIONMASK_ALL_HITBOX",umath::to_integral(CollisionMask::AllHitbox)},
		{"COLLISIONMASK_NO_COLLISION",umath::to_integral(CollisionMask::NoCollision)},

		{"FTRACE_ALL_HITS",umath::to_integral(FTRACE::ALL_HITS)},
		{"FTRACE_FILTER_INVERT",umath::to_integral(FTRACE::FILTER_INVERT)},
		{"FTRACE_IGNORE_DYNAMIC",umath::to_integral(FTRACE::IGNORE_DYNAMIC)},
		{"FTRACE_IGNORE_STATIC",umath::to_integral(FTRACE::IGNORE_STATIC)}
	});

	auto &physMod = lua.RegisterLibrary(libName);
	auto classDefRayCastData = luabind::class_<TraceData>("RayCastData");
	classDefRayCastData.def(luabind::constructor<>());
	classDefRayCastData.def("SetSource",static_cast<void(*)(lua_State*,TraceData&,const LPhysShape&)>(&Lua_TraceData_SetSource));
	classDefRayCastData.def("SetSource",static_cast<void(*)(lua_State*,TraceData&,const PhysObjHandle&)>(&Lua_TraceData_SetSource));
	classDefRayCastData.def("SetSource",static_cast<void(TraceData::*)(PhysCollisionObject*)>(&TraceData::SetSource));
	classDefRayCastData.def("SetSource",static_cast<void(TraceData::*)(const Vector3&)>(&TraceData::SetSource));
	classDefRayCastData.def("SetSource",static_cast<void(*)(lua_State*,TraceData&,const EntityHandle&)>(&Lua_TraceData_SetSource));
	classDefRayCastData.def("SetSourceRotation",&TraceData::SetSourceRotation);
	classDefRayCastData.def("SetSource",static_cast<void(TraceData::*)(const PhysTransform&)>(&TraceData::SetSource));
	classDefRayCastData.def("SetTarget",static_cast<void(TraceData::*)(const Vector3&)>(&TraceData::SetTarget));
	classDefRayCastData.def("SetTargetRotation",&TraceData::SetTargetRotation);
	classDefRayCastData.def("SetTarget",static_cast<void(TraceData::*)(const PhysTransform&)>(&TraceData::SetTarget));
	classDefRayCastData.def("SetRotation",&TraceData::SetRotation);
	classDefRayCastData.def("SetFlags",&Lua_TraceData_SetFlags);
	classDefRayCastData.def("SetFilter",&Lua_TraceData_SetFilter);
	classDefRayCastData.def("SetCollisionFilterMask",&Lua_TraceData_SetCollisionFilterMask);
	classDefRayCastData.def("SetCollisionFilterGroup",&Lua_TraceData_SetCollisionFilterGroup);
	classDefRayCastData.def("GetSourceTransform",&Lua_TraceData_GetSourceTransform);
	classDefRayCastData.def("GetTargetTransform",&Lua_TraceData_GetTargetTransform);
	classDefRayCastData.def("GetSourceOrigin",&Lua_TraceData_GetSourceOrigin);
	classDefRayCastData.def("GetTargetOrigin",&Lua_TraceData_GetTargetOrigin);
	classDefRayCastData.def("GetSourceRotation",&Lua_TraceData_GetSourceRotation);
	classDefRayCastData.def("GetTargetRotation",&Lua_TraceData_GetTargetRotation);
	classDefRayCastData.def("GetDistance",&Lua_TraceData_GetDistance);
	classDefRayCastData.def("GetDirection",&Lua_TraceData_GetDirection);
	physMod[classDefRayCastData];

	auto classDefRayCastResult = luabind::class_<TraceResult>("RayCastResult");
	classDefRayCastResult.def_readonly("hit",&TraceResult::hit);
	classDefRayCastResult.property("entity",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		if(tr.entity.IsValid() == false)
			return;
		lua_pushentity(l,tr.entity);
	}));
	classDefRayCastResult.def_readonly("physObj",&TraceResult::physObj);
	classDefRayCastResult.def_readonly("fraction",&TraceResult::fraction);
	classDefRayCastResult.def_readonly("distance",&TraceResult::distance);
	classDefRayCastResult.def_readonly("normal",&TraceResult::normal);
	classDefRayCastResult.def_readonly("position",&TraceResult::position);
	classDefRayCastResult.def_readonly("startPosition",&TraceResult::startPosition);
	classDefRayCastResult.property("colObj",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		if(tr.collisionObj.IsValid() == false)
			return;
		tr.collisionObj->GetLuaObject()->push(l);
	}));
	classDefRayCastResult.property("mesh",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh,&subMesh);

		if(mesh == nullptr)
			return;
		Lua::Push<std::shared_ptr<::ModelMesh>>(l,mesh->shared_from_this());
	}));
	classDefRayCastResult.property("subMesh",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh,&subMesh);

		if(subMesh == nullptr)
			return;
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,subMesh->shared_from_this());
	}));
	classDefRayCastResult.property("material",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		auto *mat = tr.GetMaterial();
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
	}));
	classDefRayCastResult.property("materialName",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		std::string mat;
		if(tr.GetMaterial(mat) == false)
			return;
		Lua::PushString(l,mat);
	}));
	physMod[classDefRayCastResult];

	auto classDefTransform = luabind::class_<PhysTransform>("Transform");
	classDefTransform.def(luabind::constructor<>());
	classDefTransform.def("GetOrigin",&PhysTransform::GetOrigin);
	classDefTransform.def("GetRotation",&PhysTransform::GetRotation);
	classDefTransform.def("SetOrigin",&PhysTransform::SetOrigin);
	classDefTransform.def("SetRotation",&PhysTransform::SetRotation);
	classDefTransform.def("SetIdentity",&PhysTransform::SetIdentity);
	classDefTransform.def("GetInverse",static_cast<void(*)(lua_State*,PhysTransform&)>([](lua_State *l,PhysTransform &t) {
		Lua::Push<PhysTransform>(l,t.GetInverse());
	}));
	classDefTransform.def("GetMatrix",static_cast<void(*)(lua_State*,PhysTransform&)>([](lua_State *l,PhysTransform &t) {
		auto m = umat::identity();
		m = glm::translate(m,t.GetOrigin()) *static_cast<Mat4>(t.GetRotation());
		Lua::Push<Mat4>(l,m);
	}));
	classDefTransform.def("SetMatrix",static_cast<void(*)(lua_State*,PhysTransform&,const Mat4&)>([](lua_State *l,PhysTransform &t,const Mat4 &m) {
		Mat4 transformation;
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::decompose(transformation,scale,rotation,translation,skew,perspective);
		t.SetOrigin(translation);
		t.SetRotation(rotation);
	}));
	classDefTransform.def("GetBasis",static_cast<void(*)(lua_State*,PhysTransform&)>([](lua_State *l,PhysTransform &t) {
		auto &m = t.GetBasis();
		Lua::Push<Mat3*>(l,&m);
	}));
	classDefTransform.def("SetBasis",static_cast<void(*)(lua_State*,PhysTransform&,const Mat3&)>([](lua_State *l,PhysTransform &t,const Mat3 &m) {
		t.SetBasis(m);
	}));
	classDefTransform.def(luabind::const_self *PhysTransform());
	classDefTransform.def(luabind::const_self *Vector3());
	classDefTransform.def(luabind::const_self *Quat());
	physMod[classDefTransform];

	auto classTreeIkTree = luabind::class_<Tree>("IKTree");
	classTreeIkTree.scope[luabind::def("Create",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto tree = std::make_shared<Tree>();
		Lua::Push<std::shared_ptr<Tree>>(l,tree);
	}))];
	classTreeIkTree.def("Draw",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto *game = engine->GetNetworkState(l)->GetGameState();
		auto fGetLocalTransform = [](const Node* node, btTransform& act) {
			btVector3 axis = btVector3(node->v.x, node->v.y, node->v.z);
			btQuaternion rot(0, 0, 0, 1);
			if (axis.length())
			{
				rot = btQuaternion (axis, node->theta);
			}
			act.setIdentity();
			act.setRotation(rot);
			act.setOrigin(btVector3(node->r.x, node->r.y, node->r.z));
		};
		std::function<void(Node*, const btTransform&)> fDrawTree = nullptr;
		fDrawTree = [&fGetLocalTransform,&fDrawTree,game](Node* node, const btTransform& tr) {
			btVector3 lineColor = btVector3(0, 0, 0);
			int lineWidth = 2;
			auto fUpdateLine = [game](int32_t tIdx,const Vector3 &start,const Vector3 &end,const Color &col) {
				/*auto it = m_dbgObjects.find(tIdx);
				if(it == m_dbgObjects.end())
					it = m_dbgObjects.insert(std::make_pair(tIdx,DebugRenderer::DrawLine(start,end,col))).first;
				auto &wo = static_cast<DebugRenderer::WorldObject&>(*m_dbgObjects.at(tIdx));
				wo.GetVertices().at(0) = start;
				wo.GetVertices().at(1) = end;
				wo.UpdateVertexBuffer();*/
				game->DrawLine(start,end,col,0.05f);
				//DebugRenderer::DrawLine(start,end,col,0.05f);
			};
			if (node != 0) {
			//	glPushMatrix();
				btVector3 pos = btVector3(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
				btVector3 color = btVector3(0, 1, 0);
				int pointSize = 10;
				auto enPos = uvec::create(pos /PhysEnv::WORLD_SCALE);
				//auto it = m_dbgObjects.find(0u);
				//if(it == m_dbgObjects.end())
				//	it = m_dbgObjects.insert(std::make_pair(0u,DebugRenderer::DrawPoint(enPos,Color::Lime))).first;
				//it->second->SetPos(enPos);

				auto enForward = uvec::create((tr.getBasis().getColumn(0)));
				auto enRight = uvec::create((tr.getBasis().getColumn(1)));
				auto enUp = uvec::create((tr.getBasis().getColumn(2)));
				fUpdateLine(1,enPos,enPos +enForward *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Red);
				fUpdateLine(2,enPos,enPos +enRight *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Lime);
				fUpdateLine(3,enPos,enPos +enUp *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Aqua);
			
				btVector3 axisLocal = btVector3(node->v.x, node->v.y, node->v.z);
				btVector3 axisWorld = tr.getBasis()*axisLocal;

				fUpdateLine(4,enPos,enPos +0.1f *uvec::create(axisWorld),Color::Yellow);

				//node->DrawNode(node == root);	// Recursively draw node and update ModelView matrix
				if (node->left) {
					btTransform act;
					fGetLocalTransform(node->left, act);
				
					btTransform trl = tr*act;
					auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
					auto trlOrigin = uvec::create(trl.getOrigin() /PhysEnv::WORLD_SCALE);
					fUpdateLine(5,trOrigin,trlOrigin,Color::Maroon);
					fDrawTree(node->left, trl);		// Draw tree of children recursively
				}
			//	glPopMatrix();
				if (node->right) {
					btTransform act;
					fGetLocalTransform(node->right, act);
					btTransform trr = tr*act;
					auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
					auto trrOrigin = uvec::create(trr.getOrigin() /PhysEnv::WORLD_SCALE);
					fUpdateLine(6,trOrigin,trrOrigin,Color::Silver);
					fDrawTree(node->right,trr);		// Draw right siblings recursively
				}
			}
		};
		auto fRenderScene = [&fGetLocalTransform,&fDrawTree](Tree &tree) {
			btTransform act;
			fGetLocalTransform(tree.GetRoot(), act);
			fDrawTree(tree.GetRoot(), act);
		
			//btVector3 pos = btVector3(targetaa[0].x, targetaa[0].y, targetaa[0].z);
			//btQuaternion orn(0, 0, 0, 1);
		};
		fRenderScene(tree);
	}));
	classTreeIkTree.def("GetNodeCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumNode());
	}));
	classTreeIkTree.def("GetEffectorCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumEffector());
	}));
	classTreeIkTree.def("GetJointCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumJoint());
	}));
	classTreeIkTree.def("Compute",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Compute();
	}));
	classTreeIkTree.def("Init",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Init();
	}));
	classTreeIkTree.def("UnFreeze",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.UnFreeze();
	}));
	classTreeIkTree.def("Print",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Print();
	}));
	classTreeIkTree.def("InsertRoot",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		tree.InsertRoot(&node);
	}));
	classTreeIkTree.def("InsertLeftChild",static_cast<void(*)(lua_State*,Tree&,Node&,Node&)>([](lua_State *l,Tree &tree,Node &parent,Node &child) {
		tree.InsertLeftChild(&parent,&child);
	}));
	classTreeIkTree.def("InsertRightSibling",static_cast<void(*)(lua_State*,Tree&,Node&,Node&)>([](lua_State *l,Tree &tree,Node &parent,Node &child) {
		tree.InsertRightSibling(&parent,&child);
	}));
	classTreeIkTree.def("GetJoint",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto *node = tree.GetJoint(nodeIdx);
		if(node == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,node->shared_from_this());
	}));
	classTreeIkTree.def("GetEffector",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto *node = tree.GetEffector(nodeIdx);
		if(node == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,node->shared_from_this());
	}));
	classTreeIkTree.def("GetEffectorPosition",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto &pos = tree.GetEffectorPosition(nodeIdx);
		Lua::Push<Vector3>(l,Vector3(pos.x,pos.y,pos.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}));
	classTreeIkTree.def("GetRoot",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto *root = tree.GetRoot();
		if(root == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,root->shared_from_this());
	}));
	classTreeIkTree.def("GetSuccessor",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		auto *successor = tree.GetSuccessor(&node);
		if(successor == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,successor->shared_from_this());
	}));
	classTreeIkTree.def("GetParent",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		auto *parent = tree.GetParent(&node);
		if(parent == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,parent->shared_from_this());
	}));

	auto classTreeIkTreeNode = luabind::class_<Node>("Node");
	classTreeIkTreeNode.add_static_constant("PURPOSE_JOINT",JOINT);
	classTreeIkTreeNode.add_static_constant("PURPOSE_EFFECTOR",EFFECTOR);
	classTreeIkTreeNode.scope[luabind::def("Create",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,uint32_t,double,double,double)>([](lua_State *l,const Vector3 &origin,const Vector3 &rotAxis,uint32_t purpose,double minTheta,double maxTheta,double restAngle) {
		auto node = std::make_shared<Node>(VectorR3(origin.x,origin.y,origin.z) *PhysEnv::WORLD_SCALE,VectorR3(rotAxis.x,rotAxis.y,rotAxis.z),0.0,static_cast<Purpose>(purpose),minTheta,maxTheta,restAngle);
		Lua::Push<std::shared_ptr<Node>>(l,node);
	}))];
	classTreeIkTreeNode.scope[luabind::def("Create",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,uint32_t)>([](lua_State *l,const Vector3 &origin,const Vector3 &rotAxis,uint32_t purpose) {
		auto node = std::make_shared<Node>(VectorR3(origin.x,origin.y,origin.z) *PhysEnv::WORLD_SCALE,VectorR3(rotAxis.x,rotAxis.y,rotAxis.z),0.0,static_cast<Purpose>(purpose));
		Lua::Push<std::shared_ptr<Node>>(l,node);
	}))];
	classTreeIkTreeNode.def("GetLocalTransform",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		PhysTransform t {};
		util::ik::get_local_transform(node,t);
		Lua::Push<PhysTransform>(l,t);
	}));
	classTreeIkTreeNode.def("PrintNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.PrintNode();
	}));
	classTreeIkTreeNode.def("GetRotationAxis",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &v = node.v;
		Lua::Push<Vector3>(l,Vector3(v.x,v.y,v.z));
	}));
	classTreeIkTreeNode.def("SetRotationAxis",static_cast<void(*)(lua_State*,Node&,const Vector3&)>([](lua_State *l,Node &node,const Vector3 &axis) {
		auto &v = node.v;
		v = VectorR3(axis.x,axis.y,axis.z);
	}));
	classTreeIkTreeNode.def("GetLeftChildNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto *left = node.left;
		if(left == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,left->shared_from_this());
	}));
	classTreeIkTreeNode.def("GetRightChildNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto *right = node.right;
		if(right == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,right->shared_from_this());
	}));
	classTreeIkTreeNode.def("InitNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.InitNode();
	}));
	classTreeIkTreeNode.def("GetAttach",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &r = node.GetAttach();
		Lua::Push<Vector3>(l,Vector3(r.x,r.y,r.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}));
	classTreeIkTreeNode.def("SetAttach",static_cast<void(*)(lua_State*,Node&,const Vector3&)>([](lua_State *l,Node &node,const Vector3 &attach) {
		node.attach = VectorR3(attach.x,attach.y,attach.z) *PhysEnv::WORLD_SCALE;
	}));
	classTreeIkTreeNode.def("GetRelativePosition",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &r = node.r;
		Lua::Push<Vector3>(l,Vector3(r.x,r.y,r.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}));
	classTreeIkTreeNode.def("GetTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto theta = node.GetTheta();
		Lua::PushNumber(l,theta);
	}));
	classTreeIkTreeNode.def("AddToTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double delta) {
		auto r = node.AddToTheta(delta);
		Lua::PushNumber(l,r);
	}));
	classTreeIkTreeNode.def("UpdateTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double delta) {
		auto r = node.UpdateTheta(delta);
		Lua::PushNumber(l,r);
	}));
	classTreeIkTreeNode.def("GetS",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &s = node.GetS();
		Lua::Push<Vector3>(l,Vector3(s.x,s.y,s.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}));
	classTreeIkTreeNode.def("GetW",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &w = node.GetW();
		Lua::Push<Vector3>(l,Vector3(w.x,w.y,w.z) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}));
	classTreeIkTreeNode.def("GetMinTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto minTheta = node.GetMinTheta();
		Lua::PushNumber(l,minTheta);
	}));
	classTreeIkTreeNode.def("GetMaxTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto maxTheta = node.GetMaxTheta();
		Lua::PushNumber(l,maxTheta);
	}));
	classTreeIkTreeNode.def("GetRestAngle",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto restAngle = node.GetRestAngle();
		Lua::PushNumber(l,restAngle);
	}));
	classTreeIkTreeNode.def("SetTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double theta) {
		node.SetTheta(theta);
	}));
	classTreeIkTreeNode.def("ComputeS",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.ComputeS();
	}));
	classTreeIkTreeNode.def("ComputeW",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.ComputeW();
	}));
	classTreeIkTreeNode.def("IsEffector",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsEffector());
	}));
	classTreeIkTreeNode.def("IsJoint",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsJoint());
	}));
	classTreeIkTreeNode.def("GetEffectorIndex",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushInt(l,node.GetEffectorNum());
	}));
	classTreeIkTreeNode.def("GetJointIndex",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushInt(l,node.GetJointNum());
	}));
	classTreeIkTreeNode.def("IsFrozen",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsFrozen());
	}));
	classTreeIkTreeNode.def("Freeze",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.Freeze();
	}));
	classTreeIkTreeNode.def("UnFreeze",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.UnFreeze();
	}));

	auto classIkJacobian = luabind::class_<Jacobian>("IKJacobian");
	classIkJacobian.scope[luabind::def("Create",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto jacobian = std::make_shared<Jacobian>(&tree);
		Lua::Push<std::shared_ptr<Jacobian>>(l,jacobian);
	}))];
	classIkJacobian.def("ComputeJacobian",static_cast<void(*)(lua_State*,Jacobian&,luabind::object)>([](lua_State *l,Jacobian &jacobian,luabind::object o) {
		Lua::CheckTable(l,2);
		auto numTargets = Lua::GetObjectLength(l,2);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(auto i=decltype(numTargets){0};i<numTargets;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,2);
			auto *v = Lua::CheckVector(l,-1);
			targets.push_back(VectorR3(v->x,v->y,v->z) *PhysEnv::WORLD_SCALE);
			Lua::Pop(l,1);
		}
		jacobian.ComputeJacobian(targets.data());
	}));
	classIkJacobian.def("SetJendActive",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.SetJendActive();
	}));
	classIkJacobian.def("SetJtargetActive",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.SetJtargetActive();
	}));
	//classIkJacobian.def("SetJendTrans",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetJendTrans();
	//}));
	//classIkJacobian.def("SetDeltaS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian,const Vector3 &s) {
	//	jacobian.SetDeltaS();
	//}));
	classIkJacobian.def("CalcDeltaThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetas();
	}));
	classIkJacobian.def("ZeroDeltaThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.ZeroDeltaThetas();
	}));
	classIkJacobian.def("CalcDeltaThetasTranspose",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasTranspose();
	}));
	classIkJacobian.def("CalcDeltaThetasPseudoinverse",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasPseudoinverse();
	}));
	classIkJacobian.def("CalcDeltaThetasDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasDLS();
	}));
	//classIkJacobian.def("CalcDeltaThetasDLS2",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLS2();
	//}));
	classIkJacobian.def("CalcDeltaThetasDLSwithSVD",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasDLSwithSVD();
	}));
	classIkJacobian.def("CalcDeltaThetasSDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasSDLS();
	}));
	//classIkJacobian.def("CalcDeltaThetasDLSwithNullspace",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLSwithNullspace();
	//}));
	classIkJacobian.def("UpdateThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.UpdateThetas();
	}));
	classIkJacobian.def("UpdateThetaDot",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.UpdateThetaDot();
	}));
	//classIkJacobian.def("UpdateErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.UpdateErrorArray();
	//}));
	//classIkJacobian.def("GetErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.GetErrorArray();
	//}));
	classIkJacobian.def("UpdatedSClampValue",static_cast<void(*)(lua_State*,Jacobian&,luabind::object)>([](lua_State *l,Jacobian &jacobian,luabind::object o) {
		Lua::CheckTable(l,2);
		auto numTargets = Lua::GetObjectLength(l,2);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(auto i=decltype(numTargets){0};i<numTargets;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,2);
			auto *v = Lua::CheckVector(l,-1);
			targets.push_back(VectorR3(v->x,v->y,v->z) *PhysEnv::WORLD_SCALE);
			Lua::Pop(l,1);
		}
		jacobian.UpdatedSClampValue(targets.data());
	}));
	//classIkJacobian.def("SetCurrentMode",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetCurrentMode();
	//}));
	//classIkJacobian.def("GetCurrentMode",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.GetCurrentMode();
	//}));
	//classIkJacobian.def("SetDampingDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetDampingDLS();
	//}));
	classIkJacobian.def("Reset",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.Reset();
	}));
	/*classIkJacobian.def("CompareErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CompareErrors();
	}));
	classIkJacobian.def("CountErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CountErrors();
	}));*/
	classIkJacobian.def("GetRowCount",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		Lua::PushInt(l,jacobian.GetNumRows());
	}));
	classIkJacobian.def("GetColumnCount",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		Lua::PushInt(l,jacobian.GetNumCols());
	}));
	physMod[classIkJacobian];

	classTreeIkTree.scope[classTreeIkTreeNode];
	physMod[classTreeIkTree];

	auto classIkController = luabind::class_<IKController>("IKController");
	classIkController.def("GetEffectorName",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushString(l,ikController.GetEffectorName());
	}));
	classIkController.def("GetChainLength",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushInt(l,ikController.GetChainLength());
	}));
	classIkController.def("GetType",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushString(l,ikController.GetType());
	}));
	classIkController.def("SetEffectorName",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &effectorName) {
		ikController.SetEffectorName(effectorName);
	}));
	classIkController.def("SetChainLength",static_cast<void(*)(lua_State*,IKController&,uint32_t)>([](lua_State *l,IKController &ikController,uint32_t chainLength) {
		ikController.SetChainLength(chainLength);
	}));
	classIkController.def("SetType",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &type) {
		ikController.SetType(type);
	}));
	classIkController.def("SetMethod",static_cast<void(*)(lua_State*,IKController&,uint32_t)>([](lua_State *l,IKController &ikController,uint32_t method) {
		ikController.SetMethod(static_cast<util::ik::Method>(method));
	}));
	classIkController.def("GetMethod",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushInt(l,ikController.GetMethod());
	}));
	classIkController.def("GetKeyValues",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		auto &ikKeyValues = ikController.GetKeyValues();
		auto t = Lua::CreateTable(l);
		for(auto &pair : ikKeyValues)
		{
			Lua::PushString(l,pair.first);
			Lua::PushString(l,pair.second);
			Lua::SetTableValue(l,t);
		}
	}));
	classIkController.def("SetKeyValues",static_cast<void(*)(lua_State*,IKController&,luabind::object)>([](lua_State *l,IKController &ikController,luabind::object o) {
		Lua::CheckTable(l,2);
		auto &ikKeyValues = ikController.GetKeyValues();
		ikKeyValues.clear();
		ikKeyValues.reserve(Lua::GetObjectLength(l,2));

		Lua::PushNil(l);
		while(Lua::GetNextPair(l,2) != 0)
		{
			auto *key = Lua::CheckString(l,-2);
			auto *val = Lua::CheckString(l,-1);
			ikKeyValues[key] = val;
			Lua::Pop(l,1);
		}
	}));
	classIkController.def("SetKeyValue",static_cast<void(*)(lua_State*,IKController&,const std::string&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &key,const std::string &value) {
		auto &ikKeyValues = ikController.GetKeyValues();
		ikKeyValues[key] = value;
	}));
	classIkController.def("GetKeyValue",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &key) {
		auto &ikKeyValues = ikController.GetKeyValues();
		auto it = ikKeyValues.find(key);
		if(it == ikKeyValues.end())
			return;
		Lua::PushString(l,it->second);
	}));
	classIkController.add_static_constant("METHOD_SELECTIVELY_DAMPED_LEAST_SQUARE",umath::to_integral(util::ik::Method::SelectivelyDampedLeastSquare));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES",umath::to_integral(util::ik::Method::DampedLeastSquares));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES_WITH_SINGULAR_VALUE_DECOMPOSITION",umath::to_integral(util::ik::Method::DampedLeastSquaresWithSingularValueDecomposition));
	classIkController.add_static_constant("METHOD_PSEUDOINVERSE",umath::to_integral(util::ik::Method::Pseudoinverse));
	classIkController.add_static_constant("METHOD_JACOBIAN_TRANSPOSE",umath::to_integral(util::ik::Method::JacobianTranspose));
	classIkController.add_static_constant("METHOD_DEFAULT",umath::to_integral(util::ik::Method::Default));
	physMod[classIkController];

	auto classDef = luabind::class_<::PhysSoftBodyInfo>("SoftBodyInfo");
	Lua::PhysSoftBodyInfo::register_class(l,classDef);
	physMod[classDef];

	Lua::PhysConstraint::register_class(l,physMod);
	Lua::PhysCollisionObj::register_class(l,physMod);
	Lua::PhysObj::register_class(l,physMod);
	Lua::PhysContact::register_class(l,physMod);
	Lua::PhysShape::register_class(l,physMod);
	Lua::PhysKinematicCharacterController::register_class(l,physMod);
}
int Lua::physenv::raycast(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<TraceResult> res;
	auto r = game->RayCast(*data,&res);
	if(r == false || res.empty())
	{
		Lua::PushBool(l,r);
		return 1;
	}
	auto table = Lua::CreateTable(l);
	if(data->HasFlag(FTRACE::ALL_HITS))
	{
		for(size_t i=0;i<res.size();i++)
		{
			auto &r = res[i];
			Lua_TraceData_FillTraceResultTable(l,r);
			lua_rawseti(l,table,i +1);
		}
	}
	else
		Lua_TraceData_FillTraceResultTable(l,res[0]);
	return 1;
}
int Lua::physenv::sweep(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	TraceResult res;
	auto r = game->Sweep(*data,&res);
	if(r == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	Lua_TraceData_FillTraceResultTable(l,res);
	return 1;
}
int Lua::physenv::overlap(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	TraceResult res;
	auto r = game->Overlap(*data,&res);
	if(r == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	Lua_TraceData_FillTraceResultTable(l,res);
	return 1;
}
int Lua::physenv::create_convex_hull_shape(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	env->CreateConvexHullShape()->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_box_shape(lua_State *l)
{
	auto *halfExtents = Lua::CheckVector(l,1);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto shape = env->CreateBoxShape(*halfExtents);
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_capsule_shape(lua_State *l)
{
	auto halfWidth = Lua::CheckNumber(l,1);
	auto halfHeight = Lua::CheckNumber(l,2);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto shape = env->CreateCapsuleShape(CFloat(halfWidth),CFloat(halfHeight));
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_sphere_shape(lua_State *l)
{
	auto radius = Lua::CheckNumber(l,1);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto shape = env->CreateSphereShape(CFloat(radius));
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_cylinder_shape(lua_State *l)
{
	auto radius = Lua::CheckNumber(l,1);
	auto height = Lua::CheckNumber(l,2);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto shape = env->CreateCylinderShape(radius,height);
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_torus_shape(lua_State *l)
{
	auto subDivisions = Lua::CheckInt(l,1);
	auto outerRadius = Lua::CheckNumber(l,2);
	auto innerRadius = Lua::CheckNumber(l,3);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto shape = env->CreateTorusShape(subDivisions,outerRadius,innerRadius);
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_heightfield_terrain_shape(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();

	auto width = Lua::CheckInt(l,1);
	auto length = Lua::CheckInt(l,2);
	auto maxHeight = Lua::CheckNumber(l,3);
	auto upAxis = Lua::CheckInt(l,4);

	auto shape = env->CreateHeightfieldTerrainShape(width,length,maxHeight,upAxis);
	shape->GetLuaObject(l).push(l);
	return 1;
}

int Lua::physenv::create_rigid_body(lua_State *l)
{
	auto mass = Lua::CheckNumber(l,1);
	auto *shape = Lua::CheckPhysShape(l,2);
	auto *inertia = Lua::CheckVector(l,3);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto *body = env->CreateRigidBody(CFloat(mass),shape->GetSharedPointer(),*inertia);
	if(body == nullptr)
		return 0;
	Lua::PushObject(l,body);
	return 1;
}

int Lua::physenv::create_ghost_object(lua_State *l)
{
	std::shared_ptr<::PhysShape> shape = nullptr;
	if(Lua::IsSet(l,1) == true)
		shape = Lua::CheckPhysShape(l,1)->GetSharedPointer();

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto *ghost = env->CreateGhostObject(shape);
	if(ghost == nullptr)
		return 0;
	Lua::PushObject(l,ghost);
	return 1;
}

int Lua::physenv::create_fixed_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto *bodyB = Lua::CheckPhysRigidBody(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateFixedConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,*rotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*rotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_ball_socket_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *bodyB = Lua::CheckPhysRigidBody(l,3);
	auto *pivotB = Lua::CheckVector(l,4);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateBallSocketConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_hinge_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *bodyB = Lua::CheckPhysRigidBody(l,3);
	auto *pivotB = Lua::CheckVector(l,4);
	auto *axis = Lua::CheckVector(l,5);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateHingeConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*axis
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_slider_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto *bodyB = Lua::CheckPhysRigidBody(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateSliderConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,*rotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*rotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_cone_twist_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto *bodyB = Lua::CheckPhysRigidBody(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateConeTwistConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,*rotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*rotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_DoF_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto *bodyB = Lua::CheckPhysRigidBody(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateDoFConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,*rotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*rotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_dof_spring_constraint(lua_State *l)
{
	auto *bodyA = Lua::CheckPhysRigidBody(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto *bodyB = Lua::CheckPhysRigidBody(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	auto constraint = env->CreateDoFSpringConstraint(
		static_cast<::PhysRigidBody*>(bodyA->get()),*pivotA,*rotA,
		static_cast<::PhysRigidBody*>(bodyB->get()),*pivotB,*rotB
	);
	Lua::PushConstraint(l,constraint);
	return 1;
}

int Lua::physenv::create_surface_material(lua_State *l)
{
	auto name = Lua::CheckString(l,1);
	auto friction = Lua::CheckNumber(l,2);
	auto restitution = Lua::CheckNumber(l,3);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &mat = game->CreateSurfaceMaterial(name,CFloat(friction),CFloat(restitution));
	Lua::Push<SurfaceMaterial*>(l,&mat);
	return 1;
}
int Lua::physenv::get_surface_material(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	SurfaceMaterial *mat = nullptr;
	if(Lua::IsNumber(l,1))
	{
		auto id = Lua::CheckInt(l,1);
		mat = game->GetSurfaceMaterial(CUInt32(id));
	}
	else
	{
		auto name = Lua::CheckString(l,1);
		mat = game->GetSurfaceMaterial(name);
	}
	if(mat == nullptr)
		return 0;
	Lua::Push<SurfaceMaterial*>(l,mat);
	return 1;
}
int Lua::physenv::get_surface_materials(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &mats = game->GetSurfaceMaterials();
	Lua::CreateTable(l);
	auto idxTable = Lua::GetStackTop(l);
	for(UInt i=0;i<mats.size();i++)
	{
		auto &mat = mats[i];
		Lua::PushInt(l,i +1);
		Lua::Push<SurfaceMaterial*>(l,&mat);
		Lua::SetTableValue(l,idxTable);
	}
	return 1;
}

int Lua::physenv::create_character_controller(lua_State*)
{
	/*
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(0.f,0.f,0.f));
	btPairCachingGhostObject *ghost = new btPairCachingGhostObject;
	ghost->setWorldTransform(startTransform);

	btCapsuleShape *shape = new btCapsuleShape(width,height);
	ghost->setCollisionShape(shape);
	ghost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	float stepHeight = 24.f;
	PhysKinematicCharacterController *controller = new PhysKinematicCharacterController(ghost,shape,stepHeight);
	controller->setGravity(0.f);
	*/
	/*PhysEnv *physEnv = game->GetPhysicsEnvironment();
	physEnv->addCollisionObject(ghost,btBroadphaseProxy::CharacterFilter,btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	physEnv->addAction(controller);

	Lua::Push<PhysKinematicCharacterController*>(l,controller);*/
	return 0;
}

int Lua::physenv::calc_torque_from_angular_velocity(lua_State *l)
{
	auto &angVel = *Lua::CheckVector(l,1);
	auto &invInertiaTensor = *Lua::CheckMat3(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto torque = angVel /static_cast<float>(dt);
	torque = glm::inverse(invInertiaTensor) *torque;
	Lua::Push<Vector3>(l,torque);
	return 1;
}
int Lua::physenv::calc_angular_velocity_from_torque(lua_State *l)
{
	auto &torque = *Lua::CheckVector(l,1);
	auto &invInertiaTensor = *Lua::CheckMat3(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto angVel = torque *invInertiaTensor *static_cast<float>(dt);
	Lua::Push<Vector3>(l,angVel);
	return 1;
}
int Lua::physenv::calc_force_from_linear_velocity(lua_State *l)
{
	auto &linVel = *Lua::CheckVector(l,1);
	auto mass = Lua::CheckNumber(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto force = (static_cast<float>(mass) *linVel) /static_cast<float>(dt);
	Lua::Push<Vector3>(l,force);
	return 1;
}
int Lua::physenv::calc_linear_velocity_from_force(lua_State *l)
{
	auto &force = *Lua::CheckVector(l,1);
	auto mass = Lua::CheckNumber(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto linVel = (force *static_cast<float>(dt)) /static_cast<float>(mass);
	Lua::Push<Vector3>(l,linVel);
	return 1;
}
