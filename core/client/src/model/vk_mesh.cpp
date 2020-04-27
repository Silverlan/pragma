/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/vk_mesh.h"

using namespace pragma;

VkMesh::VkMesh()
	: m_vertexBuffer(nullptr),m_vertexWeightBuffer(nullptr),
	m_alphaBuffer(nullptr),m_indexBuffer(nullptr)
{}
const std::shared_ptr<prosper::IBuffer> &VkMesh::GetVertexBuffer() const {return m_vertexBuffer;}
const std::shared_ptr<prosper::IBuffer> &VkMesh::GetVertexWeightBuffer() const {return m_vertexWeightBuffer;}
const std::shared_ptr<prosper::IBuffer> &VkMesh::GetAlphaBuffer() const {return m_alphaBuffer;}
const std::shared_ptr<prosper::IBuffer> &VkMesh::GetIndexBuffer() const {return m_indexBuffer;}
void VkMesh::SetVertexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_vertexBuffer = buffer;}
void VkMesh::SetVertexWeightBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_vertexWeightBuffer = buffer;}
void VkMesh::SetAlphaBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_alphaBuffer = buffer;}
void VkMesh::SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_indexBuffer = buffer;}
