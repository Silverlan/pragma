#include "stdafx_client.h"
#include "pragma/model/vk_mesh.h"

using namespace pragma;

VkMesh::VkMesh()
	: m_vertexBuffer(nullptr),m_vertexWeightBuffer(nullptr),
	m_alphaBuffer(nullptr),m_indexBuffer(nullptr)
{}
const std::shared_ptr<prosper::Buffer> &VkMesh::GetVertexBuffer() const {return m_vertexBuffer;}
const std::shared_ptr<prosper::Buffer> &VkMesh::GetVertexWeightBuffer() const {return m_vertexWeightBuffer;}
const std::shared_ptr<prosper::Buffer> &VkMesh::GetAlphaBuffer() const {return m_alphaBuffer;}
const std::shared_ptr<prosper::Buffer> &VkMesh::GetIndexBuffer() const {return m_indexBuffer;}
void VkMesh::SetVertexBuffer(const std::shared_ptr<prosper::Buffer> &buffer) {m_vertexBuffer = buffer;}
void VkMesh::SetVertexWeightBuffer(const std::shared_ptr<prosper::Buffer> &buffer) {m_vertexWeightBuffer = buffer;}
void VkMesh::SetAlphaBuffer(const std::shared_ptr<prosper::Buffer> &buffer) {m_alphaBuffer = buffer;}
void VkMesh::SetIndexBuffer(const std::shared_ptr<prosper::Buffer> &buffer) {m_indexBuffer = buffer;}
