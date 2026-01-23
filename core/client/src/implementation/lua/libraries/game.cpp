// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

// #define ENABLE_DEPRECATED_PHYSICS

#ifdef ENABLE_DEPRECATED_PHYSICS
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#endif

#ifdef ENABLE_DEPRECATED_PHYSICS
#include <Bullet3Common/b3AlignedObjectArray.h>
#endif

module pragma.client;

import :scripting.lua.libraries.game;
import :client_state;
import :core;
import :debug;
import :engine;
import :entities.components;
import :game;
import :scripting.lua;

#ifdef ENABLE_DEPRECATED_PHYSICS
static btSoftBody *createSoftBody(btSoftRigidDynamicsWorld *world, btSoftBodyWorldInfo *info, const btScalar s, const int numX, const int numY, const int fixed)
{
	btSoftBody *cloth = btSoftBodyHelpers::CreatePatch(*info, btVector3(-s / 2, s + 1, 0), btVector3(+s / 2, s + 1, 0), btVector3(-s / 2, s + 1, +s), btVector3(+s / 2, s + 1, +s), numX, numY, fixed, true);
	cloth->getCollisionShape()->setMargin(0.001f);
	cloth->generateBendingConstraints(2, cloth->appendMaterial());
	cloth->setTotalMass(10);
	cloth->m_cfg.piterations = 5;
	cloth->m_cfg.kDP = 0.005f;
	world->addSoftBody(cloth);
	return cloth;
}
#endif

#ifdef ENABLE_DEPRECATED_PHYSICS
static void update_vehicle(Vehicle_Car *vhc)
{
	vhc->ControlInput(-1);
	auto &t = vhc->Chassis->getWorldTransform();
	auto &linVel = vhc->Chassis->getLinearVelocity();
	auto origin = uvec::create(t.getOrigin() / PhysEnv::WORLD_SCALE);
	auto axis = uvec::create(t.getRotation().getAxis());
	uvec::normalize(&axis);
	pragma::get_cgame()->DrawLine(origin, origin + axis * 400.f, colors::Red, 0.5f);
}
#endif

enum Method { IK_JACOB_TRANS = 0, IK_PURE_PSEUDO, IK_DLS, IK_SDLS, IK_DLS_SVD };

static void get_local_bone_position(const std::function<pragma::math::Transform(uint32_t)> &fGetTransform, std::shared_ptr<pragma::animation::Bone> &bone, const Vector3 &fscale = {1.f, 1.f, 1.f}, Vector3 *pos = nullptr, Quat *rot = nullptr, Vector3 *scale = nullptr)
{
	std::function<void(std::shared_ptr<pragma::animation::Bone> &, Vector3 *, Quat *, Vector3 *)> apply;
	apply = [fGetTransform, &apply, fscale](std::shared_ptr<pragma::animation::Bone> &bone, Vector3 *pos, Quat *rot, Vector3 *scale) {
		auto parent = bone->parent.lock();
		if(parent != nullptr)
			apply(parent, pos, rot, scale);
		auto tParent = fGetTransform(bone->ID);
		auto &posParent = tParent.GetOrigin();
		auto &rotParent = tParent.GetRotation();
		auto inv = uquat::get_inverse(rotParent);
		if(pos != nullptr) {
			*pos -= posParent * fscale;
			uvec::rotate(pos, inv);
		}
		if(rot != nullptr)
			*rot = inv * (*rot);
	};
	auto parent = bone->parent.lock();
	if(parent != nullptr)
		apply(parent, pos, rot, scale);
}
static void get_local_bone_position(const std::shared_ptr<pragma::asset::Model> &mdl, const std::function<pragma::math::Transform(uint32_t)> &fGetTransform, std::shared_ptr<pragma::animation::Bone> &bone, const Vector3 &fscale = {1.f, 1.f, 1.f}, Vector3 *pos = nullptr, Quat *rot = nullptr,
  Vector3 *scale = nullptr)
{
	get_local_bone_position(fGetTransform, bone, fscale, pos, rot, scale);
	if(rot == nullptr)
		return;
	auto anim = mdl->GetAnimation(0);
	if(anim != nullptr) {
		auto frame = anim->GetFrame(0);
		if(frame != nullptr) {
			auto *frameRot = frame->GetBoneOrientation(0);
			if(frameRot != nullptr)
				*rot *= *frameRot;
		}
	}
}

#ifdef ENABLE_DEPRECATED_PHYSICS
// Source: BenchmarkDemo.cpp from Bullet source code
class RagDoll {
  public:
	enum {
		BODYPART_PELVIS = 0,
		BODYPART_SPINE,
		BODYPART_HEAD,

		BODYPART_LEFT_UPPER_LEG,
		BODYPART_LEFT_LOWER_LEG,

		BODYPART_RIGHT_UPPER_LEG,
		BODYPART_RIGHT_LOWER_LEG,

		BODYPART_LEFT_UPPER_ARM,
		BODYPART_LEFT_LOWER_ARM,

		BODYPART_RIGHT_UPPER_ARM,
		BODYPART_RIGHT_LOWER_ARM,

		BODYPART_COUNT
	};

	enum {
		JOINT_PELVIS_SPINE = 0,
		JOINT_SPINE_HEAD,

		JOINT_LEFT_HIP,
		JOINT_LEFT_KNEE,

		JOINT_RIGHT_HIP,
		JOINT_RIGHT_KNEE,

		JOINT_LEFT_SHOULDER,
		JOINT_LEFT_ELBOW,

		JOINT_RIGHT_SHOULDER,
		JOINT_RIGHT_ELBOW,

		JOINT_COUNT
	};

	btDynamicsWorld *m_ownerWorld;
	std::shared_ptr<PhysConvexShape> m_shapes[BODYPART_COUNT];
	PhysCollisionObjectHandle m_bodies[BODYPART_COUNT];
	PhysConstraint *m_joints[JOINT_COUNT];

	PhysCollisionObjectHandle createRigidBody(btScalar mass, const btTransform &startTransform, std::shared_ptr<PhysConvexShape> &shape)
	{
		bool isDynamic = (mass != 0.f);

		Vector3 localInertia(0, 0, 0);
		if(isDynamic)
			shape->CalculateLocalInertia(mass, &localInertia);

		auto *body = pragma::get_cgame()->GetPhysicsEnvironment()->CreateRigidBody(mass, shape, localInertia);

		body->Spawn();
		body->SetCollisionFilterGroup(pragma::physics::CollisionMask::Dynamic | pragma::physics::CollisionMask::Generic);
		body->SetCollisionFilterMask(pragma::physics::CollisionMask::All);
		return body->GetHandle();
	}
  public:
	RagDoll(btDynamicsWorld *ownerWorld, const btVector3 &positionOffset, btScalar scale) : m_ownerWorld(ownerWorld)
	{
		// Setup the geometry
		m_shapes[BODYPART_PELVIS] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.15) * scale, btScalar(0.20) * scale));
		m_shapes[BODYPART_SPINE] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.15) * scale, btScalar(0.28) * scale));
		m_shapes[BODYPART_HEAD] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.10) * scale, btScalar(0.05) * scale));
		m_shapes[BODYPART_LEFT_UPPER_LEG] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.07) * scale, btScalar(0.45) * scale));
		m_shapes[BODYPART_LEFT_LOWER_LEG] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.05) * scale, btScalar(0.37) * scale));
		m_shapes[BODYPART_RIGHT_UPPER_LEG] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.07) * scale, btScalar(0.45) * scale));
		m_shapes[BODYPART_RIGHT_LOWER_LEG] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.05) * scale, btScalar(0.37) * scale));
		m_shapes[BODYPART_LEFT_UPPER_ARM] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.05) * scale, btScalar(0.33) * scale));
		m_shapes[BODYPART_LEFT_LOWER_ARM] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.04) * scale, btScalar(0.25) * scale));
		m_shapes[BODYPART_RIGHT_UPPER_ARM] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.05) * scale, btScalar(0.33) * scale));
		m_shapes[BODYPART_RIGHT_LOWER_ARM] = pragma::get_cgame()->GetPhysicsEnvironment()->CreateConvexShape(new btCapsuleShape(btScalar(0.04) * scale, btScalar(0.25) * scale));

		// Setup all the rigid bodies
		btTransform offset;
		offset.setIdentity();
		offset.setOrigin(positionOffset);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.), btScalar(1.), btScalar(0.)));
		m_bodies[BODYPART_PELVIS] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_PELVIS]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.), btScalar(1.2), btScalar(0.)));
		m_bodies[BODYPART_SPINE] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_SPINE]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.), btScalar(1.6), btScalar(0.)));
		m_bodies[BODYPART_HEAD] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_HEAD]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(-0.18), btScalar(0.65), btScalar(0.)));
		m_bodies[BODYPART_LEFT_UPPER_LEG] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_LEFT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(-0.18), btScalar(0.2), btScalar(0.)));
		m_bodies[BODYPART_LEFT_LOWER_LEG] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_LEFT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.18), btScalar(0.65), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_UPPER_LEG] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.18), btScalar(0.2), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_LOWER_LEG] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(-0.35), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		m_bodies[BODYPART_LEFT_UPPER_ARM] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_LEFT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(-0.7), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		m_bodies[BODYPART_LEFT_LOWER_ARM] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_LEFT_LOWER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.35), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, -pragma::math::pi_2);
		m_bodies[BODYPART_RIGHT_UPPER_ARM] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * btVector3(btScalar(0.7), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, -pragma::math::pi_2);
		m_bodies[BODYPART_RIGHT_LOWER_ARM] = createRigidBody(btScalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_LOWER_ARM]);

		// Setup some damping on the m_bodies
		for(int i = 0; i < BODYPART_COUNT; ++i) {
			auto *pRigidBody = static_cast<PhysRigidBody *>(m_bodies[i].get());
			pRigidBody->SetDamping(btScalar(0.05), btScalar(0.85));
			pRigidBody->GetRigidBody()->setDeactivationTime(btScalar(0.8));
			pRigidBody->SetSleepingThresholds(btScalar(1.6), btScalar(2.5));
		}

		// Now setup the constraints
		PhysHinge *hingeC;
		PhysConeTwist *coneC;
		auto *physEnv = pragma::get_cgame()->GetPhysicsEnvironment();

		btTransform localA, localB;

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.15), btScalar(0.)));

		std::vector<btRigidBody *> rigidBodies;
		for(auto &hBody : m_bodies)
			rigidBodies.push_back(static_cast<PhysRigidBody *>(hBody.get())->GetRigidBody());

		hingeC = physEnv->AddHingeConstraint(new btHingeConstraint(*rigidBodies[BODYPART_PELVIS], *rigidBodies[BODYPART_SPINE], localA, localB));
		hingeC->SetLimit(btScalar(-pragma::math::pi_4), btScalar(pragma::math::pi_2));
		m_joints[JOINT_PELVIS_SPINE] = hingeC;
		hingeC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.30), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		coneC = physEnv->AddConeTwistConstraint(new btConeTwistConstraint(*rigidBodies[BODYPART_SPINE], *rigidBodies[BODYPART_HEAD], localA, localB));
		coneC->SetLimit(pragma::math::pi_4, pragma::math::pi_4, pragma::math::pi_2);
		m_joints[JOINT_SPINE_HEAD] = coneC;
		coneC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, -pragma::math::pi_4 * 5);
		localA.setOrigin(scale * btVector3(btScalar(-0.18), btScalar(-0.10), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, -pragma::math::pi_4 * 5);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
		coneC = physEnv->AddConeTwistConstraint(new btConeTwistConstraint(*rigidBodies[BODYPART_PELVIS], *rigidBodies[BODYPART_LEFT_UPPER_LEG], localA, localB));
		coneC->SetLimit(pragma::math::pi_4, pragma::math::pi_4, 0);
		m_joints[JOINT_LEFT_HIP] = coneC;
		coneC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
		hingeC = physEnv->AddHingeConstraint(new btHingeConstraint(*rigidBodies[BODYPART_LEFT_UPPER_LEG], *rigidBodies[BODYPART_LEFT_LOWER_LEG], localA, localB));
		hingeC->SetLimit(btScalar(0), btScalar(pragma::math::pi_2));
		m_joints[JOINT_LEFT_KNEE] = hingeC;
		hingeC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, pragma::math::pi_4);
		localA.setOrigin(scale * btVector3(btScalar(0.18), btScalar(-0.10), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, pragma::math::pi_4);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
		coneC = physEnv->AddConeTwistConstraint(new btConeTwistConstraint(*rigidBodies[BODYPART_PELVIS], *rigidBodies[BODYPART_RIGHT_UPPER_LEG], localA, localB));
		coneC->SetLimit(pragma::math::pi_4, pragma::math::pi_4, 0);
		m_joints[JOINT_RIGHT_HIP] = coneC;
		coneC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
		hingeC = physEnv->AddHingeConstraint(new btHingeConstraint(*rigidBodies[BODYPART_RIGHT_UPPER_LEG], *rigidBodies[BODYPART_RIGHT_LOWER_LEG], localA, localB));
		hingeC->SetLimit(btScalar(0), btScalar(pragma::math::pi_2));
		m_joints[JOINT_RIGHT_KNEE] = hingeC;
		hingeC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, pragma::math::pi);
		localA.setOrigin(scale * btVector3(btScalar(-0.2), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
		coneC = physEnv->AddConeTwistConstraint(new btConeTwistConstraint(*rigidBodies[BODYPART_SPINE], *rigidBodies[BODYPART_LEFT_UPPER_ARM], localA, localB));
		coneC->SetLimit(pragma::math::pi_2, pragma::math::pi_2, 0);
		m_joints[JOINT_LEFT_SHOULDER] = coneC;
		coneC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		hingeC = physEnv->AddHingeConstraint(new btHingeConstraint(*rigidBodies[BODYPART_LEFT_UPPER_ARM], *rigidBodies[BODYPART_LEFT_LOWER_ARM], localA, localB));
		hingeC->SetLimit(btScalar(-pragma::math::pi_2), btScalar(0));
		m_joints[JOINT_LEFT_ELBOW] = hingeC;
		hingeC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.2), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, pragma::math::pi_2);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
		coneC = physEnv->AddConeTwistConstraint(new btConeTwistConstraint(*rigidBodies[BODYPART_SPINE], *rigidBodies[BODYPART_RIGHT_UPPER_ARM], localA, localB));
		coneC->SetLimit(pragma::math::pi_2, pragma::math::pi_2, 0);
		m_joints[JOINT_RIGHT_SHOULDER] = coneC;
		coneC->DisableCollisions();

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localA.setOrigin(scale * btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
		localB.getBasis().setEulerZYX(0, pragma::math::pi_2, 0);
		localB.setOrigin(scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		hingeC = physEnv->AddHingeConstraint(new btHingeConstraint(*rigidBodies[BODYPART_RIGHT_UPPER_ARM], *rigidBodies[BODYPART_RIGHT_LOWER_ARM], localA, localB));
		hingeC->SetLimit(btScalar(-pragma::math::pi_2), btScalar(0));
		m_joints[JOINT_RIGHT_ELBOW] = hingeC;
		hingeC->DisableCollisions();
	}

	virtual ~RagDoll() {}
};

int Lua::game::Client::test(lua::State *l)
{
	/*auto &point = pragma::get_cgame()->GetLocalPlayer()->GetEntity().GetPosition();
	pragma::OcclusionCullingHandlerBSP ocHandler {};
	std::vector<pragma::OcclusionMeshInfo> renderMeshes {};
	ocHandler.PerformCulling(*pragma::get_cgame()->GetScene(),renderMeshes);
	auto *pNode = ocHandler.FindLeafNode(point);*/
	/*auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &node : ocHandler.GetBSPTree().GetNodes())
	{
		Lua::PushInt(l,idx++);
		auto t2 = Lua::CreateTable(l);
		Lua::PushInt(l,1);
		Lua::Push<Vector3>(l,node->min);
		Lua::SetTableValue(l,t2);

		Lua::PushInt(l,2);
		Lua::Push<Vector3>(l,node->max);
		Lua::SetTableValue(l,t2);
		Lua::SetTableValue(l,t);
	}*/
	//if(pNode != nullptr)
	//{
	//Lua::Push<Vector3>(l,pNode->min);
	//	//Lua::Push<Vector3>(l,pNode->max);
	//	return 2;
	//}
	if(true)
		return 0;

	auto *world = pragma::get_cgame()->GetPhysicsEnvironment()->GetWorld();
	auto offset = btVector3 {0.f, 0.f, 0.f};
	auto scale = 3.5;
	/*auto *ragdoll = new RagDoll(world,offset,scale);

	auto cbThink = pragma::get_cgame()->AddCallback("Think",FunctionCallback<>::Create([ragdoll]() {
		static auto f = Vector3{0.f,-600.f,0.f};
		for(auto &hBody : ragdoll->m_bodies)
		{
			if(hBody.IsValid() == false)
				continue;
			auto localForce = f *static_cast<PhysRigidBody*>(hBody.get())->GetMass();
			static_cast<PhysRigidBody*>(hBody.get())->ApplyForce(localForce);
		}
	}));*/

	auto mdl = pragma::get_cgame()->LoadModel("breen.wmd");
	static std::vector<PhysRigidBody *> rigidBodies {};
	if(rigidBodies.empty() == true) {
		if(mdl != nullptr) {
			std::vector<uint32_t> boneIds {};
			auto meshId = 0u;
			for(auto &colMesh : mdl->GetCollisionMeshes()) {
				auto shape = colMesh->GetShape();
				auto mass = mdl->GetMass();
				Vector3 localInertia(0, 0, 0);
				shape->CalculateLocalInertia(mass, &localInertia);

				auto *body = pragma::get_cgame()->GetPhysicsEnvironment()->CreateRigidBody(mass, shape, localInertia);

				body->Spawn();
				body->SetPos(colMesh->GetOrigin());
				body->SetCollisionFilterGroup(pragma::physics::CollisionMask::Dynamic | pragma::physics::CollisionMask::Generic);
				body->SetCollisionFilterMask(pragma::physics::CollisionMask::All);

				rigidBodies.push_back(body);
				boneIds.push_back(colMesh->GetBoneParent());
				body->SetMass(0.f);
			}

			auto &joints = mdl->GetJoints();
			for(auto it = joints.begin(); it != joints.end(); ++it) {
				auto &joint = *it;
				if(joint.src >= rigidBodies.size() || joint.dest >= rigidBodies.size())
					continue;
				auto *srcBody = rigidBodies.at(joint.dest);
				auto *dstBody = rigidBodies.at(joint.src);
				auto &pose = mdl->GetReference();
				// Constraint position/rotation
				auto boneId = boneIds.at(joint.dest);
				auto posConstraint = *pose.GetBonePosition(boneId);
				auto rotConstraint = *pose.GetBoneOrientation(boneId);

				auto posTgt = posConstraint + dstBody->GetOrigin();
				posConstraint = posConstraint + srcBody->GetOrigin();
				auto *c = pragma::get_cgame()->GetPhysicsEnvironment()->CreateFixedConstraint(srcBody, posConstraint, uquat::identity(), dstBody, posTgt, uquat::identity());
				if(c != nullptr)
					c->SetCollisionsEnabled(joint.collide);
			}
		}
	}
	else {
		for(auto *pRigidBody : rigidBodies)
			pRigidBody->SetMass(mdl->GetMass());
		auto cbThink = pragma::get_cgame()->AddCallback("Think", FunctionCallback<>::Create([]() {
			static auto f = Vector3 {0.f, -600.f, 0.f};
			for(auto *body : rigidBodies) {
				auto localForce = f * body->GetMass();
				body->Activate();
				body->ApplyForce(localForce);
			}
		}));
	}

#if 0
	if(true)
	{
		pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
		entIt.AttachFilter<EntityIteratorFilterClass>("prop_physics");
		auto it = entIt.begin();
		auto *ent = (it != entIt.end()) ? *it : nullptr;
		auto mdlComponent = ent->GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		auto &skeleton = mdl->GetSkeleton();
		auto boneId = 2u;
		auto bone = skeleton.GetBone(boneId).lock();
		auto fGetBonePos = [&bone,ent]() -> std::pair<Vector3,Quat> {
			auto pos = Vector3{};
			auto rot = uquat::identity();
		
			auto parent = bone->parent.lock();
			while(parent != nullptr)
			{
				auto posParent = Vector3{};
				auto rotParent = Quat{};
				mdlComponent->GetBonePosition(parent->ID,posParent,rotParent);
				uvec::rotate(&pos,rotParent);
				pos += posParent;
				rot = rotParent *rot;
				parent = parent->parent.lock();
			}
			return {pos,rot};
		};

		auto t = fGetBonePos();
		Con::COUT<<"Original Bone Pos: ("<<t.first.x<<","<<t.first.y<<","<<t.first.z<<")"<<Con::endl;
		Con::COUT<<"Original Bone Rot: ("<<t.second.w<<","<<t.second.x<<","<<t.second.y<<","<<t.second.z<<")"<<Con::endl;

		{
			auto npos = t.first;//*Lua::CheckVector(l,1);
			auto nrot = t.second;//*Lua::CheckQuaternion(l,2);
			get_local_bone_position(mdl,[ent](uint32_t boneId) -> Transform {
				Transform t {};
				mdlComponent->GetBonePosition(boneId,const_cast<Vector3&>(t.GetPosition()),const_cast<Quat&>(t.GetOrientation()));
				return t;
			},bone,{1.f,1.f,1.f},&npos,&nrot);

			Con::COUT<<"New Bone Pos: ("<<npos.x<<","<<npos.y<<","<<npos.z<<")"<<Con::endl;
			Con::COUT<<"New Bone Rot: ("<<nrot.w<<","<<nrot.x<<","<<nrot.y<<","<<nrot.z<<")"<<Con::endl;
		}
	}
	if(true)
	{
		/*auto *ike = new InverseKinematicsExample(IK_JACOB_TRANS);
		ike->initPhysics();

		pragma::get_cgame()->AddCallback("Tick",FunctionCallback<void>::Create([ike]() {
			ike->stepSimulation(pragma::get_cgame()->DeltaTime());
			ike->renderScene();
		}));*/
		/*#define RADIAN(X)	((X)*RadiansToDegrees)
		auto ikTree = pragma::util::make_shared<Tree>();
		auto jacobian = pragma::util::make_shared<Jacobian>(ikTree.get());

		Reset(*ikTree,jacobian.get());

		std::vector<std::shared_ptr<Node>> m_ikNodes;
		m_ikNodes.reserve(8);

		float minTheta = -4 * PI;
		float maxTheta = 4 * PI;

		const VectorR3& unitx = VectorR3::UnitX;
		const VectorR3& unity = VectorR3::UnitY;
		const VectorR3& unitz = VectorR3::UnitZ;
		const VectorR3& zero = VectorR3::Zero;

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 0.087500), unitz, 0.08, JOINT, -1e30, 1e30, RADIAN(0.)));
		ikTree->InsertRoot(m_ikNodes[0].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, -0.000000, 0.290000), unity, 0.08, JOINT, -0.5, 0.4, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[0].get(), m_ikNodes[1].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, -0.000000, 0.494500), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[1].get(), m_ikNodes[2].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 0.710000), -unity, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[2].get(), m_ikNodes[3].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 0.894500), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[3].get(), m_ikNodes[4].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 1.110000), unity, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[4].get(), m_ikNodes[5].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 1.191000), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.)));
		ikTree->InsertLeftChild(m_ikNodes[5].get(), m_ikNodes[6].get());

		m_ikNodes.push_back(pragma::util::make_shared<Node>(VectorR3(0.100000, 0.000000, 1.20000), zero, 0.08, EFFECTOR));
		ikTree->InsertLeftChild(m_ikNodes[6].get(), m_ikNodes[7].get());

		ikTree->Init();
		ikTree->Compute();

		pragma::get_cgame()->AddCallback("Tick",FunctionCallback<void>::Create([ikTree,jacobian,m_ikNodes]() {
			int m_ikMethod = IK_DLS;
			DoUpdateStep(pragma::get_cgame()->DeltaTime(),*ikTree,jacobian.get(),m_ikMethod);

			btTransform act;
			getLocalTransform(ikTree->GetRoot(), act);
			MyDrawTree(ikTree->GetRoot(), act);
		}));*/

		return 0;
	}

	if(true)
	{
		auto *vhc = new Vehicle_Car();
		vhc->InitVehicle(pragma::get_cgame()->GetPhysicsEnvironment(),Vector3());
		pragma::get_cgame()->AddCallback("Tick",FunctionCallback<void>::Create([vhc]() {
			update_vehicle(vhc);
		}));
		return 0;
	}

	if(true)
	{
		pragma::get_client_state()->LoadSoundScripts("level_sounds_trainyard.udm",false);
		auto scene = choreography::Scene::Create<choreography::Scene>();
		auto channel = scene->AddChannel<choreography::Channel>("Test");

		se_script::SceneScriptValue ssv {};
		auto f = pragma::fs::open_file("scenes/sdk_barney1.vcd", pragma::fs::FileMode::Read);//sdk_barney1.vcd","r");
		if(f != nullptr)
		{
			if(se_script::read_scene(f,ssv) == se_script::ResultCode::Ok)
			{
				for(auto &child : ssv.subValues)
				{
					if(child->identifier != "actor")
						continue;
					for(auto &child : child->subValues)
					{
						if(child->identifier != "channel")
							continue;
						for(auto &child : child->subValues)
						{
							if(child->identifier == "event" && child->parameters.empty() == false)
							{
								if(child->parameters.at(0) == "speak")
								{
									std::string sndName;
									auto startTime = 0.f;
									auto endTime = 0.f;
									for(auto &child : child->subValues)
									{
										if(child->parameters.empty())
											continue;
										if(child->identifier == "param")
											sndName = child->parameters.front();
										else if(child->identifier == "time" && child->parameters.size() >= 2)
										{
											startTime = pragma::string::to_float(child->parameters.at(0));
											endTime = pragma::string::to_float(child->parameters.at(1));
										}
									}
									if(sndName.empty() == false)
									{
										auto ev = channel->AddEvent<choreography::AudioEvent>(sndName);
										ev->SetTimeRange(startTime,endTime);

										pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
										entIt.AttachFilter<EntityIteratorFilterClass>("prop_dynamic");
										auto it = entIt.begin();
										auto *ent = (it != entIt.end()) ? *it : nullptr;
										static_cast<choreography::Event*>(ev.get())->SetActor(*static_cast<pragma::ecs::CBaseEntity*>(ent));
									}
								}
								else if(child->parameters.at(0) == "flexanimation")
								{
									struct FlexData
									{
										std::vector<choreography::FacialFlexEvent::TimeValue> values;
										std::vector<choreography::FacialFlexEvent::TimeValue> lrDistribution;
										bool stereo = false;
									};
									std::unordered_map<std::string,FlexData> values {};
									auto startTime = 0.f;
									auto endTime = 0.f;
									for(auto &child : child->subValues)
									{
										if(child->parameters.empty())
											continue;
										if(child->identifier == "flexanimations")
										{
											for(auto it=child->subValues.begin();it!=child->subValues.end();++it)
											{
												auto &flexChild = *it;
												auto bCombo = std::find_if(flexChild->parameters.begin(),flexChild->parameters.end(),[](const std::string &param) {
													return pragma::string::compare(param,"combo",false);
												}) != flexChild->parameters.end();

												auto &flexData = values.insert(std::make_pair(flexChild->identifier,FlexData{})).first->second;
												flexData.values.reserve(flexChild->subValues.size());
												for(auto &child : flexChild->subValues)
												{
													if(child->parameters.empty())
														continue;
													flexData.values.push_back({});
													auto &v = flexData.values.back();
													v.time = pragma::string::to_float(child->identifier);
													v.value = pragma::string::to_float(child->parameters.front());
												}
												if(bCombo == true)
												{
													++it;
													if(it == child->subValues.end())
														break;
													auto &child = *it;
													flexData.lrDistribution.reserve(flexChild->subValues.size());
													flexData.stereo = true;
													for(auto &child : child->subValues)
													{
														if(child->parameters.empty())
															continue;
														flexData.lrDistribution.push_back({});
														auto &v = flexData.lrDistribution.back();
														v.time = pragma::string::to_float(child->identifier);
														v.value = pragma::string::to_float(child->parameters.front());
													}
												}
											}
										}
										else if(child->identifier == "time" && child->parameters.size() >= 2)
										{
											startTime = pragma::string::to_float(child->parameters.at(0));
											endTime = pragma::string::to_float(child->parameters.at(1));
										}
									}
									if(values.empty() == false)
									{
										auto ev = channel->AddEvent<choreography::FacialFlexEvent>();
										ev->SetTimeRange(startTime,endTime);

										pragma::ecs::EntityIterator entIt{*pragma::get_cgame()};
										entIt.AttachFilter<EntityIteratorFilterClass>("prop_dynamic");
										auto it = entIt.begin();
										auto *ent = (it != entIt.end()) ? *it : nullptr;

										static_cast<choreography::Event*>(ev.get())->SetActor(*static_cast<pragma::ecs::CBaseEntity*>(ent));
										for(auto &pair : values)
											static_cast<choreography::FacialFlexEvent*>(ev.get())->SetFlexControllerValues(pair.first,pair.second.values,(pair.second.stereo == true) ? &pair.second.lrDistribution : nullptr);
									}
								}
							}
						}
					}
				}
			}
			f = nullptr;
		}
		//se_script::

		scene->Play();
		pragma::get_cgame()->AddCallback("Think",FunctionCallback<void>::Create([scene,channel]() {
			scene->Tick(pragma::get_cgame()->DeltaTime());
		}));
	}
	static btSoftBody *softBody = nullptr;
	if(softBody == nullptr)
	{
		/*btSoftBodyWorldInfo *softBodyWorldInfo = new btSoftBodyWorldInfo();
	
			auto m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration(); 
			auto m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration); 
			auto m_broadphase = new btDbvtBroadphase();
			auto m_solver = new btSequentialImpulseConstraintSolver; 
			auto m_dynamicsWorld = new btSoftRigidDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
			m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
			softBodyWorldInfo->m_broadphase = m_broadphase;
			softBodyWorldInfo->m_dispatcher = m_dispatcher;
			softBodyWorldInfo->m_gravity = m_dynamicsWorld->getGravity();
			softBodyWorldInfo->m_sparsesdf.Initialize();
	
		*/
		softBody = createSoftBody(pragma::get_cgame()->GetPhysicsEnvironment()->GetWorld(),pragma::get_cgame()->GetPhysicsEnvironment()->GetBtSoftBodyWorldInfo(),4.f,32,32,1);

		pragma::get_cgame()->AddCallback("Tick",FunctionCallback<void>::Create([]() {
			softBody->activate(true);
			//m_dynamicsWorld->stepSimulation(pragma::get_cgame()->DeltaTickTime(),1,pragma::get_cgame()->DeltaTickTime());
		}));
	}
	auto &nodes = softBody->m_nodes;
	auto nodeOffset = 0u;
	auto faceCount = softBody->m_faces.size();
	softBody->addVelocity({0.f,-1.f,0.f});
	std::vector<Vector3> verts;
	verts.reserve(faceCount *3);
	for(auto i=decltype(faceCount){0};i<faceCount;++i)
	{
		auto &f = softBody->m_faces.at(i);
		verts.push_back(Vector3(f.m_n[0]->m_x.x(),f.m_n[0]->m_x.y(),f.m_n[0]->m_x.z()) /static_cast<float>(PhysEnv::WORLD_SCALE));
		verts.push_back(Vector3(f.m_n[2]->m_x.x(),f.m_n[2]->m_x.y(),f.m_n[2]->m_x.z()) /static_cast<float>(PhysEnv::WORLD_SCALE));
		verts.push_back(Vector3(f.m_n[1]->m_x.x(),f.m_n[1]->m_x.y(),f.m_n[1]->m_x.z()) /static_cast<float>(PhysEnv::WORLD_SCALE));
	}
	DebugRenderer::DrawMesh(verts,colors::Red,colors::Lime,36.f);
	//game->Draw
#endif
	return 0;
}

#endif

namespace pragma {
	DLLCLIENT const std::unordered_map<std::string, std::string> &get_dropped_files();
};
int Lua::game::Client::open_dropped_file(lua::State *l)
{
	auto &droppedFiles = pragma::get_cengine()->GetDroppedFiles();
	const pragma::CEngine::DroppedFile *pf = nullptr;
	std::string fullPath {};
	if(IsString(l, 1)) {
		auto *fileName = CheckString(l, 1);
		auto it = std::find_if(droppedFiles.begin(), droppedFiles.end(), [&fileName](const pragma::CEngine::DroppedFile &f) { return (f.fileName == fileName) ? true : false; });
		if(it == droppedFiles.end()) {
			auto &gDroppedFiles = pragma::get_dropped_files();
			auto npath = pragma::util::Path::CreateFile(fileName).GetString();
			pragma::string::to_lower(npath);
			auto nfileName = ufile::get_file_from_filename(npath);
			auto it = gDroppedFiles.find(nfileName);
			if(it == gDroppedFiles.end())
				return 0;
			fullPath = it->second;
		}
		else
			fullPath = it->fullPath;
	}
	else {
		auto id = CheckInt(l, 1);
		--id;
		if(id < 0 || id >= droppedFiles.size())
			return 0;
		fullPath = droppedFiles[id].fullPath;
	}
	auto bBinary = false;
	if(IsSet(l, 2) == true)
		bBinary = CheckBool(l, 2);
	auto f = pragma::fs::open_system_file(fullPath, (bBinary == true) ? (pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary) : pragma::fs::FileMode::Read);
	if(f == nullptr)
		return 0;
	auto r = pragma::util::make_shared<LFile>();
	r->Construct(f);
	Lua::Push<std::shared_ptr<LFile>>(l, r);
	return 1;
}
int Lua::game::Client::set_gravity(lua::State *l)
{
	Vector3 &gravity = Lua::Check<Vector3>(l, 1);
	pragma::get_cgame()->SetGravity(gravity);
	return 0;
}
int Lua::game::Client::get_gravity(lua::State *l)
{
	Lua::Push<Vector3>(l, pragma::get_cgame()->GetGravity());
	return 1;
}
int Lua::game::Client::load_model(lua::State *l)
{
	auto *name = CheckString(l, 1);
	auto reload = false;
	if(IsSet(l, 2))
		reload = CheckBool(l, 2);
	auto mdl = pragma::get_cgame()->LoadModel(name, reload);
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l, mdl);
	return 1;
}
int Lua::game::Client::create_model(lua::State *l)
{
	std::shared_ptr<pragma::asset::Model> mdl = nullptr;
	if(!IsSet(l, 1))
		mdl = pragma::get_cgame()->CreateModel();
	else {
		if(IsBool(l, 1)) {
			auto bAddReference = true;
			if(IsSet(l, 1))
				bAddReference = CheckBool(l, 1);
			mdl = pragma::get_cgame()->CreateModel(bAddReference);
		}
		else {
			std::string name = CheckString(l, 1);
			mdl = pragma::get_cgame()->CreateModel(name);
		}
	}
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l, mdl);
	return 1;
}
int Lua::game::Client::get_action_input(lua::State *l)
{
	auto input = CheckInt(l, 1);
	PushBool(l, pragma::get_cgame()->GetActionInput(static_cast<pragma::Action>(input)));
	return 1;
}
int Lua::game::Client::set_action_input(lua::State *l)
{
	auto input = CheckInt(l, 1);
	auto pressed = CheckBool(l, 2);
	pragma::get_cgame()->SetActionInput(static_cast<pragma::Action>(input), pressed);
	return 0;
}
int Lua::game::Client::update_render_buffers(lua::State *l)
{
	auto &drawSceneInfo = Lua::Check<const pragma::rendering::DrawSceneInfo>(l, 1);
	auto &renderQueue = Lua::Check<const pragma::rendering::RenderQueue>(l, 2);
	pragma::CSceneComponent::UpdateRenderBuffers(drawSceneInfo.commandBuffer, renderQueue);
	return 0;
}
int Lua::game::Client::render_scenes(lua::State *l)
{
	std::vector<pragma::rendering::DrawSceneInfo> scenes {};
	auto n = GetObjectLength(l, 1);
	scenes.reserve(n);

	auto t = luabind::object {luabind::from_stack(l, 1)};
	for(luabind::iterator i {t}, end; i != end; ++i) {
		auto val = *i;
		auto *drawSceneInfo = luabind::object_cast<pragma::rendering::DrawSceneInfo *>(val);
		scenes.push_back(*drawSceneInfo);
	}
	pragma::get_cgame()->RenderScenes(scenes);
	return 0;
}
extern void set_debug_render_filter(std::unique_ptr<pragma::debug::DebugRenderFilter> filter);
int Lua::game::Client::set_debug_render_filter(lua::State *l)
{
	if(IsSet(l, 1) == false) {
		::set_debug_render_filter(nullptr);
		return 0;
	}
	CheckTable(l, 1);
	auto t = luabind::object {luabind::from_stack {l, 1}};
	auto filter = std::make_unique<pragma::debug::DebugRenderFilter>();
	if(t["shaderFilter"]) {
		auto shaderFilter = luabind::object {t["shaderFilter"]};
		filter->shaderFilter = [shaderFilter](pragma::ShaderGameWorld &shader) mutable -> bool {
			auto r = shaderFilter(&shader);
			return luabind::object_cast<bool>(r);
		};
	}
	if(t["materialFilter"]) {
		auto materialFilter = luabind::object {t["materialFilter"]};
		filter->materialFilter = [materialFilter](pragma::material::CMaterial &mat) mutable -> bool {
			auto r = materialFilter(static_cast<pragma::material::Material *>(&mat));
			return luabind::object_cast<bool>(r);
		};
	}
	if(t["entityFilter"]) {
		auto entityFilter = luabind::object {t["entityFilter"]};
		filter->entityFilter = [entityFilter](pragma::ecs::CBaseEntity &ent, pragma::material::CMaterial &mat) mutable -> bool {
			auto &oEnt = ent.GetLuaObject();
			auto r = entityFilter(oEnt, static_cast<pragma::material::Material *>(&mat));
			return luabind::object_cast<bool>(r);
		};
	}
	if(t["meshFilter"]) {
		auto meshFilter = luabind::object {t["meshFilter"]};
		filter->meshFilter = [meshFilter](pragma::ecs::CBaseEntity &ent, pragma::material::CMaterial *mat, pragma::geometry::CModelSubMesh &mesh, pragma::rendering::RenderMeshIndex meshIdx) mutable -> bool {
			auto &oEnt = ent.GetLuaObject();
			auto r = meshFilter(oEnt, mat ? static_cast<pragma::material::Material *>(mat) : nullptr, mesh.shared_from_this(), meshIdx);
			return luabind::object_cast<bool>(r);
		};
	}
	::set_debug_render_filter(std::move(filter));
	return 0;
}
int Lua::game::Client::queue_scene_for_rendering(lua::State *l)
{
	auto &drawSceneInfo = Lua::Check<pragma::rendering::DrawSceneInfo>(l, 1);
	pragma::get_cgame()->QueueForRendering(drawSceneInfo);
#if 0
	auto scene = drawSceneInfo.scene.valid() ? drawSceneInfo.scene.get() : pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
	auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return 0;
	auto cmdBuffer = drawSceneInfo.commandBuffer;
	if(cmdBuffer == nullptr || cmdBuffer->IsPrimary() == false)
		return 0;
	auto *clearColor = drawSceneInfo.clearColor.has_value() ? &drawSceneInfo.clearColor.value() : nullptr;
	pragma::get_cgame()->SetRenderScene(*scene);

	if(clearColor != nullptr)
	{
		auto clearCol = clearColor->ToVector4();
		auto &hdrInfo = static_cast<pragma::CRasterizationRendererComponent*>(renderer)->GetHDRInfo();
		auto &hdrImg = hdrInfo.sceneRenderTarget->GetTexture().GetImage();
		cmdBuffer->RecordImageBarrier(hdrImg,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferDstOptimal);
		cmdBuffer->RecordClearImage(hdrImg,prosper::ImageLayout::TransferDstOptimal,{{clearCol.r,clearCol.g,clearCol.b,clearCol.a}});
		cmdBuffer->RecordImageBarrier(hdrImg,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}

	auto primCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(cmdBuffer);
	pragma::get_cgame()->RenderScene(drawSceneInfo);
	pragma::get_cgame()->ResetRenderScene();
#endif
	return 0;
}
DLLCLIENT void debug_render_stats(bool enabled, bool full, bool print, bool continuous);
int Lua::game::Client::set_render_stats_enabled(lua::State *l)
{
	auto enabled = CheckBool(l, 1);
	pragma::get_cengine()->SetGpuPerformanceTimersEnabled(enabled);
	debug_render_stats(enabled, false, false, true);
	return 0;
}
int Lua::game::Client::get_queued_render_scenes(lua::State *l)
{
	auto &renderScenes = pragma::get_cgame()->GetQueuedRenderScenes();
	auto t = luabind::newtable(l);
	int32_t i = 1;
	for(auto &renderScene : renderScenes)
		t[i++] = const_cast<pragma::rendering::DrawSceneInfo &>(renderScene);
	t.push(l);
	return 1;
}
int Lua::game::Client::create_scene(lua::State *l)
{
	auto argIdx = 1;
	pragma::CSceneComponent::CreateInfo createInfo {};
	if(IsSet(l, argIdx))
		createInfo = Lua::Check<pragma::CSceneComponent::CreateInfo>(l, argIdx++);
	pragma::CSceneComponent *parent = nullptr;
	if(IsSet(l, argIdx))
		parent = &Lua::Check<pragma::CSceneComponent>(l, argIdx++);
	auto *scene = pragma::CSceneComponent::Create(createInfo, parent);
	if(scene == nullptr)
		return 0;
	scene->GetLuaObject().push(l);
	return 1;
}
int Lua::game::Client::get_render_scene(lua::State *l)
{
	auto *scene = pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
	if(scene == nullptr)
		return 0;
	scene->GetLuaObject().push(l);
	return 1;
}
int Lua::game::Client::get_render_scene_camera(lua::State *l)
{
	auto *scene = pragma::get_cgame()->GetRenderScene<pragma::CSceneComponent>();
	if(scene == nullptr)
		return 0;
	auto &cam = scene->GetActiveCamera();
	if(cam.expired())
		return 0;
	cam->PushLuaObject(l);
	return 1;
}
int Lua::game::Client::get_scene(lua::State *l)
{
	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
	if(scene == nullptr)
		return 0;
	scene->GetLuaObject().push(l);
	return 1;
}
int Lua::game::Client::get_scene_by_index(lua::State *l)
{
	auto *scene = pragma::CSceneComponent::GetByIndex(CheckInt(l, 1));
	if(scene == nullptr)
		return 0;
	scene->GetLuaObject().push(l);
	return 1;
}
int Lua::game::Client::get_scene_camera(lua::State *l)
{
	auto *cam = pragma::get_cgame()->GetPrimaryCamera<pragma::CCameraComponent>();
	if(cam == nullptr)
		return 0;
	cam->PushLuaObject(l);
	return 1;
}

int Lua::game::Client::get_draw_command_buffer(lua::State *l)
{
	auto &drawCmd = pragma::get_cengine()->GetDrawCommandBuffer();
	Push(l, std::static_pointer_cast<prosper::ICommandBuffer>(drawCmd));
	return 1;
}
int Lua::game::Client::get_setup_command_buffer(lua::State *l)
{
	auto &setupCmd = pragma::get_cengine()->GetSetupCommandBuffer();
	Lua::Push<std::shared_ptr<prosper::ICommandBuffer>>(l, setupCmd);
	return 1;
}
int Lua::game::Client::flush_setup_command_buffer(lua::State *l)
{
	pragma::get_cengine()->FlushSetupCommandBuffer();
	return 0;
}
int Lua::game::Client::get_camera_position(lua::State *l)
{
	auto *cam = pragma::get_cgame()->GetPrimaryCamera<pragma::CCameraComponent>();
	if(cam == nullptr) {
		Lua::Push<Vector3>(l, Vector3 {});
		Lua::Push<Quat>(l, Quat {});
		return 2;
	}
	Lua::Push<Vector3>(l, cam->GetEntity().GetPosition());
	Lua::Push<Quat>(l, cam->GetEntity().GetRotation());
	return 2;
}
int Lua::game::Client::get_render_clip_plane(lua::State *l)
{
	Lua::Push<::Vector4>(l, pragma::get_cgame()->GetRenderClipPlane());
	return 1;
}
int Lua::game::Client::set_render_clip_plane(lua::State *l)
{
	auto &clipPlane = Lua::Check<::Vector4>(l, 1);
	pragma::get_cgame()->SetRenderClipPlane(clipPlane);
	return 0;
}
int Lua::game::Client::get_debug_buffer(lua::State *l)
{
	auto &renderSettings = pragma::get_cgame()->GetGlobalRenderSettingsBufferData();
	Push(l, renderSettings.debugBuffer);
	return 1;
}
int Lua::game::Client::get_time_buffer(lua::State *l)
{
	auto &renderSettings = pragma::get_cgame()->GetGlobalRenderSettingsBufferData();
	Push(l, renderSettings.timeBuffer);
	return 1;
}
int Lua::game::Client::get_csm_buffer(lua::State *l)
{
	auto &renderSettings = pragma::get_cgame()->GetGlobalRenderSettingsBufferData();
	Push(l, renderSettings.csmBuffer);
	return 1;
}
int Lua::game::Client::get_render_settings_descriptor_set(lua::State *l)
{
	auto &renderSettings = pragma::get_cgame()->GetGlobalRenderSettingsBufferData();
	Push(l, renderSettings.descSetGroup);
	return 1;
}
int Lua::game::Client::load_map(lua::State *l)
{
	std::string mapName;
	Vector3 origin {};
	return game::load_map(l, mapName, nullptr, origin).second;
}
int Lua::game::Client::build_reflection_probes(lua::State *l)
{
	auto rebuild = false;
	if(IsSet(l, 1))
		rebuild = CheckBool(l, 1);
	pragma::CReflectionProbeComponent::BuildAllReflectionProbes(*pragma::get_cgame(), rebuild);
	return 0;
}
