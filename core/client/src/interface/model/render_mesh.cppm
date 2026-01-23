// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.render_mesh;

export import :rendering.shaders.scene;
export import pragma.shared;

export namespace pragma::geometry {
	class CModelSubMesh;
}
export namespace pragma::rendering {
	class DLLCLIENT SceneMesh {
	  public:
		SceneMesh();
		SceneMesh(const SceneMesh &other);
		~SceneMesh();
		SceneMesh &operator=(const SceneMesh &other);
		const std::shared_ptr<prosper::IBuffer> &GetVertexBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetVertexWeightBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetAlphaBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetIndexBuffer() const;
		void SetVertexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetVertexWeightBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetAlphaBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer, geometry::IndexType indexType);
		void ClearBuffers();

		const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(geometry::CModelSubMesh &mesh, ShaderEntity &shader, uint32_t pipelineIdx = 0u);
	  private:
		void SetDirty();
		std::vector<std::pair<prosper::PipelineID, std::shared_ptr<prosper::IRenderBuffer>>> m_renderBuffers;

		std::shared_ptr<prosper::IBuffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_vertexWeightBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_alphaBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_indexBuffer = nullptr;
		std::mutex m_renderBufferMutex;
		geometry::IndexType m_indexType;
	};
};
