/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/phys_water_surface_simulator.hpp"
#include <pragma/console/s_cvar.h>
#include <pragma/math/intersection.h>

extern DLLNETWORK Engine *engine;

// See http://www.randygaul.net/wp-content/uploads/2014/02/RigidBodies_WaterSurface.pdf for surface simulation algorithms

PhysWaterSurfaceSimulator::SplashInfo::SplashInfo(const Vector3 &_origin, float _radius, float _force, uint32_t _width, uint32_t _length) : origin(_origin.x, _origin.y, _origin.z), radius(_radius), radiusSqr(umath::pow2(_radius)), force(_force), width(_width), length(_length) {}

float PhysWaterSurfaceSimulator::Particle::GetHeight() const { return m_height; }
void PhysWaterSurfaceSimulator::Particle::SetHeight(float height) { m_height = height; }
float PhysWaterSurfaceSimulator::Particle::GetOldHeight() const { return m_oldHeight; }
void PhysWaterSurfaceSimulator::Particle::SetOldHeight(float oldHeight) { m_oldHeight = oldHeight; }
float PhysWaterSurfaceSimulator::Particle::GetTargetHeight() const { return m_targetHeight; }
void PhysWaterSurfaceSimulator::Particle::SetTargetHeight(float height) { m_targetHeight = height; }
float PhysWaterSurfaceSimulator::Particle::GetVelocity() const { return m_velocity; }
void PhysWaterSurfaceSimulator::Particle::SetVelocity(float vel) { m_velocity = vel; }
void PhysWaterSurfaceSimulator::Particle::SetNeighbor(std::size_t idx, uint32_t ptIdx)
{
	if(idx >= m_neighbors.size())
		return;
	m_neighbors.at(idx) = ptIdx;
}
PhysWaterSurfaceSimulator::Edge::Edge(uint32_t idx0, uint32_t idx1) : index0(idx0), index1(idx1) {}

PhysWaterSurfaceSimulator::PhysWaterSurfaceSimulator(Vector2 aabbMin, Vector2 aabbMax, float originY, uint32_t spacing, float stiffness, float propagation)
{
	m_surfaceInfo.stiffness = stiffness;
	m_surfaceInfo.propagation = propagation;
	m_surfaceInfo.spacing = spacing;
	m_originY = originY;

	uvec::to_min_max(aabbMin, aabbMax);
	m_bounds = {aabbMin, aabbMax};

	for(auto &v : m_bounds) {
		for(uint8_t i = 0; i < 2; ++i) {
			auto r = fmodf(umath::abs(v[i]), spacing);
			if(r > 0.f)
				v[i] -= (spacing - r);
		}
	}
	m_surfaceInfo.origin = {aabbMin.x, m_originY, aabbMin.y};
	m_surfaceInfo.length = (aabbMax.x - aabbMin.x) / spacing;
	m_surfaceInfo.width = (aabbMax.y - aabbMin.y) / spacing;
}
PhysWaterSurfaceSimulator::~PhysWaterSurfaceSimulator() { JoinThread(); }
const Vector3 &PhysWaterSurfaceSimulator::GetOrigin() const { return m_surfaceInfo.origin; }
void PhysWaterSurfaceSimulator::InitializeSurface()
{
	auto width = GetWidth();
	auto length = GetLength();
	auto numParticles = static_cast<uint64_t>(width) * static_cast<uint64_t>(length);
	if(numParticles > std::numeric_limits<uint32_t>::max())
		return;
	m_particleField.resize(numParticles);
	m_particleHeights.resize(numParticles);
	m_threadParticleHeights.resize(numParticles);

	m_particleEdges.reserve(4 * 2 +              // Corner particles
	  ((width - 2) * 2 + (length - 2) * 2) * 3 + // Edge particles
	  (width - 2) * (length - 2) * 4             // Inner particles
	);
	for(auto i = decltype(width) {0}; i < width; ++i) {
		for(auto j = decltype(length) {0}; j < length; ++j) {
			std::size_t edgeIdx = 0;
			auto ptIdx = GetParticleIndex(m_surfaceInfo, i, j);
			auto &pt = m_particleField.at(ptIdx);
			if(j > 0) {
				auto ptIdxOther = GetParticleIndex(m_surfaceInfo, i, j - 1);
				m_particleEdges.push_back(Edge(ptIdx, ptIdxOther));
				pt.SetNeighbor(edgeIdx++, m_particleEdges.back().index1);
			}
			if(j < (length - 1)) {
				auto ptIdxOther = GetParticleIndex(m_surfaceInfo, i, j + 1);
				m_particleEdges.push_back(Edge(ptIdx, ptIdxOther));
				pt.SetNeighbor(edgeIdx++, m_particleEdges.back().index1);
			}
			if(i > 0) {
				auto ptIdxOther = GetParticleIndex(m_surfaceInfo, i - 1, j);
				m_particleEdges.push_back(Edge(ptIdx, ptIdxOther));
				pt.SetNeighbor(edgeIdx++, m_particleEdges.back().index1);
			}
			if(i < (width - 1)) {
				auto ptIdxOther = GetParticleIndex(m_surfaceInfo, i + 1, j);
				m_particleEdges.push_back(Edge(ptIdx, ptIdxOther));
				pt.SetNeighbor(edgeIdx++, m_particleEdges.back().index1);
			}
		}
	}
}
void PhysWaterSurfaceSimulator::Initialize()
{
	InitializeSurface();
	if(m_particleField.empty() == true)
		return;
	if(m_bUseThread == false)
		return;
	m_simThread = std::thread([this]() {
		while(m_bRunThread == true)
			SimulateWaves(0.01); // TODO: Delta?
	});
}
uint32_t PhysWaterSurfaceSimulator::GetSpacing() const { return m_surfaceInfo.spacing; }
uint32_t PhysWaterSurfaceSimulator::GetWidth() const { return m_surfaceInfo.width; }
uint32_t PhysWaterSurfaceSimulator::GetLength() const { return m_surfaceInfo.length; }
std::vector<PhysWaterSurfaceSimulator::Particle> &PhysWaterSurfaceSimulator::GetParticleField() { return m_particleField; }
std::vector<PhysWaterSurfaceSimulator::Edge> &PhysWaterSurfaceSimulator::GetParticleEdges() { return m_particleEdges; }
const std::vector<PhysWaterSurfaceSimulator::Particle> &PhysWaterSurfaceSimulator::GetParticleField() const { return const_cast<PhysWaterSurfaceSimulator *>(this)->GetParticleField(); }
const std::vector<PhysWaterSurfaceSimulator::Edge> &PhysWaterSurfaceSimulator::GetParticleEdges() const { return const_cast<PhysWaterSurfaceSimulator *>(this)->GetParticleEdges(); }
std::size_t PhysWaterSurfaceSimulator::GetParticleCount() const { return m_particleField.size(); }
float PhysWaterSurfaceSimulator::GetStiffness() const { return m_surfaceInfo.stiffness; }
void PhysWaterSurfaceSimulator::SetStiffness(float stiffness) { m_surfaceInfo.stiffness = stiffness; }
float PhysWaterSurfaceSimulator::GetMaxWaveHeight() const { return m_surfaceInfo.maxHeight; }
void PhysWaterSurfaceSimulator::SetMaxWaveHeight(float height) { m_surfaceInfo.maxHeight = height; }
void PhysWaterSurfaceSimulator::SetPropagation(float propagation) { m_surfaceInfo.propagation = propagation; }
float PhysWaterSurfaceSimulator::GetPropagation() const { return m_surfaceInfo.propagation; }
void PhysWaterSurfaceSimulator::CreateSplash(const Vector3 &origin, float radius, float force)
{
	if(radius <= 0.f)
		return;
	m_splashMutex.lock();
	m_splashQueue.push({origin, radius, force, GetWidth(), GetLength()});
	m_splashMutex.unlock();
}
//const Vector3 &PhysWaterSurfaceSimulator::GetPosition() const {return m_position;}
//void PhysWaterSurfaceSimulator::SetPosition(const Vector3 &pos) {m_position = pos;}
//const Quat &PhysWaterSurfaceSimulator::GetRotation() const {return m_rotation;}
//void PhysWaterSurfaceSimulator::SetRotation(const Quat &rot) {m_rotation = rot;} // TODO
void PhysWaterSurfaceSimulator::Simulate(double dt)
{
	if(m_bUseThread == true)
		return;
	SimulateWaves(dt);
}
void PhysWaterSurfaceSimulator::SimulateWaves(double dt)
{
	//if(dt <= 0.0) // TODO
	//	return;

	m_settingsMutex.lock();
	auto surfInfo = m_surfaceInfo; // Copy settings to avoid race conditions
	m_settingsMutex.unlock();

	// Apply splashes
	auto width = GetWidth();
	auto length = GetLength();
	m_splashMutex.lock();
	while(m_splashQueue.empty() == false) {
		auto &info = m_splashQueue.front();
		auto r2 = info.radiusSqr;
		auto &particleField = GetParticleField();
		for(auto i = decltype(particleField.size()) {0}; i < particleField.size(); ++i) {
			auto pos = CalcParticlePosition(surfInfo, m_threadParticleHeights, i);
			auto l = uvec::length_sqr(pos - info.origin);
			if(l < r2) {
				l = umath::sqrt(l);
				auto factor = (info.radius - l) / info.radius;
				auto &pt = particleField.at(i);
				pt.SetOldHeight(pt.GetHeight());
				SetParticleHeight(surfInfo, i, pt.GetHeight() + info.force * factor);
			}
		}
		m_splashQueue.pop();
	}
	m_splashMutex.unlock();

	dt = 0.01;
	Integrate(surfInfo, dt);
	auto sovleEdgeCount = GetEdgeIterationCount();
	for(auto i = decltype(sovleEdgeCount) {0}; i < sovleEdgeCount; ++i)
		SolveEdges(surfInfo);
	SolveDepths(surfInfo);
	VelocityFixup(surfInfo, 1.0 / dt);

	m_heightMutex.lock();
	std::copy(m_threadParticleHeights.begin(), m_threadParticleHeights.end(), m_particleHeights.begin());
	m_heightMutex.unlock();
}
uint8_t PhysWaterSurfaceSimulator::GetEdgeIterationCount() const { return engine->GetServerNetworkState()->GetConVarInt("sv_water_surface_simulation_edge_iteration_count"); }
Vector3 PhysWaterSurfaceSimulator::CalcParticlePosition(const SurfaceInfo &surfInfo, const std::vector<float> &heights, std::size_t ptIdx) const
{
	auto c = GetParticleCoordinates(surfInfo, ptIdx);
	return Vector3 {surfInfo.origin.x + c.first * surfInfo.spacing, surfInfo.origin.y + heights.at(ptIdx), surfInfo.origin.z + c.second * surfInfo.spacing};
}
Vector3 PhysWaterSurfaceSimulator::CalcParticlePosition(std::size_t ptIdx) const { return CalcParticlePosition(m_surfaceInfo, m_particleHeights, ptIdx); }
void PhysWaterSurfaceSimulator::LockParticleHeights() { m_heightMutex.lock(); }
void PhysWaterSurfaceSimulator::UnlockParticleHeights() { m_heightMutex.unlock(); }
bool PhysWaterSurfaceSimulator::CalcPointSurfaceIntersection(const Vector3 &origin, Vector3 &intersection) const
{
	auto posFirst = CalcParticlePosition(0);
	auto posLast = CalcParticlePosition(m_particleField.size() - 1);
	posFirst.y = 0.f; // TODO: Relative to plane!
	posLast.y = 0.f;
	auto bounds = posLast - posFirst;
	auto offset = origin - posFirst;

	auto width = GetWidth();
	auto length = GetLength();

	// Calculate local grid coordinates
	auto x = umath::floor((offset.z / bounds.z) * static_cast<float>(width - 1));
	auto y = umath::floor((offset.x / bounds.x) * static_cast<float>(length - 1));
	if(x < 0 || x >= width || y < 0 || y >= length)
		return false;
	// Retrieve triangle indices for grid position (Always two triangles)
	auto ptIdx0 = GetParticleIndex(m_surfaceInfo, x, y);
	auto ptIdx1 = GetParticleIndex(m_surfaceInfo, x + 1, y);
	auto ptIdx2 = GetParticleIndex(m_surfaceInfo, x, y + 1);
	auto ptIdx3 = GetParticleIndex(m_surfaceInfo, x + 1, y + 1);
	auto numParticles = m_particleField.size();
	assert(ptIdx0 < numParticles && ptIdx1 < numParticles && ptIdx2 < numParticles && ptIdx3 < numParticles);
	if(ptIdx0 >= numParticles || ptIdx1 >= numParticles || ptIdx2 >= numParticles || ptIdx3 >= numParticles)
		return false;
	std::array<Vector3, 4> particlePositions = {CalcParticlePosition(ptIdx0), CalcParticlePosition(ptIdx1), CalcParticlePosition(ptIdx2), CalcParticlePosition(ptIdx3)};
	const auto n = Vector3(0, 1, 0); // TODO
	double t, u, v;
	if(umath::intersection::line_triangle(origin, n, particlePositions.at(0), particlePositions.at(1), particlePositions.at(2), t, u, v) == true || umath::intersection::line_triangle(origin, n, particlePositions.at(3), particlePositions.at(2), particlePositions.at(1), t, u, v) == true) {
		intersection = origin + n * static_cast<float>(t);
		return true;
	}
	return false;
}
