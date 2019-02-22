#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_mod_physics.h"
#include <pragma/physics/physenvironment.h>
#include <pragma/physics/physshape.h>
#include <pragma/physics/collisionmesh.h>
#include <pragma/model/model.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT PhysEnv *c_physEnv;

CParticleOperatorPhysics::CParticleOperatorPhysics(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperator(pSystem,values)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "mass")
			m_mass = util::to_float(it->second);
		else if(key == "linear_damping")
			m_linearDamping = util::to_float(it->second);
		else if(key == "linear_factor")
			m_linearFactor = uvec::create(it->second);
		else if(key == "angular_damping")
			m_angularDamping = util::to_float(it->second);
		else if(key == "angular_factor")
			m_angularFactor = uvec::create(it->second);
		else if(key == "rotation_offset")
		{
			auto ang = EulerAngles(it->second);
			m_rotOffset = uquat::create(ang);
		}
		else if(key == "translation_offset")
			m_posOffset = uvec::create(it->second);
	}
}
void CParticleOperatorPhysics::Initialize()
{
	auto maxParticles = m_particleSystem.GetMaxParticleCount();
	if(maxParticles == 0)
		return;
	auto shape = CreateShape();
	if(shape == nullptr)
		return;
	auto mass = m_mass;
	if(mass <= 0.f)
		mass = 1.f; // Mas has to be larger than 0

	Vector3 localInertia {};
	shape->CalculateLocalInertia(mass,&localInertia);
	m_physicsObjects.resize(maxParticles);
	for(auto i=decltype(maxParticles){0};i<maxParticles;++i)
	{
		auto *rigidBody = c_physEnv->CreateRigidBody(mass,shape,localInertia);
		if(rigidBody != nullptr)
		{
			m_physicsObjects[i] = std::shared_ptr<PhysRigidBody>(rigidBody);
			rigidBody->SetCollisionFilterGroup(CollisionMask::Particle);
			rigidBody->SetCollisionFilterMask(CollisionMask::All &~CollisionMask::Particle);
			rigidBody->DisableSimulation();
			rigidBody->SetCCDEnabled(true);
			rigidBody->SetLinearDamping(m_linearDamping);
			rigidBody->SetLinearFactor(m_linearFactor);
			rigidBody->SetAngularDamping(m_angularDamping);
			rigidBody->SetAngularFactor(m_angularFactor);
			rigidBody->Spawn();
		}
	}
}
void CParticleOperatorPhysics::Destroy()
{
	m_physicsObjects.clear();
}
void CParticleOperatorPhysics::Initialize(CParticle &particle)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	auto pos = m_posOffset;
	auto rot = m_rotOffset;
	uvec::local_to_world(particle.GetPosition(),particle.GetWorldRotation(),pos,rot);
	auto *rigidBody = hPhys.get();
	rigidBody->EnableSimulation();
	rigidBody->SetPos(pos);
	rigidBody->SetRotation(rot);
	rigidBody->SetLinearVelocity(particle.GetVelocity());
	rigidBody->SetAngularVelocity(particle.GetAngularVelocity());
	rigidBody->Activate();
}
void CParticleOperatorPhysics::Destroy(CParticle &particle)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	hPhys->DisableSimulation();
}

void CParticleOperatorPhysics::PreSimulate(CParticle &particle,double)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	auto pos = hPhys->GetPos();
	auto rot = hPhys->GetRotation();

	auto translationOffset = m_posOffset;
	rot = rot *uquat::get_inverse(m_rotOffset);
	uvec::rotate(&translationOffset,rot);
	pos -= translationOffset;

	particle.SetPosition(pos);
	particle.SetWorldRotation(rot);
	particle.SetVelocity(hPhys->GetLinearVelocity());
	particle.SetAngularVelocity(hPhys->GetAngularVelocity());
}

void CParticleOperatorPhysics::PostSimulate(CParticle &particle,double)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto pos = m_posOffset;
	auto rot = m_rotOffset;
	uvec::local_to_world(particle.GetPosition(),particle.GetWorldRotation(),pos,rot);
	auto &hPhys = m_physicsObjects[idx];
	hPhys->SetPos(pos);
	hPhys->SetRotation(rot);
	hPhys->SetLinearVelocity(particle.GetVelocity());
	hPhys->SetAngularVelocity(particle.GetAngularVelocity());
}

//////////////////////////////

CParticleOperatorPhysicsSphere::CParticleOperatorPhysicsSphere(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorPhysics(pSystem,values)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
	}
}

std::shared_ptr<PhysShape> CParticleOperatorPhysicsSphere::CreateShape()
{
	return c_physEnv->CreateSphereShape(m_radius);
}

//////////////////////////////

CParticleOperatorPhysicsBox::CParticleOperatorPhysicsBox(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorPhysics(pSystem,values)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "extent")
			m_extent = util::to_float(it->second);
	}
}

std::shared_ptr<PhysShape> CParticleOperatorPhysicsBox::CreateShape()
{
	return c_physEnv->CreateBoxShape({m_extent,m_extent,m_extent});
}

//////////////////////////////

CParticleOperatorPhysicsCylinder::CParticleOperatorPhysicsCylinder(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorPhysics(pSystem,values)
{
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
		else if(key == "height")
			m_height = util::to_float(it->second);
	}
}

std::shared_ptr<PhysShape> CParticleOperatorPhysicsCylinder::CreateShape()
{
	return c_physEnv->CreateCylinderShape(m_radius,m_height);
}

//////////////////////////////

CParticleOperatorPhysicsModel::CParticleOperatorPhysicsModel(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperatorPhysics(pSystem,values)
{
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "model")
		{
			m_model = c_game->LoadModel(pair.second);
			break;
		}
	}
}

void CParticleOperatorPhysicsModel::Initialize(CParticle &particle)
{
	if(m_model == nullptr)
		return;
	CParticleOperatorPhysics::Initialize(particle);
	particle.SetOrigin(m_model->GetOrigin());
}

std::shared_ptr<PhysShape> CParticleOperatorPhysicsModel::CreateShape()
{
	if(m_model == nullptr)
		return nullptr;
	auto &colMeshes = m_model->GetCollisionMeshes();
	std::vector<std::shared_ptr<PhysShape>> shapes;
	shapes.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes)
	{
		auto shape = colMesh->GetShape();
		if(shape == nullptr)
			continue;
		if(colMeshes.size() == 1)
			return shape;
		shapes.push_back(shape);
	}
	return c_physEnv->CreateCompoundShape(shapes);
}
