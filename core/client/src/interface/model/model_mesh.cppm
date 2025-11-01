// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"


export module pragma.client:model.mesh;

export import :model.render_mesh;
export import :model.vertex_buffer_data;
export import :rendering.shaders.scene;
export import pragma.shared;

export class DLLCLIENT CModelSubMesh : public pragma::ModelSubMesh {
  public:
	CModelSubMesh();
	CModelSubMesh(const CModelSubMesh &other);
	const std::shared_ptr<pragma::SceneMesh> &GetSceneMesh() const;
	virtual void Update(pragma::model::ModelUpdateFlags flags = pragma::model::ModelUpdateFlags::AllData) override;
	virtual void Centralize(const Vector3 &origin) override;
	virtual std::shared_ptr<pragma::ModelSubMesh> Copy(bool fullCopy = false) const override;
	static void InitializeBuffers();
	static void ClearBuffers();

	static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalVertexBuffer();
	static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalVertexWeightBuffer();
	static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalAlphaBuffer();
	static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalIndexBuffer();
	const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(pragma::ShaderEntity &shader, uint32_t pipelineIdx = 0u);

	using VertexType = VertexBufferData;
	using VertexWeightType = umath::VertexWeight;
	using AlphaType = Vector2;
  private:
	std::shared_ptr<pragma::SceneMesh> m_sceneMesh;
	void UpdateVertexBuffer();
};

export class DLLCLIENT CModelMesh : public ModelMesh {
  private:
	virtual void AddSubMesh(const std::shared_ptr<pragma::ModelSubMesh> &subMesh) override;
  public:
	CModelMesh();
	void AddSubMesh(const std::shared_ptr<CModelSubMesh> &subMesh);
	virtual std::shared_ptr<ModelMesh> Copy() const override;
};
