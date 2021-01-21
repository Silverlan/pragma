/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __BASE_RENDERER_HPP__
#define __BASE_RENDERER_HPP__
// TODO: Remove this file
#if 0
#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_renderflags.h"
#include <prosper_command_buffer.hpp>
#include <image/prosper_texture.hpp>
#include <cinttypes>
#include <memory>

class Scene;
class CBaseEntity;
namespace pragma {struct RenderSettings; struct CameraData; class CSceneComponent;};
namespace util {struct DrawSceneInfo;};
namespace pragma::rendering
{
	class DLLCLIENT BaseRenderer
		: public std::enable_shared_from_this<BaseRenderer>
	{
	public:
		static const std::vector<BaseRenderer*> &GetRenderers();
		template<class TRenderer>
			static std::shared_ptr<TRenderer> Create(uint32_t w,uint32_t h);
		bool operator==(const BaseRenderer &other) const;
		bool operator!=(const BaseRenderer &other) const;
		virtual ~BaseRenderer();
		virtual bool RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo);
		virtual bool Render(const util::DrawSceneInfo &drawSceneInfo);

		virtual bool ReloadRenderTarget(CSceneComponent &scene,uint32_t width,uint32_t height)=0;
		virtual bool ReloadBloomRenderTarget(uint32_t width) {return true;};
		virtual prosper::Texture *GetSceneTexture()=0;
		virtual prosper::Texture *GetPresentationTexture();
		virtual prosper::Texture *GetHDRPresentationTexture()=0;
		const prosper::Texture *GetSceneTexture() const;
		const prosper::Texture *GetPresentationTexture() const;
		const prosper::Texture *GetHDRPresentationTexture() const;
		virtual void UpdateRenderSettings();
		virtual void UpdateCameraData(CSceneComponent &scene,pragma::CameraData &cameraData);
		void Resize(uint32_t width, uint32_t height);

		virtual bool IsRasterizationRenderer() const;
		virtual bool IsRayTracingRenderer() const;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
	protected:
		virtual void BeginRendering(const util::DrawSceneInfo &drawSceneInfo);
		virtual void EndRendering()=0;
		BaseRenderer();
		virtual bool Initialize(uint32_t w,uint32_t h) = 0;
	};
};

template<class TRenderer>
	std::shared_ptr<TRenderer> pragma::rendering::BaseRenderer::Create(uint32_t w,uint32_t h)
{
	auto res = std::shared_ptr<TRenderer>{new TRenderer{}};
	if(res->Initialize(w,h) == false)
		return nullptr;
	return res;
}
#endif

#endif
