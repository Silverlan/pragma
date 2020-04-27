/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
		std::shared_ptr<prosper::IBuffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_vertexWeightBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_alphaBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_indexBuffer = nullptr;
	public:
		VkMesh();
		const std::shared_ptr<prosper::IBuffer> &GetVertexBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetVertexWeightBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetAlphaBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetIndexBuffer() const;
		void SetVertexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetVertexWeightBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetAlphaBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
		void SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer);
	};
};
#endif
