// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.operator_physics;

import :game;

extern DLLCLIENT pragma::physics::IEnvironment *c_physEnv;

void pragma::pts::CParticleOperatorPhysics::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
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
void pragma::pts::CParticleOperatorPhysics::OnParticleSystemStarted()
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
			rigidBody->SetCollisionFilterGroup(physics::CollisionMask::Particle);
			rigidBody->SetCollisionFilterMask(physics::CollisionMask::All & ~physics::CollisionMask::Particle);
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
void pragma::pts::CParticleOperatorPhysics::OnParticleSystemStopped() { m_physicsObjects.clear(); }
void pragma::pts::CParticleOperatorPhysics::OnParticleCreated(CParticle &particle)
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
void pragma::pts::CParticleOperatorPhysics::OnParticleDestroyed(CParticle &particle)
{
	auto idx = particle.GetIndex();
	if(idx >= m_physicsObjects.size())
		return;
	auto &hPhys = m_physicsObjects[idx];
	hPhys->DisableSimulation();
}

void pragma::pts::CParticleOperatorPhysics::PreSimulate(CParticle &particle, double)
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

void pragma::pts::CParticleOperatorPhysics::PostSimulate(CParticle &particle, double)
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

void pragma::pts::CParticleOperatorPhysicsSphere::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> pragma::pts::CParticleOperatorPhysicsSphere::CreateShape() { return c_physEnv->CreateSphereShape(m_radius, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void pragma::pts::CParticleOperatorPhysicsBox::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "extent")
			m_extent = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> pragma::pts::CParticleOperatorPhysicsBox::CreateShape() { return c_physEnv->CreateBoxShape({m_extent, m_extent, m_extent}, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void pragma::pts::CParticleOperatorPhysicsCylinder::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "radius")
			m_radius = util::to_float(it->second);
		else if(key == "height")
			m_height = util::to_float(it->second);
	}
}

std::shared_ptr<pragma::physics::IShape> pragma::pts::CParticleOperatorPhysicsCylinder::CreateShape() { return c_physEnv->CreateCylinderShape(m_radius, m_height, c_physEnv->GetGenericMaterial()); }

//////////////////////////////

void pragma::pts::CParticleOperatorPhysicsModel::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPhysics::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "model") {
			m_model = get_cgame()->LoadModel(pair.second);
			break;
		}
	}
}

void pragma::pts::CParticleOperatorPhysicsModel::OnParticleCreated(CParticle &particle)
{
	if(m_model == nullptr)
		return;
	CParticleOperatorPhysics::OnParticleCreated(particle);
	particle.SetOrigin(m_model->GetOrigin());
}

std::shared_ptr<pragma::physics::IShape> pragma::pts::CParticleOperatorPhysicsModel::CreateShape()
{
	if(m_model == nullptr)
		return nullptr;
	auto &colMeshes = m_model->GetCollisionMeshes();
	std::vector<physics::IShape *> shapes;
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
