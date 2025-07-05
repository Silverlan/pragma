// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_MODELMESH_H__
#define __C_MODELMESH_H__
#include "pragma/clientdefinitions.h"
#include <pragma/model/modelmesh.h>
#include <memory>

namespace prosper {
	class IDynamicResizableBuffer;
	class IRenderBuffer;
};
namespace pragma {
	class SceneMesh;
	class ShaderEntity;
};
namespace umath {
	struct VertexWeight;
};
struct VertexBufferData;
class DLLCLIENT CModelSubMesh : public ModelSubMesh {
  public:
	CModelSubMesh();
	CModelSubMesh(const CModelSubMesh &other);
	const std::shared_ptr<pragma::SceneMesh> &GetSceneMesh() const;
	virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData) override;
	virtual void Centralize(const Vector3 &origin) override;
	virtual std::shared_ptr<ModelSubMesh> Copy(bool fullCopy = false) const override;
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

class DLLCLIENT CModelMesh : public ModelMesh {
  private:
	virtual void AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh) override;
  public:
	CModelMesh();
	void AddSubMesh(const std::shared_ptr<CModelSubMesh> &subMesh);
	virtual std::shared_ptr<ModelMesh> Copy() const override;
};

#endif
