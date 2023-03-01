/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/physobj.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/collisionmesh.h"
#include <pragma/physics/movetypes.h>
#include "pragma/physics/constraint.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/model.h"
#include "pragma/entities/components/basegravity.h"
#include "pragma/game/game_coordinate_system.hpp"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

using namespace pragma;

extern DLLNETWORK Engine *engine;


uint32_t pragma::physics::PhysObjCreateInfo::AddShape(pragma::physics::IShape &shape,const umath::Transform &localPose,BoneId boneId)
{
	if(shape.IsCompoundShape())
	{
		uint32_t numShapes = 0;
		for(auto &subShapeInfo : shape.GetCompoundShape()->GetShapes())
			numShapes += AddShape(*subShapeInfo.shape,localPose *subShapeInfo.localPose,boneId);
		return numShapes;
	}
	auto it = m_shapes.find(boneId);
	if(it == m_shapes.end())
		it = m_shapes.insert(std::make_pair(boneId,std::vector<ShapeInfo>{})).first;
	it->second.push_back({});
	auto &shapeInfo = it->second.back();
	shapeInfo.shape = std::static_pointer_cast<physics::IShape>(shape.shared_from_this());
	shapeInfo.localPose = localPose;
	return 1;
}
void pragma::physics::PhysObjCreateInfo::SetModelMeshBoneMapping(MeshIndex modelMeshIndex,BoneId boneIndex)
{
	m_modelMeshIndexToShapeIndex[modelMeshIndex] = boneIndex;
}
void pragma::physics::PhysObjCreateInfo::SetModel(Model &model)
{
	m_model = model.GetHandle();
}
Model *pragma::physics::PhysObjCreateInfo::GetModel() const {return m_model.get();}
const std::unordered_map<pragma::physics::PhysObjCreateInfo::BoneId,std::vector<pragma::physics::PhysObjCreateInfo::ShapeInfo>> &pragma::physics::PhysObjCreateInfo::GetShapes() const {return m_shapes;}
const std::unordered_map<pragma::physics::PhysObjCreateInfo::MeshIndex,pragma::physics::PhysObjCreateInfo::BoneId> &pragma::physics::PhysObjCreateInfo::GetModelMeshBoneMappings() const {return m_modelMeshIndexToShapeIndex;}

/////////////

util::TSharedHandle<pragma::physics::IRigidBody> BasePhysicsComponent::CreateRigidBody(pragma::physics::IShape &shape,bool dynamic,const umath::Transform &localPose)
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto body = physEnv->CreateRigidBody(shape,dynamic);
	if(body == nullptr)
		return nullptr;
	body->TransformLocalPose(localPose);
	auto originOffset = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
	auto rot = (pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity()) *localPose.GetRotation();
	auto tOrigin = localPose.GetOrigin();
	uvec::rotate(&tOrigin,rot);
	umath::Transform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(/*-tOrigin +*/originOffset);
	startTransform.SetRotation(rot);
	auto contactProcessingThreshold = 1e30;

	auto group = GetCollisionFilter();
	if(group != CollisionMask::Default)
		body->SetCollisionFilterGroup(group);
	auto mask = GetCollisionFilterMask();
	if(mask != CollisionMask::Default)
		body->SetCollisionFilterMask(mask);
	body->SetWorldTransform(startTransform);
	body->SetContactProcessingThreshold(CFloat(contactProcessingThreshold));
	if(shape.GetMass() == 0.f)
	{
		body->SetStatic(true);
		body->UpdateAABB();
		body->SetActivationState(pragma::physics::ICollisionObject::ActivationState::AlwaysInactive);
	}
	return body;
}

PhysObjHandle BasePhysicsComponent::InitializePhysics(const physics::PhysObjCreateInfo &physObjCreateInfo,PhysFlags flags,int32_t rootMeshBoneId)
{
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || mdlComponent->HasModel() == false)
		return {};

	auto &physObjShapes = physObjCreateInfo.GetShapes();
	if(physObjShapes.empty())
		return {};

	auto bPhys = false;
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto *mdl = physObjCreateInfo.GetModel();
	auto *joints = mdl ? &mdl->GetJoints() : nullptr;

	std::unordered_map<physics::PhysObjCreateInfo::BoneId,util::TSharedHandle<physics::IRigidBody>> boneIdToRigidBody = {};
	auto itRoot = physObjShapes.find(rootMeshBoneId); // Find root mesh
	auto it = itRoot;
	auto bHasRoot = (it != physObjShapes.end()) ? true : false;
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
					it = physObjShapes.begin();
					if(it == itRoot)
						++it;
				}
				else
					bUseRoot = false;
			}
		}
		else
			++it;
		if(it == physObjShapes.end())
			break;
		else if(it == itRoot && bHasRoot == false)
			continue;
		//auto bone = it->first;
		auto &sortedShape = it->second;
		auto &shapes = sortedShape;
		std::shared_ptr<pragma::physics::IShape> shape;
		umath::Transform localPose {};
		if(shapes.size() == 1)
		{
			auto &shapeInfo = shapes.front();
			shape = shapeInfo.shape.lock();
			localPose = shapeInfo.localPose;
		}
		else
		{
			if(umath::is_flag_set(flags,PhysFlags::Dynamic) == false)
			{
				/*for(auto it=shapes.begin();it!=shapes.end();++it)
				{
					auto shape = it->shape.lock();
					if(shape == nullptr)
						continue;
					auto body = CreateRigidBody(*shape,umath::is_flag_set(flags,PhysFlags::Dynamic),it->localPose);
					if(body == nullptr)
						continue;
					if(bPhys == false)
					{
						bPhys = true;
						if(m_physObject != nullptr)
							DestroyPhysicsObject();
						m_physObject = PhysObj::Create<RigidPhysObj,pragma::physics::IRigidBody&>(*this,*body);
					}
					else
						m_physObject->AddCollisionObject(*body);
				}*/
				// Compound mesh for all geometry; This may cause a crash in btAdjustInternalEdgeContacts
				// when bullet physics is used? If so, try the code above instead!
				shape = physEnv->CreateCompoundShape();
				auto cmpShape = std::dynamic_pointer_cast<pragma::physics::ICompoundShape>(shape);
				for(auto it=shapes.begin();it!=shapes.end();++it)
				{
					auto subShape = it->shape.lock();
					if(subShape == nullptr)
						continue;
					cmpShape->AddShape(*subShape,it->localPose);
				}
				auto body = CreateRigidBody(*cmpShape,umath::is_flag_set(flags,PhysFlags::Dynamic));
				if(bPhys == false)
				{
					bPhys = true;
					if(m_physObject != nullptr)
						DestroyPhysicsObject();
					m_physObject = util::to_shared_handle<PhysObj>(PhysObj::Create<RigidPhysObj,pragma::physics::IRigidBody&>(*this,*body));
				}
				else
				m_physObject->AddCollisionObject(*body);
				continue;
			}
			shape = physEnv->CreateCompoundShape();
			if(shape)
			{
				auto cmpShape = std::dynamic_pointer_cast<pragma::physics::ICompoundShape>(shape);
				for(auto it=shapes.begin();it!=shapes.end();++it)
				{
					auto subShape = it->shape.lock();
					if(subShape == nullptr)
						continue;
					cmpShape->AddShape(*subShape,it->localPose);
				}
			}
		}
		if(shape == nullptr)
			continue;
		auto body = CreateRigidBody(*shape,umath::is_flag_set(flags,PhysFlags::Dynamic),localPose);
		if(body == nullptr)
			continue;
		body->SetBoneID(it->first);
		if(it->first >= 0 && umath::is_flag_set(flags,PhysFlags::Dynamic) == true && joints && joints->empty() == false) // Static physics and non-ragdolls can still play animation
			umath::set_flag(m_stateFlags,StateFlags::Ragdoll);
		boneIdToRigidBody[it->first] = body;
		if(bPhys == false)
		{
			bPhys = true;
			if(m_physObject != nullptr)
				DestroyPhysicsObject();
			m_physObject = util::to_shared_handle<PhysObj>(PhysObj::Create<RigidPhysObj,pragma::physics::IRigidBody&>(*this,*body));
		}
		else
			m_physObject->AddCollisionObject(*body);
	}
	auto &modelMeshIndexToShapeIndex = physObjCreateInfo.GetModelMeshBoneMappings();
	if(joints)
	{
		for(auto it=joints->begin();it!=joints->end();++it)
		{
			auto &joint = *it;
			auto itSrc = modelMeshIndexToShapeIndex.find(joint.parent);//joint.src); // TODO: Swap variable names
			auto itDest = modelMeshIndexToShapeIndex.find(joint.child);
			if(itSrc != modelMeshIndexToShapeIndex.end() && itDest != modelMeshIndexToShapeIndex.end())
			{
				auto it0 = physObjShapes.find(itSrc->second);
				auto it1 = physObjShapes.find(itDest->second);
				if(it0 == physObjShapes.end() || it1 == physObjShapes.end())
					continue;
				auto &src = *it0; // Bone Id
				auto &dest = *it1; // Bone Id
				auto itBodySrc = boneIdToRigidBody.find(src.first);
				auto itBodyDst = boneIdToRigidBody.find(dest.first);
				auto bodySrc = (itBodySrc != boneIdToRigidBody.end()) ? itBodySrc->second : nullptr;
				auto bodyTgt = (itBodyDst != boneIdToRigidBody.end()) ? itBodyDst->second : nullptr;
				if(bodySrc == nullptr || bodyTgt == nullptr)
					continue;
				util::TSharedHandle<pragma::physics::IConstraint> c = nullptr;

				auto boneId = itDest->second;

				auto &pose = mdl->GetReference();
				// Constraint position/rotation
				auto posConstraint = *pose.GetBonePosition(boneId);
				auto rotConstraint = *pose.GetBoneOrientation(boneId);

				auto posTgt = posConstraint +bodyTgt->GetOrigin();

				posConstraint = posConstraint +bodySrc->GetOrigin();
				//Con::cerr<<"Constraint for bone "<<boneId<<": ("<<posConstraint.x<<","<<posConstraint.y<<","<<posConstraint.z<<") ("<<posTgt.x<<","<<posTgt.y<<","<<posTgt.z<<") "<<Con::endl;
				//
				if(joint.type == JointType::Fixed)
					c = util::shared_handle_cast<pragma::physics::IFixedConstraint,pragma::physics::IConstraint>(physEnv->CreateFixedConstraint(*bodySrc,posConstraint,uquat::identity(),*bodyTgt,posTgt,uquat::identity()));
				else if(joint.type == JointType::ConeTwist)
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
					auto ct = physEnv->CreateConeTwistConstraint(*bodySrc,posConstraint,rotConstraint,*bodyTgt,posTgt,rotOffset);
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
						CFloat(umath::deg_to_rad(twistSpan))
					);
					ct->SetSoftness(softness);
					ct->SetDamping(relaxationFactor);
					c = util::shared_handle_cast<pragma::physics::IConeTwistConstraint,pragma::physics::IConstraint>(ct);
				}
				else if(joint.type == JointType::DOF)
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

#if 0
					auto ct = physEnv->CreateDoFSpringConstraint(*bodyTgt,posTgt,rotConstraint,*bodySrc,posConstraint,rotConstraint);
					//auto *ct = physEnv->CreateDoFConstraint(bodyTgt,posTgt,rotConstraint,bodySrc,posConstraint,rotConstraint);

					//ct->SetLinearLimit(limitLinMin,limitLinMax);
					//ct->SetAngularLimit(limitAngMin,limitAngMax);
					ct->SetLinearLowerLimit(limitLinMin);
					ct->SetLinearUpperLimit(limitLinMax);

					ct->SetAngularLowerLimit(Vector3(umath::deg_to_rad(limitAngMin.p),umath::deg_to_rad(limitAngMin.y),umath::deg_to_rad(limitAngMin.r)));
					ct->SetAngularUpperLimit(Vector3(umath::deg_to_rad(limitAngMax.p),umath::deg_to_rad(limitAngMax.y),umath::deg_to_rad(limitAngMax.r)));
#endif
					auto ct = physEnv->CreateConeTwistConstraint(*bodyTgt,posTgt,rotConstraint,*bodySrc,posConstraint,rotConstraint);
					//auto *ct = physEnv->CreateDoFConstraint(bodyTgt,posTgt,rotConstraint,bodySrc,posConstraint,rotConstraint);

					ct->SetLimit(
						Vector3(umath::deg_to_rad(limitAngMin.p),umath::deg_to_rad(limitAngMin.y),umath::deg_to_rad(limitAngMin.r)),
						Vector3(umath::deg_to_rad(limitAngMax.p),umath::deg_to_rad(limitAngMax.y),umath::deg_to_rad(limitAngMax.r))
					);

					/*auto ct = physEnv->CreateConeTwistConstraint(*bodyTgt,posTgt,rotConstraint,*bodySrc,posConstraint,rotConstraint);
					ct->SetLimit(span1,span2,twistSpan);
					ct->SetLinearLowerLimit(limitLinMin);
					ct->SetLinearUpperLimit(limitLinMax);

					ct->SetAngularLowerLimit(Vector3(umath::deg_to_rad(limitAngMin.p),umath::deg_to_rad(limitAngMin.y),umath::deg_to_rad(limitAngMin.r)));
					ct->SetAngularUpperLimit(Vector3(umath::deg_to_rad(limitAngMax.p),umath::deg_to_rad(limitAngMax.y),umath::deg_to_rad(limitAngMax.r)));

					c = util::shared_handle_cast<pragma::physics::IDoFSpringConstraint,pragma::physics::IConstraint>(ct);*/
				}
				if(c != nullptr)
				{
					c->SetCollisionsEnabled(joint.collide);
					m_joints.push_back(PhysJoint(itSrc->second,itDest->second));
					auto &jointData = m_joints.back();
					jointData.constraint = c;
				}
			}
		}
	}
	if(bPhys == false)
		return {};
	// Collision group/mask have to be set before spawning the physics object, otherwise
	// there may be incorrect collisions directly after spawn.
	if(umath::is_flag_set(flags,PhysFlags::Dynamic) == true)
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
	m_physObject->Spawn();
	auto &collisionObjs = m_physObject->GetCollisionObjects();
	auto animComponent = ent.GetAnimatedComponent();
	pragma::physics::ICollisionObject *root = nullptr;
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto posRoot = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
	if(!collisionObjs.empty())
	{
		auto &hRoot = collisionObjs.front();
		if(hRoot.IsValid())
		{
			root = hRoot.Get();
			auto boneId = root->GetBoneID();
			if(animComponent.valid() && animComponent->GetLocalBonePosition(boneId,posRoot) == true && pTrComponent != nullptr)
				uvec::local_to_world(pTrComponent->GetPosition(),pTrComponent->GetRotation(),posRoot); // World space position of root collision object bone
		}
	}

	// Move all collision objects to their respective bone counterparts (To keep our previous pose)
	std::function<void(std::unordered_map<uint32_t,std::shared_ptr<panima::Bone>>&)> fUpdateCollisionObjects;
	fUpdateCollisionObjects = [this,&fUpdateCollisionObjects](std::unordered_map<uint32_t,std::shared_ptr<panima::Bone>> &bones) {
		for(auto &it : bones)
		{
			UpdateBoneCollisionObject(it.first,true,true);
			fUpdateCollisionObjects(it.second->children);
		}
	};

	if(mdl)
	{
		auto &skeleton = mdl->GetSkeleton();
		//auto &rootBones = skeleton.GetRootBones();
		//fUpdateCollisionObjects(rootBones);

		if(m_joints.empty() == false) // Is this a ragdoll?
		{
			// Set collision object positions to current bone positions to
			// transition the current animation smoothly into the ragdoll
			auto &reference = mdl->GetReference();
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
				if(pTrComponent != nullptr)
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
			auto refAnim = mdl->GetAnimation(0);
			if(refAnim != nullptr)
			{
				auto frame = refAnim->GetFrame(0); // Reference frame with local bone transformations
				if(frame != nullptr)
				{
					auto &bones = skeleton.GetBones();
					for(auto &bone : bones)
					{
						auto it = physObjShapes.find(bone->ID);
						if(it == physObjShapes.end())
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
	}

	InitializePhysObj();
	OnPhysicsInitialized();
	return PhysObjHandle{m_physObject};
}

util::TSharedHandle<PhysObj> BasePhysicsComponent::InitializeModelPhysics(PhysFlags flags)
{
	physics::PhysObjCreateInfo physObjCreateInfo {};

	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || mdlComponent->HasModel() == false)
		return {};
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	auto &meshes = hMdl->GetCollisionMeshes();
	if(meshes.empty())
		return {};
	physObjCreateInfo.SetModel(*hMdl);
	unsigned int meshId = 0;
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent != nullptr ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	auto bScale = (scale != Vector3{1.f,1.f,1.f}) ? true : false;
	for(auto &mesh : meshes)
	{
		auto shape = (bScale == false) ? mesh->GetShape() : mesh->CreateShape(scale);
		if(shape != nullptr)
		{
			auto bone = mesh->GetBoneParent();
			// Note: Collision mesh origin has already been applied as local pose to the shape when it was created,
			// so we don't need to define any pose transform here!
			physObjCreateInfo.AddShape(*shape,umath::Transform{},bone);
			physObjCreateInfo.SetModelMeshBoneMapping(meshId,bone);
		}
		meshId++;
	}
	return util::claim_shared_handle_ownership(InitializePhysics(physObjCreateInfo,flags,meshes.front()->GetBoneParent()));
}

util::TSharedHandle<PhysObj> BasePhysicsComponent::InitializeBoxControllerPhysics()
{
	Vector3 min,max;
	GetCollisionBounds(&min,&max);
	auto origin = (min +max) /2.f;
	auto extents = max -min;

	m_physObject = util::to_shared_handle<PhysObj>(PhysObj::Create<BoxControllerPhysObj>(*this,extents *0.5f,24));
	if(m_physObject == nullptr)
		return {};
	m_physObject->GetCollisionObject()->SetOrigin(-origin);
	static_cast<BoxControllerPhysObj*>(m_physObject.get())->SetCollisionBounds(min,max);
	m_physicsType = PHYSICSTYPE::BOXCONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic | CollisionMask::Player,CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item);
	m_physObject->Spawn();
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}
util::TSharedHandle<PhysObj> BasePhysicsComponent::InitializeCapsuleControllerPhysics()
{
	Vector3 min,max;
	GetCollisionBounds(&min,&max);

	m_physObject = util::to_shared_handle<PhysObj>(PhysObj::Create<CapsuleControllerPhysObj>(*this,CUInt32(umath::max(max.x -min.x,max.z -min.z)),CUInt32(max.y -min.y),24));
	if(m_physObject == nullptr)
		return {};
	m_physicsType = PHYSICSTYPE::CAPSULECONTROLLER;
	SetCollisionFilter(CollisionMask::Dynamic | CollisionMask::Generic | CollisionMask::Player,CollisionMask::All &~CollisionMask::Particle &~CollisionMask::Item);
	m_physObject->Spawn();
	InitializePhysObj();
	OnPhysicsInitialized();
	return m_physObject;
}

void BasePhysicsComponent::ClearAwakeStatus()
{
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &awakePhysC = game.GetAwakePhysicsComponents();
	auto it = std::find_if(awakePhysC.begin(),awakePhysC.end(),[this](const pragma::ComponentHandle<pragma::BasePhysicsComponent> &hPhysC) {
		return this == hPhysC.get();
	});
	if(it == awakePhysC.end())
		return;
	awakePhysC.erase(it);
}
void BasePhysicsComponent::OnPhysicsWake(PhysObj*)
{
	GetEntity().MarkForSnapshot(true);

	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &awakePhysC = game.GetAwakePhysicsComponents();
	auto it = std::find_if(awakePhysC.begin(),awakePhysC.end(),[this](const pragma::ComponentHandle<pragma::BasePhysicsComponent> &hPhysC) {
		return this == hPhysC.get();
	});
	if(it != awakePhysC.end())
	{
		// Con::cwar<<"Physics component has woken up, but was already marked as awake previously!"<<Con::endl;
		return;
	}
	awakePhysC.push_back(GetHandle<BasePhysicsComponent>());
}
void BasePhysicsComponent::OnPhysicsSleep(PhysObj*)
{
	if(AreForcePhysicsAwakeCallbacksEnabled())
		return;
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &awakePhysC = game.GetAwakePhysicsComponents();
	auto it = std::find_if(awakePhysC.begin(),awakePhysC.end(),[this](const pragma::ComponentHandle<pragma::BasePhysicsComponent> &hPhysC) {
		return this == hPhysC.get();
	});
	if(it == awakePhysC.end())
		Con::cwar<<"Physics component has fallen asleep, but was already marked as asleep previously!"<<Con::endl;
	ClearAwakeStatus();
}

PhysObj *BasePhysicsComponent::GetPhysicsObject() const {return const_cast<PhysObj*>(m_physObject.get());}

void BasePhysicsComponent::InitializePhysObj()
{
	auto &phys = m_physObject;
	if(m_collisionFilterGroup != CollisionMask::Default)
		phys->SetCollisionFilter(m_collisionFilterGroup);
	if(m_collisionFilterMask != CollisionMask::Default)
		phys->SetCollisionFilterMask(m_collisionFilterMask);
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
PhysObj *BasePhysicsComponent::InitializePhysics(pragma::physics::IConvexShape &shape,PhysFlags flags)
{
	//btScalar contactProcessingThreshold = BT_LARGE_FLOAT;
	auto bDynamic = umath::is_flag_set(flags,PhysFlags::Dynamic);
	auto body = CreateRigidBody(shape,bDynamic);

	if(m_physObject != NULL)
		DestroyPhysicsObject();
	m_physObject = util::to_shared_handle<PhysObj>(PhysObj::Create<RigidPhysObj,pragma::physics::IRigidBody&>(*this,*body));
	auto group = GetCollisionFilter();
	auto mask = GetCollisionFilterMask();
	if(bDynamic)
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
PhysObj *BasePhysicsComponent::InitializePhysics(PHYSICSTYPE type,PhysFlags flags)
{
	umath::set_flag(flags,PhysFlags::Dynamic,type != PHYSICSTYPE::STATIC);
	if(m_physObject != NULL)
		DestroyPhysicsObject();
	if(type != PHYSICSTYPE::STATIC)
	{
		auto &ent = GetEntity();
		ent.AddComponent<pragma::VelocityComponent>();
		ent.AddComponent<pragma::GravityComponent>();
	}
	auto evInitPhysics = CEInitializePhysics{type,flags};
	if(BroadcastEvent(EVENT_INITIALIZE_PHYSICS,evInitPhysics) == util::EventReply::Handled)
		return GetPhysicsObject(); // Handled by an external component
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
			auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			if(hMdl != nullptr)
			{
				Model *mdl = hMdl.get();
				auto &meshes = mdl->GetCollisionMeshes();
				if(!meshes.empty())
					return InitializeModelPhysics(flags).get();
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
			hConstraint.Remove();
	}
	//auto pos = GetPosition();
	//auto *physRoot = m_physObject->GetCollisionObject();
	//pos += physRoot->GetOrigin() *physRoot->GetRotation();
	auto &collisionObjs = m_physObject->GetCollisionObjects();
	pragma::physics::ICollisionObject *root = nullptr;
	Vector3 posRoot {0.f,0.f,0.f};
	Vector3 originRoot {0.f,0.f,0.f};
	auto rotRoot = uquat::identity();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!collisionObjs.empty())
	{
		posRoot = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
		root = collisionObjs.front().Get();
		originRoot = root->GetOrigin();
		rotRoot = root->GetRotation();
	}

	m_physObject = nullptr;
	m_joints.clear();
	m_physObject = {};
	m_physicsType = PHYSICSTYPE::NONE;
	umath::set_flag(m_stateFlags,StateFlags::Ragdoll,false);
	GetEntity().RemoveComponent("softbody");

	if(root != nullptr)
	{
		//posRoot += originRoot *rotRoot;//uquat::get_inverse(rotRoot);
		if(pTrComponent != nullptr)
			pTrComponent->SetPosition(posRoot,true);
	}
	//SetPosition(pos,true);
	OnPhysicsDestroyed();
}
void BasePhysicsComponent::OnPhysicsInitialized()
{
	BroadcastEvent(EVENT_ON_PHYSICS_INITIALIZED);
	SetCollisionContactReportEnabled(GetCollisionContactReportEnabled());

	// Note: We always need sleep reports enabled for optimization purposes.
	// TODO: Remove IsSleepReportEnabled / SetSleepReportEnabled?
	SetSleepReportEnabled(true);
	if(AreForcePhysicsAwakeCallbacksEnabled())
		OnPhysicsWake(m_physObject.get());
	// SetSleepReportEnabled(IsSleepReportEnabled());
}
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
bool BasePhysicsComponent::ShouldCollide(PhysObj*,pragma::physics::ICollisionObject*,BaseEntity *entOther,PhysObj*,pragma::physics::ICollisionObject*,bool valDefault) const
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
		if(!info.entity.valid())
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

