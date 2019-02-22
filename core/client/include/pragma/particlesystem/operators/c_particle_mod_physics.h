#ifndef __C_PARTICLE_MOD_PHYSICS_H__
#define __C_PARTICLE_MOD_PHYSICS_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleOperatorPhysics
	: public CParticleOperator
{
protected:
	float m_mass = 0.f;
	float m_linearDamping = 0.f;
	Vector3 m_linearFactor = {1.f,1.f,1.f};
	float m_angularDamping = 0.f;
	Vector3 m_angularFactor = {1.f,1.f,1.f};
	Vector3 m_posOffset = {};
	Quat m_rotOffset = uquat::identity();
	std::vector<std::shared_ptr<PhysRigidBody>> m_physicsObjects;
	virtual std::shared_ptr<PhysShape> CreateShape()=0;
public:
	CParticleOperatorPhysics(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Initialize(CParticle &particle) override;
	virtual void Initialize() override;
	virtual void Destroy(CParticle &particle) override;
	virtual void Destroy() override;
	virtual void PreSimulate(CParticle &particle,double) override;
	virtual void PostSimulate(CParticle &particle,double) override;
};

class DLLCLIENT CParticleOperatorPhysicsSphere
	: public CParticleOperatorPhysics
{
protected:
	float m_radius = 0.f;
	virtual std::shared_ptr<PhysShape> CreateShape() override;
public:
	CParticleOperatorPhysicsSphere(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
};
REGISTER_PARTICLE_OPERATOR(physics_sphere,CParticleOperatorPhysicsSphere);

class DLLCLIENT CParticleOperatorPhysicsBox
	: public CParticleOperatorPhysics
{
protected:
	float m_extent = 0.f;
	virtual std::shared_ptr<PhysShape> CreateShape() override;
public:
	CParticleOperatorPhysicsBox(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
};
REGISTER_PARTICLE_OPERATOR(physics_box,CParticleOperatorPhysicsBox);

class DLLCLIENT CParticleOperatorPhysicsCylinder
	: public CParticleOperatorPhysics
{
protected:
	float m_radius = 0.f;
	float m_height = 0.f;
	virtual std::shared_ptr<PhysShape> CreateShape() override;
public:
	CParticleOperatorPhysicsCylinder(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
};
REGISTER_PARTICLE_OPERATOR(physics_cylinder,CParticleOperatorPhysicsCylinder);

class DLLCLIENT CParticleOperatorPhysicsModel
	: public CParticleOperatorPhysics
{
public:
	CParticleOperatorPhysicsModel(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Initialize(CParticle &particle) override;
protected:
	std::shared_ptr<Model> m_model = nullptr;
	virtual std::shared_ptr<PhysShape> CreateShape() override;
};
REGISTER_PARTICLE_OPERATOR(physics_model,CParticleOperatorPhysicsModel);

#endif
