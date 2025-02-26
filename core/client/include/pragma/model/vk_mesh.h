/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __VK_MESH_H__
#define __VK_MESH_H__

#include "pragma/clientdefinitions.h"
#include <prosper_context.hpp>
#include <memory>
#include <mutex>

namespace prosper {
	class IBuffer;
	class IRenderBuffer;
};

class CModelSubMesh;
namespace pragma {
	namespace model {
		enum class IndexType : uint8_t;
	};
	class ShaderEntity;
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
		void SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer, pragma::model::IndexType indexType);
		void ClearBuffers();

		const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(CModelSubMesh &mesh, pragma::ShaderEntity &shader, uint32_t pipelineIdx = 0u);
	  private:
		void SetDirty();
		std::vector<std::pair<prosper::PipelineID, std::shared_ptr<prosper::IRenderBuffer>>> m_renderBuffers;

		std::shared_ptr<prosper::IBuffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_vertexWeightBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_alphaBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_indexBuffer = nullptr;
		std::mutex m_renderBufferMutex;
		pragma::model::IndexType m_indexType;
	};
};
#endif
