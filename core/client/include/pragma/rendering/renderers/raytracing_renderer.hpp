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
		Anvil::DescriptorSet& GetOutputImageDescriptorSet();

		virtual void EndRendering() override;
		virtual bool RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags=FRender::All) override;
		virtual bool ReloadRenderTarget() override;
		virtual bool IsRayTracingRenderer() const override;
		virtual void OnEntityAddedToScene(CBaseEntity &ent) override;
		virtual const std::shared_ptr<prosper::Texture> &GetSceneTexture() const override;
	private:
		friend BaseRenderer;
		using BaseRenderer::BaseRenderer;
		virtual bool Initialize() override;
		std::shared_ptr<prosper::Texture> m_outputTexture = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_dsgOutputImage = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_dsgLights = nullptr;
		util::WeakHandle<prosper::Shader> m_whShader = {};
	};
};

#endif
