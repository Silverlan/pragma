#include "stdafx_shared.h"
#include <pragma/engine.h>
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physxstream.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/physics/physxcontrollerhitreport.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/collisionmesh.h"
#include <pragma/physics/movetypes.h>
#include "pragma/physics/physconstraint.h"
#include "pragma/model/modelmesh.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/model.h"
#include "pragma/entities/components/basegravity.h"
#include "pragma/game/game_coordinate_system.hpp"

using namespace pragma;

extern DLLENGINE Engine *engine;

#ifdef PHYS_ENGINE_BULLET

PhysRigidBody *BasePhysicsComponent::CreateRigidBody(const std::shared_ptr<PhysShape> &shape,float mass,const Vector3 &origin)
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	Vector3 localInertia(0.f,0.f,0.f);
	shape->CalculateLocalInertia(mass,&localInertia);
	auto *body = physEnv->CreateRigidBody(mass,shape,localInertia);
	if(body == nullptr)
		return nullptr;
	body->SetOrigin(origin);
	auto originOffset = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto rot = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();
	auto tOrigin = origin;
	uvec::rotate(&tOrigin,rot);
	PhysTransform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(/*-tOrigin +*/originOffset);
	startTransform.SetRotation(rot);
	btScalar contactProcessingThreshold = BT_LARGE_FLOAT;

	auto group = GetCollisionFilter();
	if(group != CollisionMask::Default)
		body->SetCollisionFilterGroup(group);
	auto mask = GetCollisionFilterMask();
	if(mask != CollisionMask::Default)
		body->SetCollisionFilterMask(mask);
	body->SetWorldTransform(startTransform);
	body->SetContactProcessingThreshold(CFloat(contactProcessingThreshold));
	int flags = body->GetFlags();
	if(mass == 0.f)
	{
		flags |= btCollisionObject::CF_STATIC_OBJECT;
		body->UpdateAABB();
		body->SetActivationState(DISABLE_SIMULATION);
	}
	body->SetCollisionFlags(flags);
	return body;
}

util::WeakHandle<PhysObj> BasePhysicsComponent::InitializeModelPhysics(bool bDynamic)
{
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->HasModel() == false)
		return {};
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	float mass = hMdl->GetMass();
	auto bStaticMass = (mass == 0.f) ? true : false;
	if(bDynamic == true && bStaticMass == true) // If a mesh has been initialized with a mass of 0, it can't be changed anymore? (TODO: Then why does it work for the world? CHECK AND CONFIRM/DISCONFIRM!)
		mass = 1.f;
	auto &meshes = hMdl->GetCollisionMeshes();
	if(meshes.empty())
		return {};

	auto bPhys = false;
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto &joints = hMdl->GetJoints();
	struct ShapeInfo
	{
		ShapeInfo(const std::shared_ptr<PhysShape> &_shape,const Vector3 &_origin)
			: shape(_shape),origin(_origin)
		{}
		std::shared_ptr<PhysShape> shape;
		Vector3 origin;
	};
	struct SortedShape
	{
		SortedShape()
			: body(nullptr)
		{}
		std::vector<ShapeInfo> shapes;
		PhysRigidBody *body;
	};
	std::unordered_map<int,SortedShape> sortedShapes;
	std::unordered_map<unsigned int,unsigned int> sortedShapeMap; // meshes -> sortedShapes
	unsigned int meshId = 0;
	sortedShapeMap.reserve(meshes.size());
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	auto bScale = (scale != Vector3{1.f,1.f,1.f}) ? true : false;
	for(auto &mesh : meshes)
	{
		auto shape = (bScale == false) ? mesh->GetShape() : mesh->CreateShape(scale);
		if(shape != nullptr)
		{
			auto bone = mesh->GetBoneParent();
			auto itMesh = sortedShapes.find(bone);
			if(itMesh == sortedShapes.end())
				itMesh = sortedShapes.insert(std::unordered_map<int,SortedShape>::value_type(bone,SortedShape())).first;
			itMesh->second.shapes.push_back(ShapeInfo(shape,mesh->GetOrigin() *scale));
			sortedShapeMap[meshId] = bone;
		}
		meshId++;
	}
	if(sortedShapes.empty())
		return {};
	auto itRoot = sortedShapes.find(meshes.front()->GetBoneParent()); // Find root mesh
	auto it = itRoot;
	auto bHasRoot = (it != sortedShapes.end()) ? true : false;
	auto bUseRoot = bHasRoot;
	for(;;)
	{
		if(bHasRoot == true)
		{
			if(it == itRoot)
			{
				if(bUseRoot == false)
				{
					bHasRoot = false;
					it = sortedShapes.begin();
					if(it == itRoot)
						++it;
				}
				else
					bUseRoot = false;
			}
		}
		else
			++it;
		if(it == sortedShapes.end())
			break;
		else if(it == itRoot && bHasRoot == false)
			continue;
		//auto bone = it->first;
		auto &sortedShape = it->second;
		auto &shapes = sortedShape.shapes;
		Vector3 origin(0.f,0.f,0.f);
		std::shared_ptr<PhysShape> shape;
		if(shapes.size() == 1)
		{
			auto &shapeInfo = shapes.front();
			shape = shapeInfo.shape;
			origin = shapeInfo.origin;
		}
		else
		{
			if(bDynamic == false)
			{
				for(auto it=shapes.begin();it!=shapes.end();++it)
				{
					auto *body = CreateRigidBody(it->shape,mass,it->origin);//-it->origin);
					if(bPhys == false)
					{
						bPhys = true;
						if(m_physObject != nullptr)
							DestroyPhysicsObject();
						m_physObject = std::make_shared<RigidPhysObj>(this,body);
					}
					else
						m_physObject->AddCollisionObject(body);
				}
				/*shape = physEnv->CreateCompoundShape();
				auto cmpShape = std::static_pointer_cast<PhysCompoundShape>(shape);
				for(auto it=shapes.begin();it!=shapes.end();++it)
					cmpShape->AddShape(it->shape,-it->origin);
				auto *body = CreateRigidBody(shape,mass,Vector3{});//-it->origin);
				if(bPhys == false)
				{
					bPhys = true;
					if(m_physObject != nullptr)
						DestroyPhysicsObject();
					m_physObject = new RigidPhysObj(this,body);
				}
				else
					m_physObject->AddCollisionObject(body);*/ // Compound mesh for all geometry; Causes crash in btAdjustInternalEdgeContacts
				continue;
			}
			shape = physEnv->CreateCompoundShape();
			auto cmpShape = std::static_pointer_cast<PhysCompoundShape>(shape);
			for(auto it=shapes.begin();it!=shapes.end();++it)
				cmpShape->AddShape(it->shape,-it->origin);
		}
		auto *body = CreateRigidBody(shape,mass,origin);
		body->SetBoneID(it->first);
		if(it->first >= 0 && bDynamic == true && joints.empty() == false) // Static physics and non-ragdolls can still play animation
			umath::set_flag(m_stateFlags,StateFlags::Ragdoll);
		sortedShape.body = body;
		if(bPhys == false)
		{
			bPhys = true;
			if(m_physObject != nullptr)
				DestroyPhysicsObject();
			m_physObject = std::make_shared<RigidPhysObj>(this,body);
		}
		else
			m_physObject->AddCollisionObject(body);
	}
	for(auto it=joints.begin();it!=joints.end();++it)
	{
		auto &joint = *it;
		auto itSrc = sortedShapeMap.find(joint.dest);//joint.src); // TODO: Swap variable names
		auto itDest = sortedShapeMap.find(joint.src);
		if(itSrc != sortedShapeMap.end() && itDest != sortedShapeMap.end())
		{
			auto &src = sortedShapes[itSrc->second]; // Bone Id
			auto &dest = sortedShapes[itDest->second]; // Bone Id
			auto *bodySrc = src.body;
			auto *bodyTgt = dest.body;
			if(bodySrc == nullptr || bodyTgt == nullptr)
				continue;
			PhysConstraint *c = nullptr;

			auto boneId = itDest->second;

			auto &pose = hMdl->GetReference();
			// Constraint position/rotation
			auto posConstraint = *pose.GetBonePosition(boneId);
			auto rotConstraint = *pose.GetBoneOrientation(boneId);

			auto posTgt = posConstraint +bodyTgt->GetOrigin();

			posConstraint = posConstraint +bodySrc->GetOrigin();
			//Con::cerr<<"Constraint for bone "<<boneId<<": ("<<posConstraint.x<<","<<posConstraint.y<<","<<posConstraint.z<<") ("<<posTgt.x<<","<<posTgt.y<<","<<posTgt.z<<") "<<Con::endl;
			//
			if(joint.type == JOINT_TYPE_FIXED)
				c = physEnv->CreateFixedConstraint(bodySrc,posConstraint,uquat::identity(),bodyTgt,posTgt,uquat::identity());
			else if(joint.type == JOINT_TYPE_CONETWIST)
			{
				// Conetwist constraints are deprecated for ragdolls and should be avoided.
				// Use DoF constraints instead!
				auto sp = Vector2(0.f,0.f);
				auto sp2 = Vector2(0.f,0.f);
				auto ts = Vector2(0.f,0.f);

				auto sp1l = joint.args.find("sp1l");
				if(sp1l != joint.args.end())
					sp[0] = CFloat(atof(sp1l->second.c_str()));
				auto sp1u = joint.args.find("sp1u");
				if(sp1u != joint.args.end())
					sp[1] = CFloat(atof(sp1u->second.c_str()));

				auto sp2l = joint.args.find("sp2l");
				if(sp2l != joint.args.end())
					sp2[0] = CFloat(atof(sp2l->second.c_str()));
				auto sp2u = joint.args.find("sp2u");
				if(sp2u != joint.args.end())
					sp2[1] = CFloat(atof(sp2u->second.c_str()));

				auto tsl = joint.args.find("tsl");
				if(tsl != joint.args.end())
					ts[0] = CFloat(atof(tsl->second.c_str()));
				auto tsu = joint.args.find("tsu");
				if(tsu != joint.args.end())
					ts[1] = CFloat(atof(tsu->second.c_str()));

				// Notes:
				// bodySrc is the parent collision object. The constraint origin is relative to this parent.
				// We want the origin to be at the position of the parent's bone, so we need to define it as the
				// difference between the two, which is 'posConstraint'.
				// Since the parent collision object doesn't have a rotation, we can just use the bone's rotation for
				// the constraint ('rotConstraint').
				// The same applies for the constraint child, which is the collision object affected by the constraint.
				// However, since the constraint limits are defined as min and max values, but bullet uses a single span value (=(max -min) /2.0),
				// the rotation has to be changed to point to the 'center' of the limits ('rotOffset').
				auto rotOffset = uquat::create(EulerAngles((sp.y +sp.x) *0.5f,(ts.y +ts.x) *0.5f,(sp2.y +sp2.x) *0.5f));
				auto *ct = physEnv->CreateConeTwistConstraint(bodySrc,posConstraint,rotConstraint,bodyTgt,posTgt,rotOffset);
				auto softness = 1.f;
				auto biasFactor = 0.3f;
				auto relaxationFactor = 1.f;

				auto sftn = joint.args.find("sftn");
				if(sftn != joint.args.end())
					softness = CFloat(atof(sftn->second.c_str()));
				auto bias = joint.args.find("bias");
				if(bias != joint.args.end())
					biasFactor = CFloat(atof(bias->second.c_str()));
				auto rlx = joint.args.find("rlx");
				if(rlx != joint.args.end())
					relaxationFactor = CFloat(atof(rlx->second.c_str()));

				auto swingSpan1 = (sp.y -sp.x) *0.5f;
				auto swingSpan2 = (sp2.y -sp2.x) *0.5f;
				auto twistSpan = (ts.y -ts.x) *0.5f;

				ct->SetLimit(
					CFloat(umath::deg_to_rad(swingSpan1)),
					CFloat(umath::deg_to_rad(swingSpan2)),
					CFloat(umath::deg_to_rad(twistSpan)),
					softness,biasFactor,relaxationFactor
				);
				c = ct;
			}
			else if(joint.type == JOINT_TYPE_DOF)
			{
				auto limitLinMin = Vector3(0.f,0.f,0.f);
				auto limitLinMax = Vector3(0.f,0.f,0.f);

				auto limitAngMin = EulerAngles(0.f,0.f,0.f);
				auto limitAngMax = EulerAngles(0.f,0.f,0.f);

				auto itLimitLinMin = joint.args.find("lin_limit_l");
				if(itLimitLinMin != joint.args.end())
					limitLinMin = uvec::create(itLimitLinMin->second);

				auto itLimitLinMax = joint.args.find("lin_limit_u");
				if(itLimitLinMax != joint.args.end())
					limitLinMax = uvec::create(itLimitLinMax->second);

				auto itLimitAngMin = joint.args.find("ang_limit_l");
				if(itLimitAngMin != joint.args.end())
					limitAngMin = EulerAngles(itLimitAngMin->second);

				auto itLimitAngMax = joint.args.find("ang_limit_u");
				if(itLimitAngMax != joint.args.end())
					limitAngMax = EulerAngles(itLimitAngMax->second);

				auto *ct = physEnv->CreateDoFSpringConstraint(bodyTgt,posTgt,rotConstraint,bodySrc,posConstraint,rotConstraint);
				//auto *ct = physEnv->CreateDoFConstraint(bodyTgt,posTgt,rotConstraint,bodySrc,posConstraint,rotConstraint);

				//ct->SetLinearLimit(limitLinMin,limitLinMax);
				//ct->SetAngularLimit(limitAngMin,limitAngMax);
				ct->SetLinearLowerLimit(limitLinMin);
				ct->SetLinearUpperLimit(limitLinMax);

				ct->SetAngularLowerLimit(Vector3(umath::deg_to_rad(limitAngMin.p),umath::deg_to_rad(limitAngMin.y),umath::deg_to_rad(limitAngMin.r)));
				ct->SetAngularUpperLimit(Vector3(umath::deg_to_rad(limitAngMax.p),umath::deg_to_rad(limitAngMax.y),umath::deg_to_rad(limitAngMax.r)));

				c = ct;
			}
			if(c != nullptr)
			{
				c->SetCollisionsEnabled(joint.collide);
				m_joints.push_back(PhysJoint(itSrc->second,itDest->second));
				auto &jointData = m_joints.back();
				jointData.constraint = c->GetHandle();
			}
		}
	}
	if(bPhys == false)
		return {};
	m_physObject->Spawn();
	auto &collisionObjs = m_physObject->GetCollisionObjects();
	auto animComponent = ent.GetAnimatedComponent();
	PhysCollisionObject *root = nullptr;
	auto posRoot = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	if(!collisionObjs.empty())
	{
		auto &hRoot = collisionObjs.front();
		if(hRoot.IsValid())
		{
			root = hRoot.get();
			auto boneId = root->GetBoneID();
			if(animComponent.valid() && animComponent->GetLocalBonePosition(boneId,posRoot) == true && pTrComponent.valid())
				uvec::local_to_world(pTrComponent->GetPosition(),pTrComponent->GetOrientation(),posRoot); // World space position of root collision object bone
		}
	}

	// Move all collision objects to their respective bone counterparts (To keep our previous pose)
	std::function<void(std::unordered_map<uint32_t,std::shared_ptr<Bone>>&)> fUpdateCollisionObjects;
	fUpdateCollisionObjects = [this,&fUpdateCollisionObjects](std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones) {
		for(auto &it : bones)
		{
			UpdateBoneCollisionObject(it.first,true,true);
			fUpdateCollisionObjects(it.second->children);
		}
	};
	auto &skeleton = hMdl->GetSkeleton();
	//auto &rootBones = skeleton.GetRootBones();
	//fUpdateCollisionObjects(rootBones);

	if(m_joints.empty() == false) // Is this a ragdoll?
	{
		// Set collision object positions to current bone positions to
		// transition the current animation smoothly into the ragdoll
		auto &reference = hMdl->GetReference();
		for(auto &colObj : collisionObjs)
		{
			auto boneId = colObj->GetBoneID();
			auto *posRef = reference.GetBonePosition(boneId);
			if(posRef == nullptr)
				continue;
			auto *rotRef = reference.GetBoneOrientation(boneId);
				
			Vector3 pos = {};
			Quat rot = uquat::identity();
			if(animComponent.valid())
				animComponent->GetLocalBonePosition(boneId,pos,rot);
			rot = rot *uquat::get_inverse(*rotRef);
			auto offset = *posRef +colObj->GetOrigin();
			uvec::rotate(&offset,rot);
			pos = pos +(-offset);
			if(pTrComponent.valid())
				pTrComponent->LocalToWorld(&pos,&rot);

			colObj->SetPos(pos);
			colObj->SetRotation(rot);
		}
	}

	// Update position
	//SetPosition(posRoot,true);

	// Transform all bones, which don't have a shape assigned to them, to the reference pose (As they won't be affected by ragdoll physics)
	// TODO: This can look ugly, because it 'snaps' the bones in place. This should be done over several frames to make the transition smoother!
	if(animComponent.valid())
	{
		auto refAnim = hMdl->GetAnimation(0);
		if(refAnim != nullptr)
		{
			auto frame = refAnim->GetFrame(0); // Reference frame with local bone transformations
			if(frame != nullptr)
			{
				auto &bones = skeleton.GetBones();
				for(auto &bone : bones)
				{
					auto it = sortedShapes.find(bone->ID);
					if(it == sortedShapes.end())
					{
						auto *pos = frame->GetBonePosition(bone->ID);
						auto *rot = frame->GetBoneOrientation(bone->ID);
						if(pos != nullptr)
							animComponent->SetBonePosition(bone->ID,*pos,*rot);
					}
				}
			}
		}
	}
	//

	if(bDynamic == true)
	{
		m_physicsType = PHYSICSTYPE::DYNAMIC;
		SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::All);
		SetMoveType(MOVETYPE::PHYSICS);
	}
	else
	{
		m_physicsType = PHYSICSTYPE::STATIC;
		SetCollisionFilter(CollisionMask::Static | CollisionMask::Generic,CollisionMask::All);
		SetMoveType(MOVETYPE::NONE);
	}
	InitializePhysObj();
	if(bDynamic == true && bStaticMass == true)
		m_physObject->SetMass(0.f);
	OnPhysicsInitialized();
	return m_physObject;
}

util::WeakHandle<PhysObj> BasePhysicsComponent::InitializeBrushPhysics(bool bDynamic,float mass) // Obsolete?
{
	auto &ent = GetEntity();
	NetworkState *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	PhysEnv *physEnv = game->GetPhysicsEnvironment();

	auto &meshes = GetBrushMeshes();
	auto pTrComponent = ent.GetTransformComponent();
	auto origin = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	bool bPhys = false;
	for(unsigned int i=0;i<meshes.size();i++)
	{
		std::vector<Vector3> vertices;
		auto &mesh = meshes[i];
		bool bConvex = mesh->IsConvex();
		if(bConvex == true) // Create a single convex mesh
		{
			auto shape = mesh->GetShape();

			Vector3 localInertia(0.f,0.f,0.f);
			shape->CalculateLocalInertia(mass,&localInertia);

			PhysTransform startTransform;
			startTransform.SetIdentity();
			startTransform.SetOrigin(origin);
			btScalar contactProcessingThreshold = BT_LARGE_FLOAT;

			PhysRigidBody *body = physEnv->CreateRigidBody(mass,shape,localInertia);
			//PhysRigidBody *body = physEnv->CreateRigid
			//btRigidBody *body = new btRigidBody(mass,NULL,shape,localInertia);
			body->SetWorldTransform(startTransform);
			body->SetContactProcessingThreshold(CFloat(contactProcessingThreshold));
			int flags = body->GetFlags();
			if(mass == 0.f)
				flags |= btCollisionObject::CF_STATIC_OBJECT;
			body->SetCollisionFlags(flags);
			if(bPhys == false)
			{
				bPhys = true;
				if(m_physObject != NULL)
					DestroyPhysicsObject();
				m_physObject = std::make_shared<RigidPhysObj>(this,body);
			}
			else
				m_physObject->AddCollisionObject(body);
		}
		else // It's a displacement, every side will be a seperate actor
		{
			
		}
	}
	if(bPhys == false)
		return {};
	if(bDynamic == true)
	{
		m_physicsType = PHYSICSTYPE::DYNAMIC;
		SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::All);
		SetMoveType(MOVETYPE::PHYSICS);
	}
	else
	{
		m_physicsType = PHYSICSTYPE::STATIC;
		SetCollisionFilter(CollisionMask::Static | CollisionMask::Generic,CollisionMask::All);
		SetMoveType(MOVETYPE::NONE);
	}

	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
util::WeakHandle<PhysObj> BasePhysicsComponent::InitializeBoxControllerPhysics()
{
	Vector3 min,max;
	GetCollisionBounds(&min,&max);
	auto origin = (min +max) /2.f;
	auto extents = max -min;

	m_physObject = std::static_pointer_cast<PhysObj>(std::make_shared<BoxControllerPhysObj>(this,extents *0.5f,24));
	m_physObject->GetCollisionObject()->SetOrigin(-origin);
	static_cast<BoxControllerPhysObj*>(m_physObject.get())->SetCollisionBounds(min,max);
	m_physicsType = PHYSICSTYPE::BOXCONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic | CollisionMask::Player,CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item);
	m_physObject->Spawn();
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
util::WeakHandle<PhysObj> BasePhysicsComponent::InitializeCapsuleControllerPhysics()
{
	Vector3 min,max;
	GetCollisionBounds(&min,&max);

	m_physObject = std::static_pointer_cast<PhysObj>(std::make_shared<CapsuleControllerPhysObj>(this,CUInt32(umath::max(max.x -min.x,max.z -min.z)),CUInt32(max.y -min.y),24));
	m_physicsType = PHYSICSTYPE::CAPSULECONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic | CollisionMask::Player,CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item);
	m_physObject->Spawn();
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
#elif PHYS_ENGINE_PHYSX
#pragma message ("URGENT: Implement this properly!")
static physx::PxQuat getLookAtQuat(const physx::PxVec3& fromPos, const physx::PxVec3& toPos)
{
    // Position of toPos relative to fromPos
    const physx::PxVec3 relToPos = toPos -fromPos;
 
    /**
     * First we rotate fromPos around Y axis to look at toPos
     * This gives us Euler angle around Y axis
     */
 
    // Compute the angle
    // theta = atan(z/x)
    const float yAng0 = atan(fabs(relToPos.z) / fabs(relToPos.x));
 
    // Fix the angle based on XZ quadrant point lies in
    float yAng;
    if (relToPos.x >= 0)
    {
        if (relToPos.z >= 0)
            yAng = 2 * M_PI - yAng0; // 360 - theta
        else
            yAng = yAng0;
    }
    else
    {
        if (relToPos.z >= 0)
            yAng = M_PI + yAng0; // 180 + theta
        else
            yAng = M_PI - yAng0; // 180 - theta
    }
 
    /**
     * Next fromPos will look "up" to see toPos
     * This gives us Euler angle around Z axis
     */
 
    // Compute the angle
    // theta = atan( y / sqrt(x^2 + z^2))
    const float zAng0 = atan(fabs(relToPos.y) /
        sqrt(relToPos.x * relToPos.x + relToPos.z * relToPos.z));
 
    // Fix angle based on whether toPos is above or below XZ plane
    const float zAng = (relToPos.y >= 0) ? zAng0 : -zAng0;
 
    /**
     * Convert Euler angles to quaternion that rotates
     * X axis of upright orientation to point at toPos
     * Reference: PhysX Math Primer
     */
 
    // Convert to quaternions
    physx::PxQuat qy(yAng, physx::PxVec3(0, 1, 0));
    physx::PxQuat qz(zAng, physx::PxVec3(0, 0, 1));
 
    // Rotate local axes
    physx::PxQuat q = qy * qz;
 
    return q;
}
class WVPxControllerBehaviorCallback
	: public physx::PxControllerBehaviorCallback
{
	physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape &shape,const physx::PxActor &actor)
	{
		return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | physx::PxControllerBehaviorFlag::eCCT_SLIDE;
	}
	physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController &controller)
	{
		return physx::PxControllerBehaviorFlags(0);
	}
	physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle &obstacle)
	{
		return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | physx::PxControllerBehaviorFlag::eCCT_SLIDE;
	}
};
PhysObj *BasePhysicsComponent::InitializeModelPhysics(bool bDynamic)
{
	Model *mdl = GetModel();
	if(mdl == NULL)
		return NULL;
	std::vector<CollisionMesh*> *collisionMeshes;
	mdl->GetCollisionMeshes(&collisionMeshes);
	if(collisionMeshes->empty())
		return NULL;
	float mass = mdl->GetMass();
	Skeleton *skeleton = mdl->GetSkeleton();
	physx::PxPhysics *physics = engine->GetPhysics();
	physx::PxCooking *cooking = engine->GetCookingLibrary();
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();

	physx::PxMaterial *material = physics->createMaterial(0.5f,0.5f,0.1f);
	physx::PxConvexMeshDesc meshDesc;
	meshDesc.points.stride = sizeof(Vector3);
	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
	meshDesc.vertexLimit = 256;
	std::vector<physx::PxRigidActor*> actors;
	std::unordered_map<int,unsigned int> boneActorIDs;
	bool bPhys = false;
	for(unsigned int i=0;i<collisionMeshes->size();i++)
	{
		CollisionMesh *mesh = (*collisionMeshes)[i];
		std::vector<Vector3> &verts = mesh->vertices;
		if(!verts.empty())
		{
			meshDesc.points.count = verts.size();
			meshDesc.points.data = &verts[0];
			physx::MemoryOutputStream buf;
			if(cooking->cookConvexMesh(meshDesc,buf))
			{
				physx::MemoryInputData input(buf.getData(),buf.getSize());
				physx::PxConvexMesh *convexMesh = physics->createConvexMesh(input);
				if(bDynamic == true)
				{
					int boneID = mesh->GetBoneParent();
					std::unordered_map<int,unsigned int>::iterator it = boneActorIDs.find(boneID);
					if(it == boneActorIDs.end())
					{
						Vector3 &pos = GetPosition();
						auto &rot = GetOrientation();
						physx::PxTransform t(physx::PxVec3(pos.x,pos.y,pos.z),physx::PxQuat(rot.x,rot.y,rot.z,rot.w));
						physx::PxRigidDynamic *actor = game->CreatePhysXActor(physx::PxConvexMeshGeometry(convexMesh),*material,t);
						actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD,true);
						actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION,true);
						physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor,mass);
						if(bPhys == false)
						{
							bPhys = true;
							if(m_physObject != NULL)
								DestroyPhysicsObject();
							m_physObject = new DynamicPhysObj(this,actor,boneID);
						}
						else
							static_cast<DynamicPhysObj*>(m_physObject)->AddActor(actor,boneID);
						actors.push_back(actor);
						boneActorIDs.insert(std::unordered_map<int,unsigned int>::value_type(boneID,actors.size() -1));
					}
					else
						actors[it->second]->createShape(physx::PxConvexMeshGeometry(convexMesh),*material);
				}
				else
				{
					if(!actors.empty())
						actors.back()->createShape(physx::PxConvexMeshGeometry(convexMesh),*material);
					else
					{
						Vector3 &pos = GetPosition();
						auto &rot = GetOrientation();
						physx::PxTransform t(physx::PxVec3(pos.x,pos.y,pos.z),physx::PxQuat(rot.x,rot.y,rot.z,rot.w));
						physx::PxRigidStatic *actor = game->CreateStaticPhysXActor(physx::PxConvexMeshGeometry(convexMesh),*material,t);
						if(bPhys == false)
						{
							bPhys = true;
							if(m_physObject != NULL)
								DestroyPhysicsObject();
							m_physObject = new StaticPhysObj(this,actor);
						}
						actors.push_back(actor);
					}
				}
			}
		}
	}
	if(bPhys == false)
		return NULL;
	std::unordered_map<int,unsigned int>::iterator it;
	for(it=boneActorIDs.begin();it!=boneActorIDs.end();it++)
	{
		int boneID = it->first;
		unsigned int actorID = it->second;
		physx::PxRigidActor *actor = actors[actorID];
		Bone *bone = skeleton->GetBone(boneID);
		JointInfo &joint = bone->joint;
		if(joint.type != JOINT_TYPE_NONE)
		{
			Bone *parent = bone->parent;
			std::unordered_map<int,unsigned int>::iterator itParent;
			if(parent != NULL)
			{
				itParent = boneActorIDs.find(parent->ID);
				while(parent != NULL && itParent == boneActorIDs.end())
				{
					parent = parent->parent;
					if(parent != NULL)
						itParent = boneActorIDs.find(parent->ID);
				}
			}
			if(parent != NULL && itParent != boneActorIDs.end())
			{
				std::unordered_map<int,unsigned int>::iterator itParent = boneActorIDs.find(parent->ID);
				if(itParent != boneActorIDs.end())
				{
					physx::PxRigidActor *actorParent = actors[itParent->second];
					switch(joint.type)
					{
					case JOINT_TYPE_SPHERICAL:
						{
							physx::PxVec3 fromPos = actorParent->getGlobalPose().p;
							physx::PxVec3 toPos = actor->getGlobalPose().p;
							physx::PxQuat fromToQuat = getLookAtQuat(fromPos,toPos);
							physx::PxTransform tSrc(physx::PxVec3(0.f,0.f,0.f),fromToQuat);
							physx::PxTransform tTgt((toPos -fromPos) *0.5f,fromToQuat);
							physx::PxSphericalJoint *joint = physx::PxSphericalJointCreate(
								*physics,
								actorParent,
								tSrc,
								actor,
								tTgt
							);
							if(joint != NULL)
							{
								joint->setLimitCone(physx::PxJointLimitCone(0.8f,0.8f,0.1f));
								joint->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED,true);
							}
							break;
						}
					case JOINT_TYPE_FIXED:
						{
							physx::PxTransform local;
							local.p = physx::PxVec3(0,0,0);
							local.q = physx::PxQuat(0,0,0,1);
							physx::PxFixedJointCreate(
								*physics,
								actor,
								local,
								actorParent,
								local
							);
							break;
						}
					case JOINT_TYPE_REVOLUTE:
						{
							break;
						}
					case JOINT_TYPE_DISTANCE:
						{
							break;
						}
					case JOINT_TYPE_PRISMATIC:
						{
							break;
						}
					default:
						{}
					}
				}
			}
		}
	}
	if(bDynamic == true)
	{
		m_physicsType = PHYSICSTYPE::DYNAMIC;
		SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::Dynamic | CollisionMask::Generic);
	}
	else
	{
		m_physicsType = PHYSICSTYPE::STATIC;
		SetCollisionFilter(CollisionMask::Static | CollisionMask::Generic | CollisionMask::Particle,CollisionMask::Static | CollisionMask::Generic);
	}
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
PhysObj *BasePhysicsComponent::InitializeBrushPhysics(bool bDynamic,float mass)
{
	physx::PxPhysics *physics = engine->GetPhysics();
	physx::PxCooking *cooking = engine->GetCookingLibrary();
	physx::PxMaterial *material = physics->createMaterial(0.5f,0.5f,0.1f);
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();

	std::vector<BrushMesh*> *meshes;
	GetBrushMeshes(&meshes);
	physx::PxRigidActor *physActor = NULL;
	bool bPhys = false;
	for(unsigned int i=0;i<meshes->size();i++)
	{
		std::vector<Vector3> vertices;
		BrushMesh *mesh = (*meshes)[i];
		bool bConvex = mesh->IsConvex();
		std::vector<Side*> *sides;
		mesh->GetSides(&sides);
		if(bConvex == true) // Create a single convex mesh
		{
			if(!vertices.empty())
			{
				physx::PxConvexMeshDesc meshDesc;
				meshDesc.points.count = vertices.size();
				meshDesc.points.stride = sizeof(Vector3);
				meshDesc.points.data = &vertices[0];
				meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
				meshDesc.vertexLimit = 256;
	
				physx::MemoryOutputStream buf;
				if(cooking->cookConvexMesh(meshDesc,buf))
				{
					physx::MemoryInputData input(buf.getData(),buf.getSize());
					physx::PxConvexMesh *convexMesh = physics->createConvexMesh(input);
					if(bDynamic == true)
					{
						if(physActor != NULL)
							physActor->createShape(physx::PxConvexMeshGeometry(convexMesh),*material);
						else
						{
							Vector3 *pos = GetPosition();
							auto *rot = GetOrientation();
							physx::PxTransform t(physx::PxVec3(pos->x,pos->y,pos->z),physx::PxQuat(rot->x,rot->y,rot->z,rot->w));
							physx::PxRigidDynamic *actor = game->CreatePhysXActor(physx::PxConvexMeshGeometry(convexMesh),*material,t);
							actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD,true);
							actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION,true);
							physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor,mass);
							if(bPhys == false)
							{
								bPhys = true;
								if(m_physObject != NULL)
									DestroyPhysicsObject();
								m_physObject = new DynamicPhysObj(this,actor);
							}
							else
								static_cast<DynamicPhysObj*>(m_physObject)->AddActor(actor);
							physActor = actor;
						}
					}
					else
					{
						if(physActor != NULL)
							physActor->createShape(physx::PxConvexMeshGeometry(convexMesh),*material);
						else
						{
							Vector3 *pos = GetPosition();
							auto *rot = GetOrientation();
							physx::PxTransform t(physx::PxVec3(pos->x,pos->y,pos->z),physx::PxQuat(rot->x,rot->y,rot->z,rot->w));
							physx::PxRigidStatic *actor = game->CreateStaticPhysXActor(physx::PxConvexMeshGeometry(convexMesh),*material,t);
							if(bPhys == false)
							{
								bPhys = true;
								if(m_physObject != NULL)
									DestroyPhysicsObject();
								m_physObject = new StaticPhysObj(this,actor);
							}
							physActor = actor;
						}
					}
				}
			}
		}
		else // It's a displacement, every side will be a seperate actor
		{
			for(unsigned j=0;j<sides->size();j++)
			{
				Side *side = (*sides)[j];
				std::vector<unsigned int> *triangles;
				std::vector<Vector3> *vertices;
				side->GetTriangles(&triangles);
				side->GetVertices(&vertices);
				std::vector<Vector3> triangleVerts;
				std::vector<int> indices;
				for(unsigned int i=0;i<triangles->size();i++)
				{
					triangleVerts.push_back((*vertices)[(*triangles)[i]]);
					indices.push_back(i);
				}

				physx::PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = triangleVerts.size();
				meshDesc.triangles.count = indices.size() /3;
				meshDesc.points.stride = sizeof(glm::mediump_float) *3;
				meshDesc.triangles.stride = sizeof(glm::mediump_float) *3;
				meshDesc.points.data = &triangleVerts[0][0];
				meshDesc.triangles.data = &indices[0];

				physx::MemoryOutputStream buf;
				if(cooking->cookTriangleMesh(meshDesc,buf))
				{
					physx::MemoryInputData input(buf.getData(),buf.getSize());
					physx::PxTriangleMesh *triangleMesh = physics->createTriangleMesh(input);

					if(physActor != NULL)
						physActor->createShape(physx::PxTriangleMeshGeometry(triangleMesh),*material);
					else
					{
						Vector3 *pos = GetPosition();
						auto *rot = GetOrientation();
						physx::PxTransform t(physx::PxVec3(pos->x,pos->y,pos->z),physx::PxQuat(rot->x,rot->y,rot->z,rot->w));
						physx::PxRigidStatic *actor = game->CreateStaticPhysXActor(physx::PxTriangleMeshGeometry(triangleMesh),*material,t);
						if(bPhys == false)
						{
							bPhys = true;
							if(m_physObject != NULL)
								DestroyPhysicsObject();
							m_physObject = new StaticPhysObj(this,actor);
						}
						physActor = actor;
					}
				}
			}
		}
	}
	if(bPhys == false)
		return NULL;
	if(bDynamic == true)
	{
		m_physicsType = PHYSICSTYPE::DYNAMIC;
		SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::Dynamic | CollisionMask::Generic);
	}
	else
	{
		m_physicsType = PHYSICSTYPE::STATIC;
		SetCollisionFilter(CollisionMask::Static | CollisionMask::Generic | CollisionMask::Particle,CollisionMask::Static | CollisionMask::Generic);
	}
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}

PhysObj *BasePhysicsComponent::InitializeBoxControllerPhysics()
{
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();
	physx::PxControllerManager *manager = game->GetPhysXControllerManager();
	physx::PxPhysics *physics = state->GetPhysics();
	physx::PxMaterial *material = physics->createMaterial(0.5f,0.5f,0.1f);

	Vector3 *pos = GetPosition();
	Vector3 min,max;
	GetCollisionBounds(&min,&max);
	physx::PxBoxControllerDesc desc;
	desc.halfForwardExtent = (max.x -min.x) *0.5f;
	desc.halfHeight = (max.z -min.z) *0.5f;
	desc.halfSideExtent = (max.y -min.y) *0.5f;
	desc.material = material;
	desc.contactOffset = 0.1f;
	desc.position = physx::PxExtendedVec3(pos->x,pos->y,pos->z);
	desc.reportCallback = game->GetPhysXControllerHitReport();
	physx::PxController *controller = manager->createController(desc);
	if(controller == NULL)
		return NULL;
	physx::PxBoxController *boxController = static_cast<physx::PxBoxController*>(controller);
	physx::PxRigidDynamic *actor = boxController->getActor();
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD,true);
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION,true);
	m_physObject = new BoxControllerPhysObj(this,boxController);
	m_physicsType = PHYSICSTYPE::BOXCONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::Dynamic | CollisionMask::Generic);
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
PhysObj *BasePhysicsComponent::InitializeCapsuleControllerPhysics()
{
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();

	Vector3 *pos = GetPosition();
	Vector3 min,max;
	GetCollisionBounds(&min,&max);
	physx::PxControllerManager *manager = game->GetPhysXControllerManager();
	physx::PxPhysics *physics = state->GetPhysics();
	physx::PxMaterial *material = physics->createMaterial(0.5f,0.5f,0.1f);

	physx::PxCapsuleControllerDesc desc;
	float r = Math::Max(max.x -min.x,max.z -min.z) *0.5f;
	desc.height = (max.y -min.y) *0.5f;
	desc.radius = r;
	desc.material = material;
	desc.contactOffset = 0.1f;
	desc.position = physx::PxExtendedVec3(pos->x,pos->y,pos->z);
	desc.reportCallback = game->GetPhysXControllerHitReport();
	desc.behaviorCallback = new WVPxControllerBehaviorCallback;
#pragma message ("URGENT: Remove me!!!")
	physx::PxController *controller = manager->createController(desc);
	if(controller == NULL)
		return NULL;
	physx::PxCapsuleController *capsuleController = static_cast<physx::PxCapsuleController*>(controller);
	physx::PxRigidDynamic *actor = capsuleController->getActor();
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD,true);
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION,true);
	m_physObject = new CapsuleControllerPhysObj(this,capsuleController);
	m_physicsType = PHYSICSTYPE::CAPSULECONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic,CollisionMask::Dynamic | CollisionMask::Generic);
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
#endif

void BasePhysicsComponent::AddBrushMesh(const std::shared_ptr<BrushMesh> &mesh) {m_brushMeshes.push_back(mesh);}
void BasePhysicsComponent::InitializeBrushGeometry() {}

std::vector<std::shared_ptr<BrushMesh>> &BasePhysicsComponent::GetBrushMeshes() {return m_brushMeshes;}
const std::vector<std::shared_ptr<BrushMesh>> &BasePhysicsComponent::GetBrushMeshes() const {return const_cast<BasePhysicsComponent*>(this)->GetBrushMeshes();}

void BasePhysicsComponent::OnPhysicsWake(PhysObj*) {GetEntity().MarkForSnapshot(true);}
void BasePhysicsComponent::OnPhysicsSleep(PhysObj*) {}

PhysObj *BasePhysicsComponent::GetPhysicsObject() const {return m_physObject.get();}

void BasePhysicsComponent::InitializePhysObj()
{
	auto &phys = m_physObject;
	if(m_collisionFilterGroup != CollisionMask::Default)
		phys->SetCollisionFilter(m_collisionFilterGroup);
	if(m_collisionFilterMask != CollisionMask::Default)
		phys->SetCollisionFilterMask(m_collisionFilterMask);
#ifdef PHYS_ENGINE_PHYSX
	if(GetCollisionCallbacksEnabled() == true)
		SetCollisionCallbacksEnabled(true);
	if(GetCollisionContactReportEnabled() == true)
		SetCollisionContactReportEnabled(true);
#endif
	if(IsKinematic())
		SetKinematic(true);
	phys->SetStatic((GetPhysicsType() == PHYSICSTYPE::STATIC) ? true : false);
	auto pVelComponent = GetEntity().GetComponent<pragma::VelocityComponent>();
	phys->SetLinearVelocity(pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3{});
	UpdateCCD();
	//auto pos = GetPosition();
	//auto *o = phys->GetCollisionObject();
	//if(o != nullptr)
	//	pos -= o->GetOrigin();
	//SetPosition(pos);//,true); // Update our position, since our origin may have changed
}
PhysObj *BasePhysicsComponent::InitializePhysics(PhysConvexShape &shape,float mass)
{
	//btScalar contactProcessingThreshold = BT_LARGE_FLOAT;
	auto *body = CreateRigidBody(shape.shared_from_this(),mass);

	if(m_physObject != NULL)
		DestroyPhysicsObject();
	m_physObject = std::make_shared<RigidPhysObj>(this,body);
	auto group = GetCollisionFilter();
	auto mask = GetCollisionFilterMask();
	if(mass == 0.f)
	{
		m_physicsType = PHYSICSTYPE::STATIC;
		if(group == CollisionMask::Default)
			group = CollisionMask::Static | CollisionMask::Generic;
		if(mask == CollisionMask::Default)
			mask = CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item;
		SetMoveType(MOVETYPE::NONE);
	}
	else
	{
		m_physicsType = PHYSICSTYPE::DYNAMIC;
		if(group == CollisionMask::Default)
			group = CollisionMask::Dynamic | CollisionMask::Generic;
		if(mask == CollisionMask::Default)
			mask = CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item;
		SetMoveType(MOVETYPE::PHYSICS);
	}
	if(group != CollisionMask::Default)
		SetCollisionFilterGroup(group);
	if(mask != CollisionMask::Default)
		SetCollisionFilterMask(mask);
	auto &ent = GetEntity();
	ent.AddComponent<pragma::VelocityComponent>();
	ent.AddComponent<pragma::GravityComponent>();
	InitializePhysObj(); // Has to be called BEFORE the physics object is spawned, to make sure the proper collision group and masks are set!
	m_physObject->Spawn();
	OnPhysicsInitialized();
	return m_physObject.get();
}
PhysObj *BasePhysicsComponent::InitializePhysics(PHYSICSTYPE type)
{
	if(m_physObject != NULL)
		DestroyPhysicsObject();
	if(type != PHYSICSTYPE::STATIC)
	{
		auto &ent = GetEntity();
		ent.AddComponent<pragma::VelocityComponent>();
		ent.AddComponent<pragma::GravityComponent>();
	}
	switch(type)
	{
		case PHYSICSTYPE::BOXCONTROLLER:
			return InitializeBoxControllerPhysics().get();
		case PHYSICSTYPE::CAPSULECONTROLLER:
			return InitializeCapsuleControllerPhysics().get();
		case PHYSICSTYPE::STATIC:
		case PHYSICSTYPE::DYNAMIC:
		{
			auto mdlComponent = GetEntity().GetModelComponent();
			auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			if(hMdl != nullptr)
			{
				Model *mdl = hMdl.get();
				auto &meshes = mdl->GetCollisionMeshes();
				if(!meshes.empty())
					return InitializeModelPhysics((type == PHYSICSTYPE::DYNAMIC) ? true : false).get();
			}
			return nullptr;//InitializeBrushPhysics((type == PHYSICSTYPE::DYNAMIC) ? true : false); // Obsolete?
		}
		case PHYSICSTYPE::SOFTBODY:
			return InitializeSoftBodyPhysics().get();
	}
	return nullptr;
}
void BasePhysicsComponent::DestroyPhysicsObject()
{
	if(m_physObject == NULL)
		return;
	for(auto it=m_joints.begin();it!=m_joints.end();++it)
	{
		auto &hConstraint = it->constraint;
		if(hConstraint.IsValid())
			hConstraint->Remove();
	}
	//auto pos = GetPosition();
	//auto *physRoot = m_physObject->GetCollisionObject();
	//pos += physRoot->GetOrigin() *physRoot->GetRotation();
	auto &collisionObjs = m_physObject->GetCollisionObjects();
	PhysCollisionObject *root = nullptr;
	Vector3 posRoot {0.f,0.f,0.f};
	Vector3 originRoot {0.f,0.f,0.f};
	auto rotRoot = uquat::identity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!collisionObjs.empty())
	{
		posRoot = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
		root = collisionObjs.front().get();
		originRoot = root->GetOrigin();
		rotRoot = root->GetRotation();
	}

	m_physObject = nullptr;
	m_joints.clear();
	m_physObject = NULL;
	m_physicsType = PHYSICSTYPE::NONE;
	umath::set_flag(m_stateFlags,StateFlags::Ragdoll,false);
	GetEntity().RemoveComponent("softbody");

	if(root != nullptr)
	{
		//posRoot += originRoot *rotRoot;//uquat::get_inverse(rotRoot);
		if(pTrComponent.valid())
			pTrComponent->SetPosition(posRoot,true);
	}
	//SetPosition(pos,true);
	OnPhysicsDestroyed();
}
void BasePhysicsComponent::OnPhysicsInitialized() {BroadcastEvent(EVENT_ON_PHYSICS_INITIALIZED);}
void BasePhysicsComponent::OnPhysicsDestroyed() {BroadcastEvent(EVENT_ON_PHYSICS_DESTROYED);}
bool BasePhysicsComponent::IsKinematic() const {return umath::is_flag_set(m_stateFlags,StateFlags::Kinematic);}
void BasePhysicsComponent::SetKinematic(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::Kinematic,b);
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || phys->IsStatic())
		return;
	PhysObjKinematic *kinematic = dynamic_cast<PhysObjKinematic*>(phys);
	kinematic->SetKinematic(b);
}
bool BasePhysicsComponent::ShouldCollide(PhysObj*,PhysCollisionObject*,BaseEntity *entOther,PhysObj*,PhysCollisionObject*,bool valDefault) const
{
	if(m_customCollisions.empty())
		return valDefault;
	auto it = const_cast<BasePhysicsComponent*>(this)->FindCollisionInfo(entOther);
	if(it == m_customCollisions.end())
		return valDefault;
	return it->shouldCollide;
}

BasePhysicsComponent::CollisionInfo::CollisionInfo()
	: shouldCollide(false)
{}
BasePhysicsComponent::CollisionInfo::CollisionInfo(BaseEntity *_ent,Bool _shouldCollide)
	: entity(_ent->GetHandle()),shouldCollide(_shouldCollide)
{}
std::vector<BasePhysicsComponent::CollisionInfo>::iterator BasePhysicsComponent::FindCollisionInfo(BaseEntity *ent)
{
	return std::find_if(m_customCollisions.begin(),m_customCollisions.end(),[ent](CollisionInfo &info) {
		return (ent == info.entity.get()) ? true : false;
	});
}
BasePhysicsComponent::PhysJoint::PhysJoint(unsigned int _source,unsigned int _target)
	: source(_source),target(_target)
{}
void BasePhysicsComponent::ResetCollisions(BaseEntity *ent)
{
	auto it = FindCollisionInfo(ent);
	if(it == m_customCollisions.end())
		return;
	m_customCollisions.erase(it);
}
void BasePhysicsComponent::SetCollisionsEnabled(BaseEntity *ent,bool b)
{
	for(auto it=m_customCollisions.begin();it!=m_customCollisions.end();)
	{
		auto &info = *it;
		if(!info.entity.IsValid())
			it = m_customCollisions.erase(it);
		else
			++it;
	}
	auto it = FindCollisionInfo(ent);
	if(it != m_customCollisions.end())
		it->shouldCollide = b;
	else
		m_customCollisions.push_back(CollisionInfo(ent,b));
	UpdateCCD();
}
void BasePhysicsComponent::DisableCollisions(BaseEntity *ent) {SetCollisionsEnabled(ent,false);}
void BasePhysicsComponent::EnableCollisions(BaseEntity *ent) {SetCollisionsEnabled(ent,true);}
void BasePhysicsComponent::UpdateCCD()
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	auto bEnableCcd = m_customCollisions.empty(); // CCD doesn't work with custom collisions; Disable it if there are any
	for(auto &hObj : phys->GetCollisionObjects())
	{
		if(hObj.IsValid() == false)
			continue;
		hObj->SetCCDEnabled(bEnableCcd);
	}
}

void BasePhysicsComponent::ApplyForce(const Vector3 &force)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyForce(force);
}
void BasePhysicsComponent::ApplyForce(const Vector3 &force,const Vector3 &relPos)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyForce(force,relPos);
}
void BasePhysicsComponent::ApplyImpulse(const Vector3 &impulse)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyImpulse(impulse);
}
void BasePhysicsComponent::ApplyImpulse(const Vector3 &impulse,const Vector3 &relPos)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyImpulse(impulse,relPos);
}
void BasePhysicsComponent::ApplyTorque(const Vector3 &torque)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyTorque(torque);
}
void BasePhysicsComponent::ApplyTorqueImpulse(const Vector3 &torque)
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	phys->ApplyTorqueImpulse(torque);
}
float BasePhysicsComponent::GetMass() const
{
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return 0.f;
	return phys->GetMass();
}
