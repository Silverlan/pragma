/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_FORWARDPLUS_HPP__
#define __C_FORWARDPLUS_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <memory>

namespace prosper
{
	class IPrContext;
	class IBuffer;
	class Texture;
	class ICommandBuffer;
	class IDescriptorSetGroup;
	class IDescriptorSet;
};

#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class CCameraComponent;
	namespace rendering
	{
		class RasterizationRenderer;
		class DLLCLIENT ForwardPlusInstance
		{
		public:
			ForwardPlusInstance(RasterizationRenderer &rasterizer);
			bool Initialize(prosper::IPrContext &context,uint32_t width,uint32_t height,prosper::Texture &depthTexture);

			std::pair<uint32_t,uint32_t> GetWorkGroupCount() const;
			uint32_t GetTileCount() const;
			const std::vector<uint32_t> &GetShadowLightBits() const;
			prosper::IDescriptorSet *GetDepthDescriptorSetGraphics() const;
			const std::shared_ptr<prosper::IBuffer> &GetTileVisLightIndexBuffer() const;
			const std::shared_ptr<prosper::IBuffer> &GetVisLightIndexBuffer() const;

			void Compute(prosper::IPrimaryCommandBuffer &cmdBuffer,prosper::IImage &imgDepth,prosper::IDescriptorSet &descSetCam);

			static std::pair<uint32_t,uint32_t> CalcWorkGroupCount(uint32_t w,uint32_t h);
			static uint32_t CalcTileCount(uint32_t w,uint32_t h);
		private:
			RasterizationRenderer &m_rasterizer;
			uint32_t m_workGroupCountX = 0u;
			uint32_t m_workGroupCountY = 0u;
			uint32_t m_tileCount = 0u;
			std::vector<uint32_t> m_shadowLightBits;
			std::shared_ptr<prosper::IBuffer> m_bufTileVisLightIndex = nullptr;
			std::shared_ptr<prosper::IBuffer> m_bufVisLightIndex = nullptr;
			util::WeakHandle<prosper::Shader> m_shaderLightCulling = {};
			util::WeakHandle<prosper::Shader> m_shaderLightIndexing = {};
			std::shared_ptr<prosper::IPrimaryCommandBuffer> m_cmdBuffer = nullptr;
			uint32_t m_cmdBufferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

			std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgSceneDepthBuffer = nullptr;
		};
	};
};
#pragma warning(pop)

#endif
