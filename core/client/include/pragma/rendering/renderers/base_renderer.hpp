#ifndef __BASE_RENDERER_HPP__
#define __BASE_RENDERER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_renderflags.h"
#include <prosper_command_buffer.hpp>
#include <image/prosper_texture.hpp>
#include <cinttypes>
#include <memory>

class Scene;
class CBaseEntity;
namespace pragma {struct RenderSettings; struct CameraData;};
namespace pragma::rendering
{
	class DLLCLIENT BaseRenderer
		: public std::enable_shared_from_this<BaseRenderer>
	{
	public:
		template<class TRenderer>
			static std::shared_ptr<TRenderer> Create(Scene &scene);
		virtual ~BaseRenderer()=default;
		virtual bool RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags=FRender::All);
		virtual bool ReloadRenderTarget()=0;
		virtual const std::shared_ptr<prosper::Texture> &GetSceneTexture() const=0;
		virtual const std::shared_ptr<prosper::Texture> &GetPresentationTexture() const;
		virtual const std::shared_ptr<prosper::Texture> &GetHDRPresentationTexture() const=0;
		virtual void UpdateRenderSettings(pragma::RenderSettings &renderSettings);
		virtual void UpdateCameraData(pragma::CameraData &cameraData);
		virtual void OnEntityAddedToScene(CBaseEntity &ent);
		void Resize(uint32_t width, uint32_t height);

		virtual bool IsRasterizationRenderer() const;
		virtual bool IsRayTracingRenderer() const;

		Scene &GetScene() const;
	protected:
		virtual void BeginRendering(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		virtual void EndRendering()=0;
		BaseRenderer(Scene &scene);
		virtual bool Initialize() = 0;
		Scene &m_scene;
	};
};

template<class TRenderer>
	std::shared_ptr<TRenderer> pragma::rendering::BaseRenderer::Create(Scene &scene)
{
	auto res = std::shared_ptr<TRenderer>{new TRenderer{scene}};
	if(res->Initialize() == false)
		return nullptr;
	return res;
}

#endif
