#include "stdafx_shared.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/controller.hpp"
#include "pragma/physics/collision_object.hpp"
#include <pragma/game/game.h>
#include "pragma/physics/raytraces.h"
#include "pragma/math/util_engine_math.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include <BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h>

ControllerPhysObj::~ControllerPhysObj()
{
	//NetworkState *state = m_networkState;
	//Game *game = state->GetGameState();
	//PhysEnv *physEnv = game->GetPhysicsEnvironment();

	for(auto it=m_collisionObjects.begin();it!=m_collisionObjects.end();)
	{
		if(it->Get() == m_collisionObject.Get())
			it = m_collisionObjects.erase(it);
		else
			it++;
	}
	m_hitData.Clear();
}

bool ControllerPhysObj::SetGroundContactPoint(int32_t idx,const pragma::physics::ICollisionObject *o,const pragma::physics::ICollisionObject *oOther)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto d = PhysContactInfo::CalcXZDistance(contactPoint,idx);
	if(m_groundInfo.has_value())
		m_groundInfo->minContactDistance = umath::min(m_groundInfo->contactDistance,d) /PhysEnv::WORLD_SCALE;

	// Check if ground is walkable
	auto n = PhysContactInfo::GetContactNormal(uvec::create(contactPoint.m_normalWorldOnB),idx);
	auto charComponent = m_owner.valid() ? m_owner->GetEntity().GetCharacterComponent() : util::WeakHandle<pragma::BaseCharacterComponent>{};
	auto angle = umath::acos(uvec::dot(n,charComponent.valid() ? charComponent->GetUpDirection() : uvec::UP)); // TODO: Determine the up-direction from the rotation of collision object 'o' and skip the detour through the character component
	auto slopeLimit = GetMaxSlope();
	auto bGroundWalkable = (angle <= umath::deg_to_rad(slopeLimit));

	auto pos = uvec::create(((idx == 0) ? contactPoint.getPositionWorldOnA() : contactPoint.getPositionWorldOnB()) /PhysEnv::WORLD_SCALE);
	//GetOwner()->GetEntity().GetNetworkState()->GetGameState()->DrawLine(pos,pos +n *100.f,Color::Red,0.3f);
	if(bGroundWalkable == false)
	{
		// Discard this point if we already have a walkable point (even if this point is closer)
		if(m_groundInfo.has_value() && m_groundInfo->groundWalkable == true)
			return false;
	}

	if(m_groundInfo.has_value() == true && d >= m_groundInfo->contactDistance)
		return false; // We already have a better candidate, discard this one

	auto dCur = m_groundInfo.has_value() ? m_groundInfo->contactDistance : std::numeric_limits<double>::max();
	m_groundInfo = {GroundInfo{contactPoint,static_cast<int8_t>(idx)}};
	m_groundInfo->groundWalkable = bGroundWalkable;
	m_groundInfo->contactDistance = d;
	m_groundInfo->minContactDistance = umath::min(d,dCur) /PhysEnv::WORLD_SCALE;
	auto &contactInfo = m_groundInfo->contactInfo;

	auto *obj0 = static_cast<PhysCollisionObject*>(o->getUserPointer());
	auto *obj1 = static_cast<PhysCollisionObject*>(oOther->getUserPointer());
	contactInfo.contactObject0 = (obj0 != nullptr) ? obj0->GetHandle() : PhysCollisionObjectHandle{};
	contactInfo.contactObject1 = (obj1 != nullptr) ? obj1->GetHandle() : PhysCollisionObjectHandle{};

	SetCurrentFriction(CFloat(contactPoint.m_combinedFriction));
	auto *shape = oOther->getCollisionShape();
	if(shape->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
	{
		auto *mtShape = const_cast<btMultimaterialTriangleMeshShape*>(static_cast<const btMultimaterialTriangleMeshShape*>(shape));
		auto *surface = static_cast<const SurfaceMaterial*>(mtShape->getMaterialProperties((idx == 0) ? contactPoint.m_partId1 : contactPoint.m_partId0,(idx == 0) ? contactPoint.m_index1 : contactPoint.m_index0));
		if(surface != nullptr)
			contactInfo.surfaceMaterialId = CUInt32(surface->GetIndex());
	}
	else
	{
		auto *obj = static_cast<PhysCollisionObject*>(oOther->getUserPointer());
		if(obj != nullptr)
		{
			auto *surface = GetNetworkState()->GetGameState()->GetSurfaceMaterial(obj->GetSurfaceMaterial());
			if(surface != nullptr)
				contactInfo.surfaceMaterialId = CUInt32(surface->GetIndex());
		}
	}
	return true;
#else
	return false;
#endif
}
void ControllerPhysObj::ClearGroundContactPoint()
{
	m_groundInfo = {};
	SetCurrentFriction(1.f);
}

int32_t ControllerPhysObj::GetGroundSurfaceMaterial() const
{
	if(m_groundInfo.has_value() == false)
		return -1;
	return m_groundInfo->contactInfo.surfaceMaterialId;
}
bool ControllerPhysObj::IsOnGround() const {return m_groundInfo.has_value();}
bool ControllerPhysObj::IsGroundWalkable() const {return IsOnGround() && m_groundInfo->groundWalkable == true;}
const PhysContactInfo *ControllerPhysObj::GetGroundContactInfo() const {return m_groundInfo.has_value() ? &m_groundInfo->contactInfo : nullptr;}
double ControllerPhysObj::GetMinGroundXZContactDistance() const {return m_groundInfo.has_value() ? m_groundInfo->minContactDistance : std::numeric_limits<double>::max();}
BaseEntity *ControllerPhysObj::GetGroundEntity() const
{
	auto *o = GetGroundPhysObject();
	auto *pOwner = (o != nullptr) ? o->GetOwner() : nullptr;
	return (pOwner != nullptr) ? &pOwner->GetEntity() : nullptr;
}
PhysObj *ControllerPhysObj::GetGroundPhysObject() const
{
	auto *o = GetGroundPhysCollisionObject();
	return (o != nullptr) ? static_cast<PhysObj*>(o->userData) : nullptr;
}
pragma::physics::ICollisionObject *ControllerPhysObj::GetGroundPhysCollisionObject()
{
	if(m_groundInfo.has_value() == false)
		return nullptr;
	return m_groundInfo->contactInfo.contactObject1.Get();
}
const pragma::physics::ICollisionObject *ControllerPhysObj::GetGroundPhysCollisionObject() const {return const_cast<ControllerPhysObj*>(this)->GetGroundPhysCollisionObject();}

ControllerHitData &ControllerPhysObj::GetControllerHitData() {return m_hitData;}
void ControllerPhysObj::PostSimulate()
{
	PhysObjDynamic::PostSimulate();
	auto *owner = GetOwner();
	if(owner == nullptr)
		return;
	NetworkState *state = owner->GetEntity().GetNetworkState();
	Game *game = state->GetGameState();
	//PhysTransform t = m_ghostObject->GetWorldTransform();
	//auto shape = m_ghostObject->GetCollisionShape();

	// Overlap doesn't work properly in some cases (dynamic objects?)
	/*auto origin = t.GetOrigin();
	origin -= owner->GetUp() *0.5f;
	origin.y -= 0.5f;
	//auto pos = GetPosition();
	//pos.y -= 0.5f;
	t.SetOrigin(origin);

	TraceData data;
	data.SetSource(owner);
	data.SetTarget(origin);
	data.SetFilter(GetHandle());
	data.SetFlags(RayCastFlags::InvertFilter);
	data.SetCollisionFilterGroup(owner->GetCollisionFilter());
	data.SetCollisionFilterMask(owner->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	m_groundRayResult = std::make_shared<TraceResult>(game->Overlap(data));*/

	/* // Obsolete; Now handled by simulation and ControllerPhysObj::SetGroundContactPoint
	auto pTrComponent = owner->GetEntity().GetTransformComponent();
	auto up = pTrComponent.valid() ? pTrComponent->GetUp() : uvec::UP;
	auto origin = GetPosition();
	//auto d = m_controller->GetCharacterController()->Get
	const auto upDist = 20.f; // Ray starts slightly above ground position, in case we're stuck inside an object
	const auto downDist = -5.f;
	TraceData data;
	data.SetSource(origin +up *upDist);
	data.SetTarget(origin +up *downDist);
	data.SetFilter(GetHandle());
	data.SetFlags(RayCastFlags::InvertFilter);
	auto pPhysComponent = owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
	{
		data.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		data.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	}
	static auto bFirst = true;
	if(bFirst == true)
	{
		bFirst = false;
		m_groundRayResult = std::make_shared<TraceResult>(game->RayCast(data));
		m_groundRayResult->hit = false;
	}
	*/
	
	// See also: BaseCharacter::GetAimTraceData

	//m_groundRayResult = std::make_shared<TraceResult>(game->Overlap(data));
	//m_groundRayResult = std::make_shared<TraceResult>(game->RayCast(data));
	/*auto origin = t.GetOrigin();
	origin.y -= 100.f;//0.5f;
	//auto pos = GetPosition();
	//pos.y -= 0.5f;
	t.SetOrigin(origin);

	TraceData data;
	auto up = dynamic_cast<BaseCharacter*>(owner)->GetUpDirection();
	data.SetSource(owner->GetPosition() +up *10.f);
	data.SetTarget(owner->GetPosition() -up *100.f);
	data.SetFilter(GetHandle());
	data.SetFlags(RayCastFlags::InvertFilter);
	data.SetCollisionFilterGroup(owner->GetCollisionFilter());
	data.SetCollisionFilterMask(owner->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	// See also: BaseCharacter::GetAimTraceData

	//m_groundRayResult = std::make_shared<TraceResult>(game->Overlap(data));
	m_groundRayResult = std::make_shared<TraceResult>(game->RayCast(data));*/

	//auto bOnGround = m_bOnGround;
	//if(r.hit == true && r.position.y > pos.y)
	//	std::cout<<"DETECTED OVERLAP: "<<(r.position.y -pos.y)<<std::endl;
	//std::cout<<"OnGround: "<<m_bOnGround<<std::endl;
}
void ControllerPhysObj::SetKinematic(bool)
{
}

void ControllerPhysObj::SetOrientation(const Quat &rot)
{
	//PhysObj::SetOrientation(rot);
}
pragma::BaseEntityComponent *ControllerPhysObj::GetOwner() {return PhysObj::GetOwner();}
void ControllerPhysObj::SetCollisionBounds(const Vector3&,const Vector3&) {}
void ControllerPhysObj::GetCollisionBounds(Vector3 *min,Vector3 *max)
{
	*min = Vector3(0,0,0);
	*max = Vector3(0,0,0);
}

void ControllerPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	m_velocity = vel;
}

bool ControllerPhysObj::IsController() {return true;}
bool ControllerPhysObj::IsCapsule() {return false;}
Vector3 &ControllerPhysObj::GetOffset() {return m_offset;}
void ControllerPhysObj::SetOffset(const Vector3 &offset) {m_offset = offset;}
Vector3 ControllerPhysObj::GetGroundVelocity() const
{
	auto *physColGround = GetGroundPhysCollisionObject();
	if(physColGround == nullptr || physColGround->IsRigid() == false)
		return {};
	auto *rigidBody = physColGround->GetRigidBody();
	auto v = rigidBody->GetLinearVelocity();
	v += util::angular_velocity_to_linear(rigidBody->GetPos(),rigidBody->GetAngularVelocity(),const_cast<ControllerPhysObj*>(this)->GetPosition());
	return v;
}

void ControllerPhysObj::Simulate(double tDelta,bool bIgnoreGravity)
{
	if(m_bDisabled == true || IsKinematic())
		return;
	auto *owner = GetOwner();
	if(owner == NULL || m_collisionObject == nullptr)
		return;
	//NetworkState *state = owner->GetNetworkState();
	//Game *game = state->GetGameState();
	//double tCur = game->CurTime();
	//physx::PxControllerState controllerState;
	//m_controller->getState(controllerState);

	////if(tCur -m_tLastMove >= 1.0f)
	////{
	//static Vector3 posOld = GetPosition();
	//	physx::PxRigidDynamic *actor = m_controller->getActor();
	//	Vector3 velOld = GetLinearVelocity();
	//	physx::PxVec3 disp = physx::PxVec3(1,0,0) *tDelta;// *tDelta;;//physx::PxVec3(10 /60.f,0,0);// *tDelta;
	//	physx::PxU32 collisionFlags = m_controller->move(disp,0.01f,0/*tCur -m_tLastMove*/,physx::PxControllerFilters());
	//	physx::PxVec3 velNew = actor->getLinearVelocity();//(actor->getLinearVelocity() /60.f);// /tDelta;
	//	Con::cwar<<"Disp: ("<<disp.x<<","<<disp.y<<","<<disp.z<<")"<<Con::endl;
	//	Con::cwar<<"Old Velocity: ("<<velOld.x<<","<<velOld.y<<","<<velOld.z<<")"<<Con::endl;
	//	Con::cwar<<"Velocity: ("<<velNew.x<<","<<velNew.y<<","<<velNew.z<<")"<<Con::endl;
	//	Con::cwar<<"Time passed: "<<(tCur -m_tLastMove)<<Con::endl;
	//	Con::cwar<<"-------------"<<Con::endl;
	//	if(tCur -m_tLastMove >= 1.0f)
	//	{
	//		Vector3 posNew = GetPosition();
	//		Con::crit<<"Move distance in 1 second: "<<glm::distance(posOld,posNew)<<Con::endl;
	//		posOld = posNew;
	//		m_tLastMove = tCur;
	//	}
	////}
	//disp = velNew;
	Vector3 disp = m_velocity;
	disp *= tDelta;
	auto t = m_collisionObject->GetWorldTransform();
	//m_originLast = t.GetOrigin();
	m_controller->Move(disp);
	//m_controller->setVelocityForTimeInterval(disp,1);
}

void ControllerPhysObj::SetCurrentFriction(Float friction) {m_currentFriction = friction;}
Float ControllerPhysObj::GetCurrentFriction() const {return m_currentFriction;}

void ControllerPhysObj::UpdateVelocity()
{
	if(m_owner.expired())
		return;
	NetworkState *state = m_owner->GetEntity().GetNetworkState();
	Game *game = state->GetGameState();
	double delta = game->DeltaTickTime();
	float scale;
	if(delta == 0)
		scale = 1;
	else
		scale = 1.f /static_cast<float>(delta);
	auto t = m_collisionObject->GetWorldTransform();
	Vector3 pos = t.GetOrigin();
	//m_velocity = (pos -m_posLast) *scale;
	m_velocity = pos -m_posLast;
	m_posLast = pos;
	if(m_velocity.y < 0.f && IsOnGround())
	{
		//m_velocity.y = 0.f;
		//m_originLast.y = 0.f;
	}
	m_velocity *= scale;
	//m_velocity = (m_velocity -m_originLast) *scale;
	/*m_velocity = ((pos -m_posLast) -m_originLast) *scale;
	if(m_velocity.y < 0.f && IsOnGround())
		m_velocity.y = 0.f;*/
	//else
	//	std::cout<<"!!"<<std::endl;
	m_originLast = pos;
	//m_posLast = Vector3(0.f,0.f,0.f);
}

bool CapsuleControllerPhysObj::IsCapsule() {return true;}

BoxControllerPhysObj::BoxControllerPhysObj(pragma::BaseEntityComponent *owner)
	: ControllerPhysObj(owner)
{}

bool BoxControllerPhysObj::Initialize(const Vector3 &halfExtents,unsigned int stepHeight,float maxSlopeDeg)
{
	if(ControllerPhysObj::Initialize() == false)
		return false;
	m_halfExtents = halfExtents;
	m_stepHeight = static_cast<float>(stepHeight);

	auto pTrComponent = GetOwner()->GetEntity().GetTransformComponent();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	pragma::physics::Transform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(pos);
	m_posLast = pos;
	NetworkState *state = m_networkState;
	Game *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	m_controller = physEnv->CreateBoxController(halfExtents,m_stepHeight,maxSlopeDeg,startTransform);
	auto *collisionObject = m_controller.IsValid() ? m_controller->GetCollisionObject() : nullptr;
	if(collisionObject == nullptr)
		return false;
	m_collisionObject = collisionObject;
	collisionObject->userData = this;
	m_collisionObjects.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::ICollisionObject>(collisionObject->ClaimOwnership()));
	return true;
}

Vector3 &BoxControllerPhysObj::GetHalfExtents() {return m_halfExtents;}
void BoxControllerPhysObj::SetPosition(const Vector3 &pos)
{
	auto t = m_collisionObject->GetWorldTransform();
	Vector3 posCur = t.GetOrigin();
	PhysObj::SetPosition(pos);
	m_posLast += pos -posCur;
}
Vector3 BoxControllerPhysObj::GetPosition() {return PhysObj::GetPosition();}

ControllerPhysObj::ControllerPhysObj(pragma::BaseEntityComponent *owner)
	: PhysObj(owner)
{}
float ControllerPhysObj::GetStepHeight() {return m_stepHeight;}
void ControllerPhysObj::SetStepOffset(float) {}
void ControllerPhysObj::SetPosition(const Vector3 &pos)
{
	auto t = m_collisionObject->GetWorldTransform();
	Vector3 posCur = t.GetOrigin();
	PhysObj::SetPosition(pos);
	m_posLast += pos -posCur;
}
Vector3 ControllerPhysObj::GetDimensions() const
{
	if(m_controller == nullptr)
		return Vector3(0.f,0.f,0.f);
	return m_controller->GetDimensions();
}
void ControllerPhysObj::SetDimensions(const Vector3 &dimensions)
{
	if(m_controller == nullptr)
		return;
	m_controller->SetDimensions(dimensions);
}
Vector3 ControllerPhysObj::GetPosition()
{
	return PhysObj::GetPosition();
}
unsigned int ControllerPhysObj::Move(const Vector3&,float,float)
{
	return 0;
}
void BoxControllerPhysObj::SetCollisionBounds(const Vector3 &min,const Vector3 &max)
{
	m_offset = max +min;
}
void BoxControllerPhysObj::GetCollisionBounds(Vector3 *min,Vector3 *max)
{
	*min = Vector3(0.f,0.f,0.f);
	*max = Vector3(0.f,0.f,0.f);
}
CapsuleControllerPhysObj::CapsuleControllerPhysObj(pragma::BaseEntityComponent *owner)
	: ControllerPhysObj(owner)
{}
bool CapsuleControllerPhysObj::Initialize(unsigned int width,unsigned int height,unsigned int stepHeight,float maxSlopeDeg)
{
	if(ControllerPhysObj::Initialize() == false)
		return false;
	m_width = CFloat(width);
	m_height= CFloat(height);
	m_stepHeight = static_cast<float>(stepHeight);

	auto pTrComponent = GetOwner()->GetEntity().GetTransformComponent();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	pragma::physics::Transform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(pos);
	m_posLast = pos;

	NetworkState *state = m_networkState;
	Game *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	m_controller = physEnv->CreateCapsuleController(width *0.5f,height *0.5f,m_stepHeight,maxSlopeDeg,startTransform);
	auto *collisionObject = m_controller.IsValid() ? m_controller->GetCollisionObject() : nullptr;
	if(collisionObject == nullptr)
		return false;
	m_collisionObject = collisionObject;
	collisionObject->userData = this;
	m_collisionObjects.push_back(util::shared_handle_cast<pragma::physics::IBase,pragma::physics::ICollisionObject>(collisionObject->ClaimOwnership()));
	return true;
}
float CapsuleControllerPhysObj::GetWidth() const {return m_width;}
float CapsuleControllerPhysObj::GetHeight() const {return m_height;}
void CapsuleControllerPhysObj::SetHeight(float height)
{
	auto pos = GetPosition();
	m_height = height;
	if(m_controller == nullptr)
		return;
	auto dimensions = m_controller->GetDimensions();
	dimensions.y = (height -GetWidth()) *0.5f; // Top +Bottom Capsule = Half-Width Each; y-axis of dimension refers to half cylinder height
	m_controller->SetDimensions(dimensions);
	SetPosition(pos);
}
void CapsuleControllerPhysObj::SetPosition(const Vector3 &pos)
{
	Vector3 npos = pos;

	auto rot = ControllerPhysObj::GetOrientation();
	auto offset = Vector3(0,-m_height *0.5f,0);
	uvec::rotate(&offset,rot);
	npos -= offset;

	// Deprecated
	// npos.y += m_height *0.5f;

	ControllerPhysObj::SetPosition(npos);
	//ControllerPhysObj::SetPosition(pos);
}
Vector3 CapsuleControllerPhysObj::GetPosition()
{
	Vector3 pos = ControllerPhysObj::GetPosition();

	auto rot = ControllerPhysObj::GetOrientation();
	auto offset = Vector3(0,-m_height *0.5f,0);
	uvec::rotate(&offset,rot);
	pos += offset;

	// Deprecated
	// pos.y -= m_height *0.5f;

	return pos;
	//return ControllerPhysObj::GetPosition();
}
void CapsuleControllerPhysObj::SetCollisionBounds(const Vector3 &min,const Vector3 &max)
{
	m_offset = max +min;
}
void CapsuleControllerPhysObj::GetCollisionBounds(Vector3 *min,Vector3 *max)
{
	*min = Vector3(0.f,0.f,0.f);
	*max = Vector3(0.f,0.f,0.f);
}
pragma::physics::IController *ControllerPhysObj::GetController() {return m_controller.Get();}
pragma::physics::ICollisionObject *ControllerPhysObj::GetCollisionObject() {return m_collisionObject.Get();}
