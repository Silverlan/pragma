/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/operators/c_particle_mod_physics.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/physics/environment.hpp>
#include <pragma/physics/shape.hpp>
#include <pragma/physics/collisionmesh.h>
#include <pragma/physics/collision_object.hpp>
#include <pragma/model/model.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT pragma::physics::IEnvironment *c_physEnv;

void CParticleOperatorPhysics::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
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
		else if(key == "rotation_offset") {
			auto ang = EulerAngles(it->second);
			m_rotOffset = uquat::create(ang);
		}
		else if(key == "translation_offset")
			m_posOffset = uvec::create(it->second);
	}
}
void CParticleOperatorPhysics::OnParticleSystemStarted()
{
	auto maxParticles = GetParticleSystem().GetMaxParticleCount();
	if(maxParticles == 0)
		return;
	auto shape = CreateShape();
	if(shape == nullptr)
		return;
	auto mass = m_mass;
	if(mass <= 0.f)
		mass = 1.f; // Mas has to be larger than 0
	shape->SetMass(mass);

	m_physicsObjects.resize(maxParticles);
	for(auto i = decltype(maxParticles) {0}; i < maxParticles; ++i) {
		auto rigidBody = c_physEnv->CreateRigidBody(*shape);
		if(rigidBody != nullptr) {
			m_physicsObjects[i] = rigidBody;
			rigidBody->SetCollisionFilterGroup(CollisionMask::Particle);
			rigidBody->SetCollisionFilterMask(CollisionMask::All & ~CollisionMask::Particle);
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
void CParticleOperatorPhysics::OnParticleSystemStopped() { m_physicsObjects.clear(); }
void CParticleOperatorPhysics::OnParticleCreated(CParticle &particle)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	auto pos = m_posOffset;
	auto rot = m_rotOffset;
	uvec::local_to_world(particle.GetPosition(), particle.GetWorldRotation(), pos, rot);
	auto *rigidBody = hPhys.Get();
	rigidBody->EnableSimulation();
	rigidBody->SetPos(pos);
	rigidBody->SetRotation(rot);
	rigidBody->SetLinearVelocity(particle.GetVelocity());
	rigidBody->SetAngularVelocity(particle.GetAngularVelocity());
	rigidBody->WakeUp();
}
void CParticleOperatorPhysics::OnParticleDestroyed(CParticle &particle)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	hPhys->DisableSimulation();
}

void CParticleOperatorPhysics::PreSimulate(CParticle &particle, double)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	auto pos = hPhys->GetPos();
	auto rot = hPhys->GetRotation();

	auto translationOffset = m_posOffset;
	rot = rot * uquat::get_inverse(m_rotOffset);
	uvec::rotate(&translationOffset, rot);
	pos -= translationOffset;

	particle.SetPosition(pos);
	particle.SetWorldRotation(rot);
	particle.SetVelocity(hPhys->GetLinearVelocity());
	particle.SetAngularVelocity(hPhys->GetAngularVelocity());
}

void CParticleOperatorPhysics::PostSimulate(CParticle &particle, double)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto pos = m_posOffset;
	auto rot = m_rotOffset;
	uvec::local_to_world(particle.GetPosition(), particle.GetWorldRotation(), pos, rot);
	auto &hPhys = m_physicsObjects[idx];
	hPhys->SetPos(pos);
	hPhys->SetRotation(rot);
	hPhys->SetLinearVelocity(particle.GetVelocity());
	hPhys->SetAngularVelocity(particle.GetAngularVelocity());
}

//////////////////////////////

void CParticleOperatorPhysicsSphere::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> CParticleOperatorPhysicsSphere::CreateShape() { return c_physEnv->CreateSphereShape(m_radius, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void CParticleOperatorPhysicsBox::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "extent")
			m_extent = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> CParticleOperatorPhysicsBox::CreateShape() { return c_physEnv->CreateBoxShape({m_extent, m_extent, m_extent}, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void CParticleOperatorPhysicsCylinder::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
		else if(key == "height")
			m_height = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> CParticleOperatorPhysicsCylinder::CreateShape() { return c_physEnv->CreateCylinderShape(m_radius, m_height, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void CParticleOperatorPhysicsModel::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "model") {
			m_model = c_game->LoadModel(pair.second);
			break;
		}
	}
}

void CParticleOperatorPhysicsModel::OnParticleCreated(CParticle &particle)
{
	if(m_model == nullptr)
		return;
	CParticleOperatorPhysics::OnParticleCreated(particle);
	particle.SetOrigin(m_model->GetOrigin());
}

std::shared_ptr<pragma::physics::IShape> CParticleOperatorPhysicsModel::CreateShape()
{
	if(m_model == nullptr)
		return nullptr;
	auto &colMeshes = m_model->GetCollisionMeshes();
	std::vector<pragma::physics::IShape *> shapes;
	shapes.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes) {
		auto shape = colMesh->GetShape();
		if(shape == nullptr)
			continue;
		if(colMeshes.size() == 1)
			return shape;
		shapes.push_back(shape.get());
	}
	return c_physEnv->CreateCompoundShape(shapes);
}
