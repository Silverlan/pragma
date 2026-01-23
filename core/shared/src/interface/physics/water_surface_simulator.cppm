// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.water_surface_simulator;

export import pragma.math;

export namespace pragma::physics {
	class DLLNETWORK PhysWaterSurfaceSimulator : public std::enable_shared_from_this<PhysWaterSurfaceSimulator> {
	public:
#pragma pack(push, 1)
		class DLLNETWORK Particle {
		private:
			std::array<uint32_t, 4> m_neighbors = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
			float m_height = 0.f;
			float m_oldHeight = 0.f;
			float m_targetHeight = 0.f;
			float m_velocity = 0.f;
		public:
			float GetHeight() const;
			void SetHeight(float height);
			float GetOldHeight() const;
			void SetOldHeight(float oldHeight);
			float GetTargetHeight() const;
			void SetTargetHeight(float height);
			float GetVelocity() const;
			void SetVelocity(float vel);
			void SetNeighbor(std::size_t idx, uint32_t ptIdx);
		};
#pragma pack(pop)
#pragma pack(push, 1)
		struct DLLNETWORK Edge {
			Edge(uint32_t idx0, uint32_t idx1);
			uint32_t index0 = 0;
			uint32_t index1 = 0;
		};
#pragma pack(pop)
#pragma pack(push, 1)
		struct DLLNETWORK SplashInfo {
			SplashInfo(const Vector3 &origin, float radius, float force, uint32_t width, uint32_t length);
			Vector3 origin;
			float radius = 0.f;
			float radiusSqr = 0.f;
			float force = 0.f;
			uint32_t width = 0;
			uint32_t length = 0;
		};
#pragma pack(pop)

		PhysWaterSurfaceSimulator(Vector2 aabbMin, Vector2 aabbMax, float originY, uint32_t spacing, float stiffness = 0.1f, float propagation = 100.f);
		virtual ~PhysWaterSurfaceSimulator();
		uint32_t GetWidth() const;
		uint32_t GetLength() const;
		const std::vector<Particle> &GetParticleField() const;
		const std::vector<Edge> &GetParticleEdges() const;
		std::size_t GetParticleCount() const;
		float GetStiffness() const;
		void SetStiffness(float stiffness);
		float GetMaxWaveHeight() const;
		void SetMaxWaveHeight(float height);
		float GetPropagation() const;
		void SetPropagation(float propagation);
		uint32_t GetSpacing() const;
		const Vector3 &GetOrigin() const;
		std::size_t GetParticleIndex(uint32_t x, uint32_t y) const;
		std::pair<uint32_t, uint32_t> GetParticleCoordinates(std::size_t idx) const;
		virtual void Simulate(double dt);

		void Initialize();
		void CreateSplash(const Vector3 &origin, float radius, float force);

		void LockParticleHeights();
		void UnlockParticleHeights();

		// Not safe to call, unless particle heights were locked!
		Vector3 CalcParticlePosition(std::size_t ptIdx) const;
		bool CalcPointSurfaceIntersection(const Vector3 &origin, Vector3 &intersection) const;
	protected:
#pragma pack(push, 1)
		struct DLLNETWORK SurfaceInfo {
			Vector3 origin = {};
			float stiffness = 0.f;
			float propagation = 0.f;
			float maxHeight = 100.f;
			uint32_t spacing = 0;
			uint32_t width = 0;
			uint32_t length = 0;
		};
#pragma pack(pop)
		virtual void InitializeSurface();
		SurfaceInfo m_surfaceInfo = {};
		std::queue<SplashInfo> m_splashQueue;
		std::vector<Edge> m_particleEdges;
		std::vector<float> m_particleHeights;
		std::array<Vector2, 2> m_bounds {};
		float m_originY = 0.f;
		bool m_bUseThread = true;

		std::vector<Particle> &GetParticleField();
		std::vector<Edge> &GetParticleEdges();
		virtual uint8_t GetEdgeIterationCount() const;
		Vector3 CalcParticlePosition(const SurfaceInfo &surfInfo, const std::vector<float> &heights, std::size_t ptIdx) const;

		// Threaded data (Not thread-safe!)
		std::vector<float> m_threadParticleHeights;
		std::vector<Particle> m_particleField;
		std::thread m_simThread;
		std::atomic<bool> m_bRunThread = {true};
		std::mutex m_splashMutex;
		std::mutex m_settingsMutex;
		std::mutex m_heightMutex;
		void SimulateWaves(double dt);
		void JoinThread();
		void SolveDepths(const SurfaceInfo &surfInfo);
		void SolveEdges(const SurfaceInfo &surfInfo);
		void Integrate(const SurfaceInfo &surfInfo, double dt);
		void VelocityFixup(const SurfaceInfo &surfInfo, double invDt);
		void SetParticleHeight(const SurfaceInfo &surfInfo, std::size_t ptIdx, float height);
		std::size_t GetParticleIndex(const SurfaceInfo &surfInfo, uint32_t x, uint32_t y) const;
		std::pair<uint32_t, uint32_t> GetParticleCoordinates(const SurfaceInfo &surfInfo, std::size_t idx) const;
	};
}
