// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.water_surface_simulator;

void pragma::physics::PhysWaterSurfaceSimulator::JoinThread()
{
	m_bRunThread = false;
	m_simThread.join();
}
std::size_t pragma::physics::PhysWaterSurfaceSimulator::GetParticleIndex(uint32_t x, uint32_t y) const { return GetParticleIndex(m_surfaceInfo, x, y); }
std::pair<uint32_t, uint32_t> pragma::physics::PhysWaterSurfaceSimulator::GetParticleCoordinates(std::size_t idx) const { return GetParticleCoordinates(m_surfaceInfo, idx); }
std::size_t pragma::physics::PhysWaterSurfaceSimulator::GetParticleIndex(const SurfaceInfo &surfInfo, uint32_t x, uint32_t y) const { return y * surfInfo.width + x; }
std::pair<uint32_t, uint32_t> pragma::physics::PhysWaterSurfaceSimulator::GetParticleCoordinates(const SurfaceInfo &surfInfo, std::size_t idx) const { return std::pair<uint32_t, uint32_t>(idx / surfInfo.width, idx % surfInfo.width); }
void pragma::physics::PhysWaterSurfaceSimulator::SolveDepths(const SurfaceInfo &surfInfo)
{
	std::size_t ptIdx = 0;
	for(auto &pt : GetParticleField()) {
		auto x = pt.GetTargetHeight() - pt.GetHeight();
		SetParticleHeight(surfInfo, ptIdx++, pt.GetHeight() + x * surfInfo.stiffness);
	}
}
void pragma::physics::PhysWaterSurfaceSimulator::SolveEdges(const SurfaceInfo &surfInfo)
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
void pragma::physics::PhysWaterSurfaceSimulator::SetParticleHeight(const SurfaceInfo &surfInfo, std::size_t ptIdx, float height)
{
	auto &pt = m_particleField.at(ptIdx);
	pt.SetHeight(math::min(height, surfInfo.maxHeight));
	m_threadParticleHeights.at(ptIdx) = height;
}
void pragma::physics::PhysWaterSurfaceSimulator::Integrate(const SurfaceInfo &surfInfo, double dt)
{
	std::size_t ptIdx = 0;
	for(auto &pt : GetParticleField())
		SetParticleHeight(surfInfo, ptIdx++, pt.GetHeight() + dt * pt.GetVelocity());
}
void pragma::physics::PhysWaterSurfaceSimulator::VelocityFixup(const SurfaceInfo &surfInfo, double invDt)
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
