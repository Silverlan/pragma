#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/physics/physicstypes.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_networked_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/model/model.h"
#include "pragma/entities/baseentity_events.hpp"

DEFINE_BASE_HANDLE(DLLNETWORK,BaseEntity,Entity);

#ifdef PHYS_ENGINE_BULLET
bool BaseEntity::IsStatic() const
{
	if(GetAnimatedComponent().expired())
		return false;
	auto physComponent = GetPhysicsComponent();
	auto type = physComponent.valid() ? physComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	return (type == PHYSICSTYPE::NONE || type == PHYSICSTYPE::STATIC) ? true : false;
}
bool BaseEntity::IsDynamic() const {return !IsStatic();}

#elif PHYS_ENGINE_PHYSX
void BaseEntity::OnContact(BaseEntity *ent,PhysObj *phys,physx::PxActor *actor)
{

}
void BaseEntity::OnContact(BaseEntity *ent,PhysObj *phys,physx::PxActor *actor,const physx::PxContactPair &cp,unsigned int nbContacts,physx::PxContactPairPoint *contacts)
{
	if(contact != NULL)
		delete contact;
	contact = new TestContact;
	contact->entity = ent;
	contact->physObj = phys;
	contact->actor = actor;
	for(unsigned int i=0;i<nbContacts;i++)
	{
		physx::PxContactPairPoint &point = contacts[i];
		contact->contactPoints.push_back(Vector3(point.position.x,point.position.y,point.position.z));
	}
}
void BaseEntity::SetCollisionNotificationEnabled(unsigned int flags,bool bEnabled)
{
	PhysObj *phys = GetPhysicsObject();
	if(phys != NULL)
	{
		std::vector<physx::PxRigidActor*> *actors = phys->GetActors();
		for(unsigned int i=0;i<actors->size();i++)
		{
			physx::PxRigidActor *actor = (*actors)[i];
			unsigned int numShapes = actor->getNbShapes();
			physx::PxShape **shapes = new physx::PxShape*[numShapes];
			actor->getShapes(&shapes[0],numShapes);
			for(unsigned int j=0;j<numShapes;j++)
			{
				physx::PxShape *shape = shapes[j];
				physx::PxFilterData filter = shape->getSimulationFilterData();
				if(bEnabled == true)
					filter.word2 |= unsigned int(flags);
				else
					filter.word2 &= ~unsigned int(flags);
				shape->setSimulationFilterData(filter);
			}
			delete[] shapes;
		}
	}
}
void BaseEntity::SetCollisionCallbacksEnabled(bool b)
{
	m_bColCallbacksEnabled = b;
	SetCollisionNotificationEnabled(physx::PxPairFlag::eNOTIFY_TOUCH_FOUND,b);
}
void BaseEntity::SetCollisionContactReportEnabled(bool b)
{
	m_bColContactReportEnabled = b;
	SetCollisionNotificationEnabled(physx::PxPairFlag::eNOTIFY_CONTACT_POINTS,b);
}
bool BaseEntity::GetControllerState(physx::PxControllerState &state)
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || !phys->IsController())
		return false;
	ControllerPhysObj *physController = static_cast<ControllerPhysObj*>(phys);
	physx::PxController *controller = physController->GetController();
	controller->getState(state);
	return true;
}
void BaseEntity::UpdatePhysicsData()
{
	PHYSICSTYPE type = GetPhysicsType();
	if(type == PHYSICSTYPE::NONE)
		return;
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL)
	{
		m_velocity->x = 0;
		m_velocity->y = 0;
		m_velocity->z = 0;
		return;
	}
	//if(phys->IsSleeping())
	//	return;
	physx::PxRigidActor *actor = phys->GetActor();
	if(actor == NULL)
		return;
	if(phys->IsController())
		static_cast<ControllerPhysObj*>(phys)->UpdateVelocity();
	*m_velocity = phys->GetLinearVelocity();

	bool bStatic = phys->IsStatic();
	bool bSnapshot = !bStatic;
	Vector3 pos = phys->GetPosition();
	physx::PxTransform t = actor->getGlobalPose();
	if(!m_physObject->IsController()) // TODO
	{
		if(bStatic == false && (m_orientation->w != t.q.w || m_orientation->x != t.q.x || m_orientation->y != t.q.y || m_orientation->z != t.q.z))
			bSnapshot = true;
		m_orientation->w = t.q.w;
		m_orientation->x = t.q.x;
		m_orientation->y = t.q.y;
		m_orientation->z = t.q.z;

		if(!bStatic)
		{
			Vector3 angVelocity = phys->GetAngularVelocity();
			m_angVelocity->x = angVelocity.x;
			m_angVelocity->y = angVelocity.y;
			m_angVelocity->z = angVelocity.z;
		}
	}
	if(bSnapshot == false && (m_pos->x != pos.x || m_pos->y != pos.y || m_pos->z != pos.z))
		bSnapshot = true;
	m_pos->x = pos.x;
	m_pos->y = pos.y;
	m_pos->z = pos.z;
	if(type == PHYSICSTYPE::DYNAMIC)
	{
		DynamicPhysObj *phys = static_cast<DynamicPhysObj*>(m_physObject);
		std::vector<DynamicActorInfo> &actorInfo = phys->GetActorInfo();
		Model *mdl = GetModel();
		Frame *frame;
		Skeleton *skeleton;
		if(mdl != NULL)
		{
			Animation *ref = mdl->GetAnimation(0);
			frame = ref->GetFrame(0);
			skeleton = mdl->GetSkeleton();
		}
#pragma message("TODO: Do this properly!")
		for(unsigned int i=1;i<actorInfo.size();i++)
		{
			DynamicActorInfo &info = actorInfo[i];
			int boneID = info.GetBoneID();
			physx::PxRigidDynamic *actor = info.GetActor();
			physx::PxTransform t = actor->getGlobalPose();
			Vector3 posActor = Vector3(t.p.x,t.p.y,t.p.z);
			Quat rotActor = Quat(t.q.w,t.q.x,t.q.y,t.q.z);

			if(mdl != NULL)
			{
				Vector3 posBind = *frame->GetBonePosition(boneID);
				Quat rotBind = *frame->GetBoneOrientation(boneID);

				posActor -= rotActor *Quat(Angle(-Vector3::getNormal(posBind))).Forward() *glm::length(posBind);
				posActor = (posActor -(*m_pos)) *(*m_orientation);

				rotActor = rotActor *rotBind;
				rotActor = glm::inverse(*m_orientation) *rotActor;

				SetBonePosition(boneID,posActor,rotActor);
			}
		}
	}
	if(bSnapshot)
		MarkForSnapshot(true);
}
void BaseEntity::RayCast(const Vector3 &dir,float distance)
{
	Vector3 origin = *GetPosition();
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();
	physx::PxScene *scene = game->GetPhysXScene();
	TraceResult res;
	bool b = game->RayCast(origin,dir,distance,this,&res);
	if(b == false)
		return;
	SetPosition(res.position);
}

void BaseEntity::Sweep(const Vector3 &dir,float distance)
{
	Vector3 origin = *GetPosition();
	NetworkState *state = GetNetworkState();
	Game *game = state->GetGameState();
	TraceResult res;
	PhysObj *phys = GetPhysicsObject();
	physx::PxRigidActor *actor = (phys != NULL) ? phys->GetActor() : NULL;
	physx::PxShape *shape = NULL;
	if(actor != NULL)
		actor->getShapes(&shape,1);
	PHYSICSTYPE type = (shape != NULL) ? GetPhysicsType() : PHYSICSTYPE::NONE;
	bool b;
	switch(type)
	{
	case PHYSICSTYPE::DYNAMIC:
	case PHYSICSTYPE::STATIC:
		{
			physx::PxConvexMeshGeometry geometry;
			shape->getConvexMeshGeometry(geometry);
			b = game->Sweep(geometry,origin,dir,distance,this,&res);
			break;
		}
	case PHYSICSTYPE::BOXCONTROLLER:
		{
			physx::PxBoxGeometry geometry;
			shape->getBoxGeometry(geometry);
			b = game->Sweep(geometry,origin,dir,distance,this,&res);
			break;
		}
	case PHYSICSTYPE::CAPSULECONTROLLER:
		{
			physx::PxCapsuleGeometry geometry;
			shape->getCapsuleGeometry(geometry);
			b = game->Sweep(geometry,origin,dir,distance,this,&res);
			break;
		}
	default:
		{
			b = game->RayCast(origin,dir,distance,this,&res);
		}
	};
	if(b == false)
		return;
	SetPosition(res.position);
	SetVelocity(Vector3(0,0,0));
}
BaseEntity *BaseEntity::GetGroundEntity()
{
	physx::PxControllerState state;
	if(!GetControllerState(state))
		return NULL;
	physx::PxRigidActor *actor = state.touchedActor;
	if(actor == NULL)
		return NULL;
	void *data = actor->userData;
	if(data == NULL)
		return NULL;
	PhysObj *phys = static_cast<PhysObj*>(data);
	return phys->GetOwner();
}
physx::PxRigidActor *BaseEntity::GetGroundActor()
{
	physx::PxControllerState state;
	if(!GetControllerState(state))
		return NULL;
	return state.touchedActor;
}
physx::PxShape *BaseEntity::GetGroundState()
{
	physx::PxControllerState state;
	if(!GetControllerState(state))
		return NULL;
	return state.touchedShape;
}
bool BaseEntity::GetCollisionCallbacksEnabled() {return m_bColCallbacksEnabled;}

bool BaseEntity::GetCollisionContactReportEnabled() {return m_bColContactReportEnabled;}
#endif

extern DLLENGINE Engine *engine;
Con::c_cout& BaseEntity::print(Con::c_cout &os)
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent*>(FindComponent("name").get());
	os<<"Entity["<<m_index<<"]["<<GetClass()<<"]["<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os<<"NULL";
	else
		os<<hMdl->GetName();
	os<<"]";
	return os;
}

std::ostream& BaseEntity::print(std::ostream &os)
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent*>(FindComponent("name").get());
	os<<"Entity["<<m_index<<"]["<<GetClass()<<"]["<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os<<"NULL";
	else
		os<<hMdl->GetName();
	os<<"]";
	return os;
}

pragma::ComponentEventId BaseEntity::EVENT_HANDLE_KEY_VALUE = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_POST_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_REMOVE = pragma::INVALID_COMPONENT_ID;
BaseEntity::BaseEntity()
	: pragma::BaseEntityComponentSystem(),LuaObj<EntityHandle>()
{}
pragma::NetEventId BaseEntity::FindNetEvent(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->FindNetEvent(name);
}

BaseEntity::StateFlags BaseEntity::GetStateFlags() const {return m_stateFlags;}
void BaseEntity::ResetStateChangeFlags() {m_stateFlags &= ~(StateFlags::CollisionBoundsChanged | StateFlags::PositionChanged | StateFlags::RenderBoundsChanged | StateFlags::RotationChanged);}
bool BaseEntity::HasStateFlag(StateFlags flag) const {return ((m_stateFlags &flag) == flag) ? true : false;}
void BaseEntity::SetStateFlag(StateFlags flag) {m_stateFlags |= flag;}
pragma::ComponentEventId BaseEntity::RegisterComponentEvent(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->GetEntityComponentManager().RegisterEvent(name);
}
pragma::ComponentEventId BaseEntity::GetEventId(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEventId(name);
}
void BaseEntity::OnRemove()
{
	for(auto it=m_entsRemove.begin();it!=m_entsRemove.end();++it)
	{
		auto &hEnt = *it;
		if(hEnt.IsValid())
			hEnt->Remove();
	}
	BroadcastEvent(EVENT_ON_REMOVE);
	ClearComponents();
}

void BaseEntity::Construct(unsigned int idx)
{
	m_index = idx;
	Initialize();
}

bool BaseEntity::IsMapEntity() const
{
	auto *mapComponent = static_cast<pragma::MapComponent*>(FindComponent("map").get());
	return mapComponent != nullptr && mapComponent->GetMapIndex() != 0;
}
uint64_t BaseEntity::GetUniqueIndex() const {return m_uniqueIndex;}
void BaseEntity::SetUniqueIndex(uint64_t idx) {m_uniqueIndex = idx;}

void BaseEntity::RemoveEntityOnRemoval(BaseEntity *ent,Bool bRemove) {RemoveEntityOnRemoval(ent->GetHandle(),bRemove);}
void BaseEntity::RemoveEntityOnRemoval(const EntityHandle &hEnt,Bool bRemove)
{
	if(!hEnt.IsValid())
		return;
	auto *ent = hEnt.get();
	auto it = std::find_if(m_entsRemove.begin(),m_entsRemove.end(),[ent](EntityHandle &hOther) {
		return (hOther.IsValid() && hOther.get() == ent) ? true : false;
	});
	if(bRemove == true)
	{
		if(it == m_entsRemove.end())
			m_entsRemove.push_back(hEnt);
		return;
	}
	else if(it == m_entsRemove.end())
		return;
	m_entsRemove.erase(it);
}
void BaseEntity::SetKeyValue(std::string key,std::string val)
{
	ustring::to_lower(key);
	pragma::CEKeyValueData inputData{key,val};
	if(BroadcastEvent(EVENT_HANDLE_KEY_VALUE,inputData) == util::EventReply::Handled)
		return;
	if(key == "spawnflags")
		m_spawnFlags = util::to_int(val);
}
void BaseEntity::SetSpawnFlags(uint32_t spawnFlags) {m_spawnFlags = spawnFlags;}
unsigned int BaseEntity::GetSpawnFlags() const {return m_spawnFlags;}

void BaseEntity::MarkForSnapshot(bool b)
{
	if(b)
		m_stateFlags |= StateFlags::SnapshotUpdateRequired;
	else
		m_stateFlags &= ~StateFlags::SnapshotUpdateRequired;
}
bool BaseEntity::IsMarkedForSnapshot() const {return (m_stateFlags &StateFlags::SnapshotUpdateRequired) != StateFlags::None;}

void BaseEntity::EraseFunction(int) {}

lua_State *BaseEntity::GetLuaState() const
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	return game->GetLuaState();
}

pragma::NetEventId BaseEntity::SetupNetEvent(const std::string &name) const {return GetNetworkState()->GetGameState()->SetupNetEvent(name);}

void BaseEntity::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_HANDLE_KEY_VALUE = componentManager.RegisterEvent("HANDLE_KEY_VALUE");
	EVENT_ON_SPAWN = componentManager.RegisterEvent("ON_SPAWN");
	EVENT_ON_POST_SPAWN = componentManager.RegisterEvent("ON_POST_SPAWN");
	EVENT_ON_REMOVE = componentManager.RegisterEvent("ON_REMOVE");
}

void BaseEntity::Initialize()
{
	InitializeHandle();
	BaseEntityComponentSystem::Initialize(*this,GetNetworkState()->GetGameState()->GetEntityComponentManager());
	AddComponent("entity");
}

std::string BaseEntity::GetClass() const {return m_class;}

const Vector3 &BaseEntity::GetPosition() const
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return uvec::ORIGIN;
	return trComponent->GetPosition();
}
void BaseEntity::SetPosition(const Vector3 &pos)
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return;
	trComponent->SetPosition(pos);
}
Vector3 BaseEntity::GetCenter() const
{
	auto physComponent = GetPhysicsComponent();
	if(physComponent.expired())
		return GetPosition();
	return physComponent->GetCenter();
}
const Quat &BaseEntity::GetRotation() const
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return uquat::UNIT;
	return trComponent->GetOrientation();
}
void BaseEntity::SetRotation(const Quat &rot)
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return;
	trComponent->SetOrientation(rot);
}

void BaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	pragma::BaseEntityComponentSystem::OnComponentAdded(component);
	auto *ptrTransformComponent = dynamic_cast<pragma::BaseTransformComponent*>(&component);
	if(ptrTransformComponent != nullptr)
		m_transformComponent = std::static_pointer_cast<pragma::BaseTransformComponent>(ptrTransformComponent->shared_from_this());
}
void BaseEntity::OnComponentRemoved(pragma::BaseEntityComponent &component)
{
	pragma::BaseEntityComponentSystem::OnComponentRemoved(component);
}

void BaseEntity::DoSpawn()
{
	// Flag has to be set before events are triggered, in case
	// one of the events relies (directly or indirectly) on :IsSpawned
	m_stateFlags |= StateFlags::Spawned;
	BroadcastEvent(EVENT_ON_SPAWN);
}

void BaseEntity::Spawn()
{
	if(IsSpawned())
		return;
	DoSpawn();
	OnPostSpawn();
}

void BaseEntity::OnSpawn() {}

void BaseEntity::OnPostSpawn() {BroadcastEvent(EVENT_ON_POST_SPAWN);}

bool BaseEntity::IsSpawned() const {return (m_stateFlags &StateFlags::Spawned) != StateFlags::None;}

bool BaseEntity::IsInert() const
{
	if(GetAnimatedComponent().valid())
		return false;
	if(IsStatic() == true)
		return true;
	auto pPhysComponent = GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	return (phys != nullptr && phys->IsSleeping()) ? true : false;
}

unsigned int BaseEntity::GetIndex() const {return m_index;}

bool BaseEntity::IsWorld() const {return false;}
bool BaseEntity::IsScripted() const {return false;}

void BaseEntity::PrecacheModels() {}

util::WeakHandle<pragma::BaseTransformComponent> BaseEntity::GetTransformComponent() const {return m_transformComponent.expired() ? nullptr : m_transformComponent.lock();}

void BaseEntity::Remove() {}
void BaseEntity::RemoveSafely() {GetNetworkState()->GetGameState()->ScheduleEntityForRemoval(*this);}

void BaseEntity::InitializeHandle() {BaseEntity::InitializeHandle<EntityHandle>();}

////////////////////////////////////

DLLNETWORK bool operator==(const EntityHandle &a,const EntityHandle &b) {return a.get() == b.get();}

DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,const EntityHandle &ent)
{
	if(!ent.IsValid())
		os<<"NULL";
	else
		os<<*ent.get();
	return os;
}

static std::ostream& operator<<(std::ostream &os,BaseEntity &ent) {return ent.print(os);}

DLLNETWORK std::ostream& operator<<(std::ostream &os,const EntityHandle ent)
{
	if(!ent.IsValid())
		os<<"NULL";
	else
		os<<*ent.get();
	return os;
}
