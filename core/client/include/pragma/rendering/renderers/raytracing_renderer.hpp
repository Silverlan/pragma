/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RAYTRACING_RENDERER_HPP__
#define __RAYTRACING_RENDERER_HPP__

#include "pragma/rendering/renderers/base_renderer.hpp"
#include <sharedutils/util_weak_handle.hpp>

namespace prosper {class Texture; class DescriptorSetGroup; class Shader;};
namespace Anvil {class DescriptorSet;};
namespace pragma::rendering
{
	class DLLCLIENT RaytracingRenderer
		: public BaseRenderer
	{
	public:
		prosper::IDescriptorSet& GetOutputImageDescriptorSet();

		virtual void EndRendering() override;
		virtual bool RenderScene(const util::DrawSceneInfo &drawSceneInfo) override;
		virtual bool ReloadRenderTarget(uint32_t width,uint32_t height) override;
		virtual bool IsRayTracingRenderer() const override;
		virtual prosper::Texture *GetSceneTexture() override;
		virtual prosper::Texture *GetHDRPresentationTexture() override;
	private:
		friend BaseRenderer;
		using BaseRenderer::BaseRenderer;
		virtual bool Initialize() override;
		std::shared_ptr<prosper::Texture> m_outputTexture = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgOutputImage = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgLights = nullptr;
		util::WeakHandle<prosper::Shader> m_whShader = {};
	};
};

#endif
