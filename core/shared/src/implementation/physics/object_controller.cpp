// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.object;

pragma::physics::ControllerPhysObj::~ControllerPhysObj()
{
	//auto *state = m_networkState;
	//Game *game = state->GetGameState();
	//PhysEnv *physEnv = game->GetPhysicsEnvironment();
	if(m_controller.IsValid())
		m_controller.Remove();
	m_hitData.Clear();
}

int32_t pragma::physics::ControllerPhysObj::GetGroundSurfaceMaterial() const
{
	auto *physMat = m_controller->GetGroundMaterial();
	auto *surfMat = physMat ? physMat->GetSurfaceMaterial() : nullptr;
	if(surfMat == nullptr)
		return -1;
	return surfMat->GetIndex();
}
pragma::physics::IMaterial *pragma::physics::ControllerPhysObj::GetGroundMaterial() const { return m_controller->GetGroundMaterial(); }
bool pragma::physics::ControllerPhysObj::IsOnGround() const { return m_controller->IsTouchingGround(); }
bool pragma::physics::ControllerPhysObj::IsGroundWalkable() const
{
	if(IsOnGround() == false)
		return false;
	auto n = m_controller->GetGroundTouchNormal();
	if(!n.has_value())
		return false;
	auto angle = math::acos(uvec::dot(*n, m_controller->GetUpDirection()));
	auto slopeLimit = GetSlopeLimit();
	auto bGroundWalkable = (angle <= math::deg_to_rad(slopeLimit));
	return bGroundWalkable;
}
pragma::ecs::BaseEntity *pragma::physics::ControllerPhysObj::GetGroundEntity() const
{
	auto *o = GetGroundPhysObject();
	auto *pOwner = (o != nullptr) ? o->GetOwner() : nullptr;
	return (pOwner != nullptr) ? &pOwner->GetEntity() : nullptr;
}
pragma::physics::PhysObj *pragma::physics::ControllerPhysObj::GetGroundPhysObject() const
{
	auto *o = GetGroundPhysCollisionObject();
	return o ? o->GetPhysObj() : nullptr;
}
pragma::physics::ICollisionObject *pragma::physics::ControllerPhysObj::GetGroundPhysCollisionObject() { return m_controller->GetGroundBody(); }
const pragma::physics::ICollisionObject *pragma::physics::ControllerPhysObj::GetGroundPhysCollisionObject() const { return const_cast<ControllerPhysObj *>(this)->GetGroundPhysCollisionObject(); }

pragma::physics::ControllerHitData &pragma::physics::ControllerPhysObj::GetControllerHitData() { return m_hitData; }
void pragma::physics::ControllerPhysObj::PostSimulate()
{
	PhysObjDynamic::PostSimulate();
	auto *owner = GetOwner();
	if(owner == nullptr)
		return;
	auto *state = owner->GetEntity().GetNetworkState();
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
	data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	data.SetCollisionFilterGroup(owner->GetCollisionFilter());
	data.SetCollisionFilterMask(owner->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	m_groundRayResult = pragma::util::make_shared<TraceResult>(game->Overlap(data));*/

	/* // Obsolete; Now handled by simulation and pragma::physics::ControllerPhysObj::SetGroundContactPoint
	auto pTrComponent = owner->GetEntity().GetTransformComponent();
	auto up = pTrComponent != nullptr ? pTrComponent->GetUp() : uvec::PRM_UP;
	auto origin = GetPosition();
	//auto d = m_controller->GetCharacterController()->Get
	const auto upDist = 20.f; // Ray starts slightly above ground position, in case we're stuck inside an object
	const auto downDist = -5.f;
	TraceData data;
	data.SetSource(origin +up *upDist);
	data.SetTarget(origin +up *downDist);
	data.SetFilter(GetHandle());
	data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	auto pPhysComponent = owner->GetEntity().GetPhysicsComponent();
	if(pPhysComponent != nullptr)
	{
		data.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		data.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	}
	static auto bFirst = true;
	if(bFirst == true)
	{
		bFirst = false;
		m_groundRayResult = pragma::util::make_shared<TraceResult>(game->RayCast(data));
		m_groundRayResult->hit = false;
	}
	*/

	// See also: BaseCharacter::GetAimTraceData

	//m_groundRayResult = pragma::util::make_shared<TraceResult>(game->Overlap(data));
	//m_groundRayResult = pragma::util::make_shared<TraceResult>(game->RayCast(data));
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
	data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	data.SetCollisionFilterGroup(owner->GetCollisionFilter());
	data.SetCollisionFilterMask(owner->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	// See also: BaseCharacter::GetAimTraceData

	//m_groundRayResult = pragma::util::make_shared<TraceResult>(game->Overlap(data));
	m_groundRayResult = pragma::util::make_shared<TraceResult>(game->RayCast(data));*/

	//auto bOnGround = m_bOnGround;
	//if(r.hit == true && r.position.y > pos.y)
	//	std::cout<<"DETECTED OVERLAP: "<<(r.position.y -pos.y)<<std::endl;
	//std::cout<<"OnGround: "<<m_bOnGround<<std::endl;
}
void pragma::physics::ControllerPhysObj::SetKinematic(bool) {}

void pragma::physics::ControllerPhysObj::SetOrientation(const Quat &rot)
{
	//PhysObj::SetOrientation(rot);
}
pragma::BaseEntityComponent *pragma::physics::ControllerPhysObj::GetOwner() { return PhysObj::GetOwner(); }
pragma::math::Degree pragma::physics::ControllerPhysObj::GetSlopeLimit() const
{
	if(m_controller == nullptr)
		return 0.f;
	return m_controller->GetSlopeLimit();
}
void pragma::physics::ControllerPhysObj::SetSlopeLimit(math::Degree limit)
{
	if(m_controller == nullptr)
		return;
	m_controller->SetSlopeLimit(limit);
}
void pragma::physics::ControllerPhysObj::SetCollisionBounds(const Vector3 &, const Vector3 &) {}
void pragma::physics::ControllerPhysObj::GetCollisionBounds(Vector3 *min, Vector3 *max)
{
	*min = Vector3(0, 0, 0);
	*max = Vector3(0, 0, 0);
}

void pragma::physics::ControllerPhysObj::SetLinearVelocity(const Vector3 &vel)
{
	m_velocity = vel;
	m_controller->SetLinearVelocity(vel);
}

bool pragma::physics::ControllerPhysObj::IsController() const { return true; }
bool pragma::physics::ControllerPhysObj::IsCapsule() const { return false; }
Vector3 &pragma::physics::ControllerPhysObj::GetOffset() { return m_offset; }
void pragma::physics::ControllerPhysObj::SetOffset(const Vector3 &offset) { m_offset = offset; }
Vector3 pragma::physics::ControllerPhysObj::GetGroundVelocity() const
{
	auto *physColGround = GetGroundPhysCollisionObject();
	if(physColGround == nullptr || physColGround->IsRigid() == false)
		return {};
	auto *rigidBody = physColGround->GetRigidBody();
	auto v = rigidBody->GetLinearVelocity();
	v += math::angular_velocity_to_linear(rigidBody->GetPos(), rigidBody->GetAngularVelocity(), const_cast<ControllerPhysObj *>(this)->GetPosition());
	return v;
}

void pragma::physics::ControllerPhysObj::Simulate(double tDelta, bool bIgnoreGravity)
{
	if(IsDisabled() || IsKinematic())
		return;
	auto *owner = GetOwner();
	if(owner == nullptr || m_collisionObject == nullptr)
		return;
	//auto *state = owner->GetNetworkState();
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
	//	Con::CWAR<<"Disp: ("<<disp.x<<","<<disp.y<<","<<disp.z<<")"<<Con::endl;
	//	Con::CWAR<<"Old Velocity: ("<<velOld.x<<","<<velOld.y<<","<<velOld.z<<")"<<Con::endl;
	//	Con::CWAR<<"Velocity: ("<<velNew.x<<","<<velNew.y<<","<<velNew.z<<")"<<Con::endl;
	//	Con::CWAR<<"Time passed: "<<(tCur -m_tLastMove)<<Con::endl;
	//	Con::CWAR<<"-------------"<<Con::endl;
	//	if(tCur -m_tLastMove >= 1.0f)
	//	{
	//		Vector3 posNew = GetPosition();
	//		Con::CRIT<<"Move distance in 1 second: "<<glm::distance(posOld,posNew)<<Con::endl;
	//		posOld = posNew;
	//		m_tLastMove = tCur;
	//	}
	////}
	//disp = velNew;
	Vector3 disp = m_velocity;
	disp *= tDelta;
	auto t = m_collisionObject->GetWorldTransform();
	//m_originLast = t.GetOrigin();
	m_controller->SetMoveVelocity(disp);
	//m_controller->setVelocityForTimeInterval(disp,1);
}

void pragma::physics::ControllerPhysObj::UpdateVelocity()
{
	if(m_owner.expired())
		return;
	auto *state = m_owner->GetEntity().GetNetworkState();
	Game *game = state->GetGameState();
	double delta = game->DeltaTickTime();
	float scale;
	if(delta == 0)
		scale = 1;
	else
		scale = 1.f / static_cast<float>(delta);
	Vector3 pos = GetPosition();
	m_velocity = m_controller->GetLinearVelocity();
	m_originLast = pos;
}

bool pragma::physics::CapsuleControllerPhysObj::IsCapsule() const { return true; }

pragma::physics::BoxControllerPhysObj::BoxControllerPhysObj(BaseEntityComponent *owner) : ControllerPhysObj(owner) {}

bool pragma::physics::BoxControllerPhysObj::Initialize(const Vector3 &halfExtents, unsigned int stepHeight, float maxSlopeDeg)
{
	if(ControllerPhysObj::Initialize() == false)
		return false;
	m_halfExtents = halfExtents;
	m_stepHeight = static_cast<float>(stepHeight);

	auto pTrComponent = GetOwner()->GetEntity().GetTransformComponent();
	auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	math::Transform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(pos);
	auto *state = m_networkState;
	Game *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	m_controller = physEnv->CreateBoxController(halfExtents, m_stepHeight, maxSlopeDeg, startTransform);
	auto *collisionObject = m_controller.IsValid() ? m_controller->GetCollisionObject() : nullptr;
	if(collisionObject == nullptr)
		return false;
	m_collisionObject = pragma::util::shared_handle_cast<IBase, ICollisionObject>(collisionObject->ClaimOwnership());
	collisionObject->SetPhysObj(*this);
	m_collisionObjects.push_back(m_collisionObject);

	collisionObject->SetAlwaysAwake(true);
	return true;
}

Vector3 &pragma::physics::BoxControllerPhysObj::GetHalfExtents() { return m_halfExtents; }

pragma::physics::ControllerPhysObj::ControllerPhysObj(BaseEntityComponent *owner) : PhysObj(owner) {}
float pragma::physics::ControllerPhysObj::GetStepHeight() const { return m_stepHeight; }
void pragma::physics::ControllerPhysObj::SetStepOffset(float) {}
Vector3 pragma::physics::ControllerPhysObj::GetDimensions() const
{
	if(m_controller == nullptr)
		return Vector3(0.f, 0.f, 0.f);
	return m_controller->GetDimensions();
}
void pragma::physics::ControllerPhysObj::SetDimensions(const Vector3 &dimensions)
{
	if(m_controller == nullptr)
		return;
	m_controller->SetDimensions(dimensions);
}
void pragma::physics::ControllerPhysObj::SetPosition(const Vector3 &pos)
{
	if(m_controller == nullptr) {
		PhysObj::SetPosition(pos);
		return;
	}
	auto posCur = m_controller->GetFootPos();
	m_controller->SetFootPos(pos);
}
Vector3 pragma::physics::ControllerPhysObj::GetPosition() const
{
	if(m_controller == nullptr)
		return PhysObj::GetPosition();
	return m_controller->GetFootPos();
}
unsigned int pragma::physics::ControllerPhysObj::Move(const Vector3 &, float, float) { return 0; }
void pragma::physics::BoxControllerPhysObj::SetCollisionBounds(const Vector3 &min, const Vector3 &max) { m_offset = max + min; }
void pragma::physics::BoxControllerPhysObj::GetCollisionBounds(Vector3 *min, Vector3 *max)
{
	*min = Vector3(0.f, 0.f, 0.f);
	*max = Vector3(0.f, 0.f, 0.f);
}
pragma::physics::CapsuleControllerPhysObj::CapsuleControllerPhysObj(BaseEntityComponent *owner) : ControllerPhysObj(owner) {}
bool pragma::physics::CapsuleControllerPhysObj::Initialize(unsigned int width, unsigned int height, unsigned int stepHeight, float maxSlopeDeg)
{
	if(ControllerPhysObj::Initialize() == false)
		return false;
	m_width = CFloat(width);
	m_height = CFloat(height);
	m_stepHeight = static_cast<float>(stepHeight);

	auto pTrComponent = GetOwner()->GetEntity().GetTransformComponent();
	auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	math::Transform startTransform;
	startTransform.SetIdentity();
	startTransform.SetOrigin(pos);

	auto *state = m_networkState;
	Game *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	m_controller = physEnv->CreateCapsuleController(width * 0.5f, height * 0.5f, m_stepHeight, maxSlopeDeg, startTransform);
	auto *collisionObject = m_controller.IsValid() ? m_controller->GetCollisionObject() : nullptr;
	if(collisionObject == nullptr)
		return false;
	m_collisionObject = pragma::util::shared_handle_cast<IBase, ICollisionObject>(collisionObject->ClaimOwnership());
	collisionObject->SetPhysObj(*this);
	m_collisionObjects.push_back(m_collisionObject);

	collisionObject->SetAlwaysAwake(true);
	return true;
}
float pragma::physics::CapsuleControllerPhysObj::GetWidth() const { return m_width; }
float pragma::physics::CapsuleControllerPhysObj::GetHeight() const { return m_height; }
void pragma::physics::CapsuleControllerPhysObj::SetHeight(float height)
{
	auto pos = GetPosition();
	m_height = height;
	if(m_controller == nullptr)
		return;
	m_controller->Resize(height);
	/*auto dimensions = m_controller->GetDimensions();
	dimensions.y = (height -GetWidth()) *0.5f; // Top +Bottom Capsule = Half-Width Each; y-axis of dimension refers to half cylinder height
	m_controller->SetDimensions(dimensions);
	SetPosition(pos);*/
}
void pragma::physics::CapsuleControllerPhysObj::SetCollisionBounds(const Vector3 &min, const Vector3 &max) { m_offset = max + min; }
void pragma::physics::CapsuleControllerPhysObj::GetCollisionBounds(Vector3 *min, Vector3 *max)
{
	*min = Vector3(0.f, 0.f, 0.f);
	*max = Vector3(0.f, 0.f, 0.f);
}
pragma::physics::IController *pragma::physics::ControllerPhysObj::GetController() { return m_controller.Get(); }
pragma::physics::ICollisionObject *pragma::physics::ControllerPhysObj::GetCollisionObject() { return m_collisionObject.Get(); }
