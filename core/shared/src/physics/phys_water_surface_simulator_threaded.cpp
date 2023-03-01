/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/phys_water_surface_simulator.hpp"

void PhysWaterSurfaceSimulator::JoinThread()
{
	m_bRunThread = false;
	m_simThread.join();
}
std::size_t PhysWaterSurfaceSimulator::GetParticleIndex(uint32_t x, uint32_t y) const { return GetParticleIndex(m_surfaceInfo, x, y); }
std::pair<uint32_t, uint32_t> PhysWaterSurfaceSimulator::GetParticleCoordinates(std::size_t idx) const { return GetParticleCoordinates(m_surfaceInfo, idx); }
std::size_t PhysWaterSurfaceSimulator::GetParticleIndex(const SurfaceInfo &surfInfo, uint32_t x, uint32_t y) const { return y * surfInfo.width + x; }
std::pair<uint32_t, uint32_t> PhysWaterSurfaceSimulator::GetParticleCoordinates(const SurfaceInfo &surfInfo, std::size_t idx) const { return std::pair<uint32_t, uint32_t>(idx / surfInfo.width, idx % surfInfo.width); }
void PhysWaterSurfaceSimulator::SolveDepths(const SurfaceInfo &surfInfo)
{
	std::size_t ptIdx = 0;
	for(auto &pt : GetParticleField()) {
		auto x = pt.GetTargetHeight() - pt.GetHeight();
		SetParticleHeight(surfInfo, ptIdx++, pt.GetHeight() + x * surfInfo.stiffness);
	}
}
void PhysWaterSurfaceSimulator::SolveEdges(const SurfaceInfo &surfInfo)
{
	auto &particleField = GetParticleField();
	for(auto &edge : GetParticleEdges()) {
		auto &pt0 = particleField.at(edge.index0);
		auto &pt1 = particleField.at(edge.index1);

		auto d = pt1.GetHeight() - pt0.GetHeight();
		d *= surfInfo.propagation;

		SetParticleHeight(surfInfo, edge.index0, pt0.GetHeight() + d);
		SetParticleHeight(surfInfo, edge.index1, pt1.GetHeight() - d);
	}
}
void PhysWaterSurfaceSimulator::SetParticleHeight(const SurfaceInfo &surfInfo, std::size_t ptIdx, float height)
{
	auto &pt = m_particleField.at(ptIdx);
	pt.SetHeight(umath::min(height, surfInfo.maxHeight));
	m_threadParticleHeights.at(ptIdx) = height;
}
void PhysWaterSurfaceSimulator::Integrate(const SurfaceInfo &surfInfo, double dt)
{
	std::size_t ptIdx = 0;
	for(auto &pt : GetParticleField())
		SetParticleHeight(surfInfo, ptIdx++, pt.GetHeight() + dt * pt.GetVelocity());
}
void PhysWaterSurfaceSimulator::VelocityFixup(const SurfaceInfo &surfInfo, double invDt)
{
	auto &particleField = GetParticleField();
	for(auto i = decltype(particleField.size()) {0}; i < particleField.size(); ++i) {
		auto &pt = particleField.at(i);
		auto delta = pt.GetHeight() - pt.GetOldHeight();
		pt.SetVelocity(invDt * delta);
		pt.SetOldHeight(pt.GetHeight());

		//UpdateParticlePosition(surfInfo,i);
	}
}
