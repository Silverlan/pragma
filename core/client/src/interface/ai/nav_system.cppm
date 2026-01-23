// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:ai.nav_system;

import :debug;

export namespace pragma {
	namespace nav {
		class DLLCLIENT CMesh : public Mesh {
		  public:
			static std::shared_ptr<CMesh> Create(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config);
			static std::shared_ptr<CMesh> Load(Game &game, const std::string &fname);
			using Mesh::Mesh;
			friend Mesh;

			void Clear();
			void ShowNavMeshes(bool b);
			void SetDebugPathStart(Vector3 &start);
			void SetDebugPathEnd(Vector3 &end);
		  private:
			bool m_bShowNavMeshes = false;
			uint32_t m_numVertices = 0;
			uint32_t m_numLines = 0;
			uint32_t m_numPath = 0;
			std::unique_ptr<Vector3> m_dbgPathStart = nullptr;
			std::unique_ptr<Vector3> m_dbgPathEnd = nullptr;
			std::array<std::shared_ptr<debug::DebugRenderer::BaseObject>, 2> m_dbgPointLines = {nullptr, nullptr};
			std::shared_ptr<debug::DebugRenderer::BaseObject> m_dbgNavPath = nullptr;
			std::shared_ptr<debug::DebugRenderer::BaseObject> m_dbgNavMesh = nullptr;

			void UpdateDebugPath(Vector3 &start, Vector3 &end);
			void UpdateDepthPathTargets();
		};
	};
};
