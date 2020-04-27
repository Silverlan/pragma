/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
	std::vector<util::TSharedHandle<pragma::physics::IRigidBody>> m_physicsObjects;
	virtual std::shared_ptr<pragma::physics::IShape> CreateShape()=0;
public:
	CParticleOperatorPhysics()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	virtual void OnParticleSystemStarted() override;
	virtual void OnParticleDestroyed(CParticle &particle) override;
	virtual void OnParticleSystemStopped() override;
	virtual void PreSimulate(CParticle &particle,double) override;
	virtual void PostSimulate(CParticle &particle,double) override;
};

class DLLCLIENT CParticleOperatorPhysicsSphere
	: public CParticleOperatorPhysics
{
protected:
	float m_radius = 0.f;
	virtual std::shared_ptr<pragma::physics::IShape> CreateShape() override;
public:
	CParticleOperatorPhysicsSphere()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
};
REGISTER_PARTICLE_OPERATOR(physics_sphere,CParticleOperatorPhysicsSphere);

class DLLCLIENT CParticleOperatorPhysicsBox
	: public CParticleOperatorPhysics
{
protected:
	float m_extent = 0.f;
	virtual std::shared_ptr<pragma::physics::IShape> CreateShape() override;
public:
	CParticleOperatorPhysicsBox()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
};
REGISTER_PARTICLE_OPERATOR(physics_box,CParticleOperatorPhysicsBox);

class DLLCLIENT CParticleOperatorPhysicsCylinder
	: public CParticleOperatorPhysics
{
protected:
	float m_radius = 0.f;
	float m_height = 0.f;
	virtual std::shared_ptr<pragma::physics::IShape> CreateShape() override;
public:
	CParticleOperatorPhysicsCylinder()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
};
REGISTER_PARTICLE_OPERATOR(physics_cylinder,CParticleOperatorPhysicsCylinder);

class DLLCLIENT CParticleOperatorPhysicsModel
	: public CParticleOperatorPhysics
{
public:
	CParticleOperatorPhysicsModel()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
protected:
	std::shared_ptr<Model> m_model = nullptr;
	virtual std::shared_ptr<pragma::physics::IShape> CreateShape() override;
};
REGISTER_PARTICLE_OPERATOR(physics_model,CParticleOperatorPhysicsModel);

#endif
