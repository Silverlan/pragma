#ifndef __VK_MESH_H__
#define __VK_MESH_H__

#include "pragma/clientdefinitions.h"
#include <memory>

namespace prosper
{
	class Buffer;
};

namespace pragma
{
	class DLLCLIENT VkMesh
	{
	private:
		std::shared_ptr<prosper::Buffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::Buffer> m_vertexWeightBuffer = nullptr;
		std::shared_ptr<prosper::Buffer> m_alphaBuffer = nullptr;
		std::shared_ptr<prosper::Buffer> m_indexBuffer = nullptr;
	public:
		VkMesh();
		const std::shared_ptr<prosper::Buffer> &GetVertexBuffer() const;
		const std::shared_ptr<prosper::Buffer> &GetVertexWeightBuffer() const;
		const std::shared_ptr<prosper::Buffer> &GetAlphaBuffer() const;
		const std::shared_ptr<prosper::Buffer> &GetIndexBuffer() const;
		void SetVertexBuffer(const std::shared_ptr<prosper::Buffer> &buffer);
		void SetVertexWeightBuffer(const std::shared_ptr<prosper::Buffer> &buffer);
		void SetAlphaBuffer(const std::shared_ptr<prosper::Buffer> &buffer);
		void SetIndexBuffer(const std::shared_ptr<prosper::Buffer> &buffer);
	};
};
#endif
