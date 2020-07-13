/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PREPASS_HPP__
#define __C_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <sharedutils/util_weak_handle.hpp>

namespace Anvil {class PrimaryCommandBuffer;};
namespace prosper {class IPrContext; class RenderTarget; class Texture; class Shader; class IPrimaryCommandBuffer;};
namespace util {struct DrawSceneInfo;};
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class ShaderPrepassBase;
	namespace rendering
	{
		class DLLCLIENT Prepass
		{
		public:
			bool Initialize(prosper::IPrContext &context,uint32_t width,uint32_t height,prosper::SampleCountFlags sampleCount,bool bExtended);
			pragma::ShaderPrepassBase &GetShader() const;
			void BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo);
			void EndRenderPass(const util::DrawSceneInfo &drawSceneInfo);
			std::shared_ptr<prosper::Texture> textureNormals = nullptr;

			// Required for SSAO
			std::shared_ptr<prosper::Texture> textureDepth = nullptr;

			// Depth buffer used for sampling (e.g. particle render pass)
			std::shared_ptr<prosper::Texture> textureDepthSampled = nullptr;

			std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;

			void SetUseExtendedPrepass(bool b,bool bForceReload=false);
			bool IsExtended() const;
		private:
			bool m_bExtended = false;
			std::vector<prosper::ClearValue> m_clearValues = {};
			util::WeakHandle<prosper::Shader> m_shaderPrepass = {};
			util::WeakHandle<prosper::Shader> m_shaderPrepassDepth = {};
		};
	};
};
#pragma warning(pop)

#endif
